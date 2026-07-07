#include "git/GitService.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTimer>

#include <memory>
#include <utility>

namespace Artemis {

struct GitService::MutationWorkflow {
    QString path;
    Handler handler;
    std::function<void()> resume;
    QPointer<QProcess> process;
    bool canceled = false;
};

GitService::GitService(QObject *parent) : QObject(parent) {}

namespace {

QProcessEnvironment gitEnvironment(QProcessEnvironment environment)
{
    if (environment.isEmpty())
        environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("GIT_TERMINAL_PROMPT"), QStringLiteral("0"));
    environment.insert(QStringLiteral("GCM_INTERACTIVE"), QStringLiteral("never"));
    if (!environment.contains(QStringLiteral("GIT_SSH_COMMAND")))
        environment.insert(QStringLiteral("GIT_SSH_COMMAND"),
                           QStringLiteral("ssh -oBatchMode=yes"));
    return environment;
}

QString mutationStepText(const QStringList &arguments)
{
    if (arguments.isEmpty())
        return QStringLiteral("Running Git...");
    const auto command = arguments.first();
    if (command == QStringLiteral("add"))
        return QStringLiteral("Staging changes...");
    if (command == QStringLiteral("commit"))
        return QStringLiteral("Creating commit...");
    if (command == QStringLiteral("switch") && arguments.contains(QStringLiteral("-c")))
        return QStringLiteral("Creating feature branch...");
    if (command == QStringLiteral("branch"))
        return QStringLiteral("Checking current branch...");
    if (command == QStringLiteral("rev-parse"))
        return QStringLiteral("Checking upstream branch...");
    if (command == QStringLiteral("remote"))
        return QStringLiteral("Checking remote...");
    if (command == QStringLiteral("push") && arguments.contains(QStringLiteral("--set-upstream")))
        return QStringLiteral("Pushing and setting upstream...");
    if (command == QStringLiteral("push"))
        return QStringLiteral("Pushing changes...");
    return QStringLiteral("Running git %1...").arg(command);
}

} // namespace

GitResult GitService::runSync(const QString &cwd, const QStringList &arguments,
                              const QProcessEnvironment &environment)
{
    QProcess process;
    process.setWorkingDirectory(cwd);
    process.setProcessEnvironment(gitEnvironment(environment));
    process.start(QStringLiteral("git"), arguments);
    if (!process.waitForStarted(3000))
        return {-1, {}, process.errorString().toUtf8()};
    process.closeWriteChannel();
    if (!process.waitForFinished(120000)) {
        process.kill();
        process.waitForFinished(1000);
        return {-1, process.readAllStandardOutput(),
                QByteArray("Git operation timed out after 120 seconds")};
    }
    if (process.exitStatus() != QProcess::NormalExit)
        return {-1, process.readAllStandardOutput(), process.readAllStandardError()};
    return {process.exitCode(), process.readAllStandardOutput(), process.readAllStandardError()};
}

QPointer<QProcess> GitService::run(const QString &cwd, const QStringList &arguments,
                                   Handler handler,
                                   const QProcessEnvironment &environment, int timeoutMs,
                                   qsizetype maxOutputBytes)
{
    auto *process = new QProcess(this);
    auto *timeout = new QTimer(process);
    timeout->setSingleShot(true);
    process->setWorkingDirectory(cwd);
    process->setProcessEnvironment(gitEnvironment(environment));
    auto output = std::make_shared<QByteArray>();
    auto error = std::make_shared<QByteArray>();
    auto outputTruncated = std::make_shared<bool>(false);
    auto errorTruncated = std::make_shared<bool>(false);
    const auto appendOutput = [maxOutputBytes](QByteArray *target, const QByteArray &chunk,
                                               bool *truncated) {
        if (chunk.isEmpty())
            return;
        if (maxOutputBytes <= 0 || target->size() < maxOutputBytes) {
            const auto available = maxOutputBytes <= 0
                ? chunk.size()
                : qMin<qsizetype>(chunk.size(), maxOutputBytes - target->size());
            target->append(chunk.constData(), available);
            if (available < chunk.size())
                *truncated = true;
        } else {
            *truncated = true;
        }
    };
    connect(process, &QProcess::readyReadStandardOutput, this,
            [process, output, outputTruncated, appendOutput] {
        appendOutput(output.get(), process->readAllStandardOutput(), outputTruncated.get());
    });
    connect(process, &QProcess::readyReadStandardError, this,
            [process, error, errorTruncated, appendOutput] {
        appendOutput(error.get(), process->readAllStandardError(), errorTruncated.get());
    });
    const auto complete = [process, timeout, output, error, outputTruncated, errorTruncated,
                           appendOutput, handler = std::move(handler),
                           completed = std::make_shared<bool>(false)](
                              int code, const QByteArray &fallbackError = {}) {
        if (std::exchange(*completed, true))
            return;
        timeout->stop();
        appendOutput(output.get(), process->readAllStandardOutput(), outputTruncated.get());
        appendOutput(error.get(), process->readAllStandardError(), errorTruncated.get());
        if (*outputTruncated)
            output->append("\n\n[Artemis truncated git stdout output]\n");
        if (*errorTruncated)
            error->append("\n\n[Artemis truncated git stderr output]\n");
        const GitResult result{
            code,
            *output,
            error->isEmpty() ? fallbackError : *error
        };
        process->deleteLater();
        if (handler)
            handler(result);
    };
    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [complete](int code, QProcess::ExitStatus status) mutable {
        complete(status == QProcess::NormalExit ? code : -1,
                 QByteArray("Git process crashed"));
    });
    connect(process, &QProcess::errorOccurred, this,
            [complete, process](QProcess::ProcessError error) mutable {
        if (error == QProcess::FailedToStart)
            complete(-1, process->errorString().toUtf8());
    });
    connect(timeout, &QTimer::timeout, this, [complete, process]() mutable {
        process->kill();
        complete(-1, QByteArray("Git operation timed out"));
    });
    connect(process, &QProcess::started, process, &QProcess::closeWriteChannel);
    process->start(QStringLiteral("git"), arguments);
    timeout->start(timeoutMs);
    return process;
}

bool GitService::isRepository(const QString &path)
{
    QDir directory(path);
    while (directory.exists()) {
        if (QFileInfo(directory.filePath(QStringLiteral(".git"))).exists())
            return true;
        if (!directory.cdUp())
            break;
    }
    return false;
}

bool GitService::statusHasChanges(const QByteArray &porcelainStatus)
{
    const auto records = porcelainStatus.split('\0');
    for (const auto &record : records) {
        if (!record.isEmpty() && !record.startsWith("## "))
            return true;
    }
    return false;
}

QString GitService::canonicalProjectPath(const QString &path)
{
    QFileInfo info(path);
    const auto canonical = info.canonicalFilePath();
    return canonical.isEmpty() ? QDir::cleanPath(info.absoluteFilePath()) : canonical;
}

QString GitService::suggestedBranch(const QString &subject)
{
    QString value = subject.toLower();
    value.replace(QRegularExpression(QStringLiteral("[^a-z0-9]+")), QStringLiteral("-"));
    value.remove(QRegularExpression(QStringLiteral("^-+|-+$")));
    if (value.size() > 48)
        value = value.left(48).remove(QRegularExpression(QStringLiteral("-+$")));
    return QStringLiteral("feature/%1").arg(value.isEmpty() ? QStringLiteral("artemis-change") : value);
}

bool GitService::isIndexLockError(const QByteArray &error)
{
    return error.contains("index.lock") && error.contains("File exists");
}

void GitService::status(const QString &path, Handler handler)
{
    run(path, {QStringLiteral("status"), QStringLiteral("--porcelain=v1"),
               QStringLiteral("--branch"), QStringLiteral("-z")}, std::move(handler));
}

void GitService::diff(const QString &path, Handler handler)
{
    run(path, {QStringLiteral("diff"), QStringLiteral("--no-ext-diff"),
               QStringLiteral("HEAD")}, std::move(handler), {}, 120000, 524288);
}

void GitService::remoteUrl(const QString &path, const QString &remote, Handler handler)
{
    run(path, {QStringLiteral("remote"), QStringLiteral("get-url"), remote},
        std::move(handler));
}

void GitService::commitAllAndPush(const QString &path, const QString &subject,
                                  const QString &body, Handler handler)
{
    startMutation(path, std::move(handler),
                  [this, subject, body](const Workflow &workflow) {
        runMutationStep(workflow, {QStringLiteral("add"), QStringLiteral("-A")},
            [this, workflow, subject, body](const GitResult &add) {
            if (!add.ok()) {
                finishMutation(workflow, add);
                return;
            }
            commitStagedAndPush(workflow, subject, body);
            });
    });
}

void GitService::commitStagedAndPush(const Workflow &workflow, const QString &subject,
                                     const QString &body)
{
    QStringList arguments{QStringLiteral("commit"), QStringLiteral("-m"), subject.trimmed()};
    if (!body.trimmed().isEmpty())
        arguments << QStringLiteral("-m") << body.trimmed();
    runMutationStep(workflow, arguments,
        [this, workflow](const GitResult &commit) {
            if (commit.ok()) {
                pushCurrentBranch(workflow, commit.output);
                return;
            }
            run(workflow->path, {QStringLiteral("status"), QStringLiteral("--porcelain")},
                [this, workflow, commit](const GitResult &status) {
                if (!status.ok() || !status.output.trimmed().isEmpty()) {
                    finishMutation(workflow, commit);
                    return;
                }
                pushCurrentBranch(workflow, commit.output);
            });
        });
}

void GitService::pushCurrentBranch(const Workflow &workflow, const QByteArray &commitOutput)
{
    run(workflow->path, {QStringLiteral("branch"), QStringLiteral("--show-current")},
        [this, workflow, commitOutput](const GitResult &branchResult) {
        const auto branch = QString::fromUtf8(branchResult.output).trimmed();
        if (!branchResult.ok() || branch.isEmpty()) {
            finishMutation(workflow,
                           {-1, commitOutput,
                            QByteArray("Changes are committed locally, but the current branch could "
                                       "not be determined; nothing was pushed.")});
            return;
        }
        run(workflow->path, {QStringLiteral("rev-parse"), QStringLiteral("--abbrev-ref"),
                   QStringLiteral("--symbolic-full-name"), QStringLiteral("@{upstream}")},
            [this, workflow, branch, commitOutput](const GitResult &upstream) {
            if (upstream.ok()) {
                run(workflow->path, {QStringLiteral("push")},
                    [this, workflow](const GitResult &push) {
                    finishMutation(workflow, push);
                });
                return;
            }
            run(workflow->path, {QStringLiteral("remote"), QStringLiteral("get-url"),
                       QStringLiteral("origin")},
                [this, workflow, branch, commitOutput](const GitResult &origin) {
                if (!origin.ok()) {
                    finishMutation(
                        workflow,
                        {-1, commitOutput,
                         QByteArray("Changes are committed locally, but this branch has no "
                                    "upstream and the 'origin' remote is not configured; "
                                    "nothing was pushed.")});
                    return;
                }
                run(workflow->path,
                    {QStringLiteral("push"), QStringLiteral("--set-upstream"),
                     QStringLiteral("origin"), branch},
                    [this, workflow](const GitResult &push) {
                    finishMutation(workflow, push);
                });
            });
        });
    });
}

void GitService::createBranchCommitPush(const QString &path, const QString &branch,
                                        const QString &subject, const QString &body,
                                        const QString &remote, Handler handler)
{
    startMutation(path, std::move(handler),
                  [this, branch, subject, body, remote](const Workflow &workflow) {
        runMutationStep(workflow, {QStringLiteral("switch"), QStringLiteral("-c"), branch},
            [this, workflow, branch, subject, body, remote](const GitResult &checkout) {
            if (!checkout.ok()) {
                finishMutation(workflow, checkout);
                return;
            }
            runMutationStep(workflow, {QStringLiteral("add"), QStringLiteral("-A")},
                [this, workflow, branch, subject, body, remote](const GitResult &add) {
                if (!add.ok()) {
                    finishMutation(workflow, add);
                    return;
                }
                QStringList arguments{
                    QStringLiteral("commit"), QStringLiteral("-m"), subject.trimmed()
                };
                if (!body.trimmed().isEmpty())
                    arguments << QStringLiteral("-m") << body.trimmed();
                runMutationStep(workflow, arguments,
                    [this, workflow, branch, remote](const GitResult &commit) {
                    if (!commit.ok()) {
                        finishMutation(workflow, commit);
                        return;
                    }
                    run(workflow->path,
                        {QStringLiteral("push"), QStringLiteral("--set-upstream"), remote, branch},
                        [this, workflow](const GitResult &push) {
                        finishMutation(workflow, push);
                    });
                });
            });
        });
    });
}

void GitService::startMutation(const QString &path, Handler handler,
                               std::function<void(const Workflow &)> start)
{
    if (m_activeMutation) {
        if (handler) {
            handler({-1, {},
                     QByteArray("Another Artemis Git operation is already in progress."),
                     GitFailure::MutationInProgress});
        }
        return;
    }
    auto workflow = std::make_shared<MutationWorkflow>();
    workflow->path = path;
    workflow->handler = std::move(handler);
    m_activeMutation = workflow;
    start(workflow);
}

void GitService::runMutationStep(const Workflow &workflow, const QStringList &arguments,
                                 StepHandler handler, int retry)
{
    if (workflow->canceled)
        return;
    emit mutationStepChanged(mutationStepText(arguments));
    workflow->process = run(workflow->path, arguments,
        [this, workflow, arguments, handler = std::move(handler), retry](
            GitResult result) mutable {
        workflow->process.clear();
        if (workflow->canceled || m_activeMutation != workflow)
            return;
        if (!result.ok() && isIndexLockError(result.error)) {
            result.failure = GitFailure::IndexLocked;
            if (retry < 2) {
                const int delayMs = retry == 0 ? 300 : 700;
                QTimer::singleShot(delayMs, this,
                    [this, workflow, arguments, handler = std::move(handler), retry]() mutable {
                    runMutationStep(workflow, arguments, std::move(handler), retry + 1);
                });
                return;
            }
            workflow->resume =
                [this, workflow, arguments, handler = std::move(handler)]() mutable {
                runMutationStep(workflow, arguments, std::move(handler));
            };
            if (workflow->handler)
                workflow->handler(result);
            return;
        }
        workflow->resume = {};
        handler(result);
    });
}

void GitService::finishMutation(const Workflow &workflow, const GitResult &result)
{
    if (m_activeMutation == workflow)
        m_activeMutation.reset();
    if (workflow->handler)
        workflow->handler(result);
}

void GitService::retryLockedOperation()
{
    if (!m_activeMutation || !m_activeMutation->resume)
        return;
    auto resume = std::move(m_activeMutation->resume);
    resume();
}

void GitService::removeIndexLockAndRetry(Handler handler)
{
    if (!m_activeMutation || !m_activeMutation->resume) {
        if (handler)
            handler({-1, {}, QByteArray("There is no blocked Git operation to recover.")});
        return;
    }

    const auto workflow = m_activeMutation;
    run(
        workflow->path,
        {QStringLiteral("rev-parse"), QStringLiteral("--git-path"),
         QStringLiteral("index.lock")},
        [this, workflow, handler = std::move(handler)](const GitResult &lockResult) {
        if (m_activeMutation != workflow || !workflow->resume) {
            if (handler)
                handler({-1, {}, QByteArray("The blocked Git operation is no longer active.")});
            return;
        }
        if (!lockResult.ok()) {
            if (handler)
                handler(lockResult);
            return;
        }

        auto lockPath = QString::fromUtf8(lockResult.output).trimmed();
        if (QDir::isRelativePath(lockPath))
            lockPath = QDir(workflow->path).absoluteFilePath(lockPath);
        QFile lock(lockPath);
        if (lock.exists() && !lock.remove()) {
            if (handler) {
                handler({-1, {},
                         QStringLiteral("Could not remove Git lock file: %1")
                             .arg(lock.errorString()).toUtf8()});
            }
            return;
        }
        if (handler)
            handler({0, QByteArray("Removed stale Git index lock."), {}});
        retryLockedOperation();
    });
}

void GitService::cancelLockedOperation()
{
    if (m_activeMutation) {
        m_activeMutation->canceled = true;
        m_activeMutation->resume = {};
        if (m_activeMutation->process)
            m_activeMutation->process->kill();
    }
    m_activeMutation.reset();
}

void GitService::generateCommitSnapshot(const QString &path, Handler handler)
{
    auto temp = std::make_shared<QTemporaryDir>();
    if (!temp->isValid()) {
        if (handler)
            handler({-1, {}, QByteArray("Could not create temporary Git index")});
        return;
    }
    const auto indexPath = QDir(temp->path()).filePath(QStringLiteral("index"));
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("GIT_INDEX_FILE"), indexPath);
    run(path, {QStringLiteral("read-tree"), QStringLiteral("HEAD")},
        [this, path, environment, temp, handler = std::move(handler)](
            const GitResult &readTree) mutable {
        if (!readTree.ok()) {
            if (handler)
                handler(readTree);
            return;
        }
        run(path, {QStringLiteral("add"), QStringLiteral("-A")},
            [this, path, environment, temp, handler = std::move(handler)](
                const GitResult &add) mutable {
            if (!add.ok()) {
                if (handler)
                    handler(add);
                return;
            }
            run(path, {QStringLiteral("diff"), QStringLiteral("--cached"),
                       QStringLiteral("--stat"), QStringLiteral("--patch"),
                       QStringLiteral("--no-ext-diff")},
                [handler = std::move(handler), temp](const GitResult &result) {
                    if (handler)
                        handler(result);
                }, environment, 120000, 262144);
        }, environment);
    }, environment);
}

} // namespace Artemis
