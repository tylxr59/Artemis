#include "git/GitService.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTimer>

#include <memory>
#include <utility>

namespace Artemis {

GitService::GitService(QObject *parent) : QObject(parent) {}

GitResult GitService::runSync(const QString &cwd, const QStringList &arguments,
                              const QProcessEnvironment &environment)
{
    QProcess process;
    process.setWorkingDirectory(cwd);
    if (!environment.isEmpty())
        process.setProcessEnvironment(environment);
    process.start(QStringLiteral("git"), arguments);
    if (!process.waitForStarted(3000))
        return {-1, {}, process.errorString().toUtf8()};
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

void GitService::run(const QString &cwd, const QStringList &arguments, Handler handler,
                     const QProcessEnvironment &environment, int timeoutMs)
{
    auto *process = new QProcess(this);
    auto *timeout = new QTimer(process);
    timeout->setSingleShot(true);
    process->setWorkingDirectory(cwd);
    if (!environment.isEmpty())
        process->setProcessEnvironment(environment);
    const auto complete = [process, timeout, handler = std::move(handler),
                           completed = std::make_shared<bool>(false)](
                              int code, const QByteArray &fallbackError = {}) {
        if (std::exchange(*completed, true))
            return;
        timeout->stop();
        const auto error = process->readAllStandardError();
        const GitResult result{
            code,
            process->readAllStandardOutput(),
            error.isEmpty() ? fallbackError : error
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
    process->start(QStringLiteral("git"), arguments);
    timeout->start(timeoutMs);
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

void GitService::status(const QString &path, Handler handler)
{
    run(path, {QStringLiteral("status"), QStringLiteral("--porcelain=v1"),
               QStringLiteral("--branch"), QStringLiteral("-z")}, std::move(handler));
}

void GitService::diff(const QString &path, Handler handler)
{
    run(path, {QStringLiteral("diff"), QStringLiteral("--no-ext-diff"),
               QStringLiteral("--binary"), QStringLiteral("HEAD")}, std::move(handler));
}

void GitService::commitAllAndPush(const QString &path, const QString &subject,
                                  const QString &body, Handler handler)
{
    run(path, {QStringLiteral("add"), QStringLiteral("-A")},
        [this, path, subject, body, handler = std::move(handler)](
            const GitResult &add) mutable {
        if (!add.ok()) {
            if (handler)
                handler(add);
            return;
        }
        commitStagedAndPush(path, subject, body, std::move(handler));
    });
}

void GitService::commitStagedAndPush(const QString &path, const QString &subject,
                                     const QString &body, Handler handler)
{
    QStringList arguments{QStringLiteral("commit"), QStringLiteral("-m"), subject.trimmed()};
    if (!body.trimmed().isEmpty())
        arguments << QStringLiteral("-m") << body.trimmed();
    run(path, arguments,
        [this, path, handler = std::move(handler)](const GitResult &commit) mutable {
        if (commit.ok()) {
            pushCurrentBranch(path, commit.output, std::move(handler));
            return;
        }
        run(path, {QStringLiteral("status"), QStringLiteral("--porcelain")},
            [this, path, commit, handler = std::move(handler)](
                const GitResult &status) mutable {
            if (!status.ok() || !status.output.trimmed().isEmpty()) {
                if (handler)
                    handler(commit);
                return;
            }
            pushCurrentBranch(path, commit.output, std::move(handler));
        });
    });
}

void GitService::pushCurrentBranch(const QString &path, const QByteArray &commitOutput,
                                   Handler handler)
{
    run(path, {QStringLiteral("branch"), QStringLiteral("--show-current")},
        [this, path, commitOutput, handler = std::move(handler)](
            const GitResult &branchResult) mutable {
        const auto branch = QString::fromUtf8(branchResult.output).trimmed();
        if (!branchResult.ok() || branch.isEmpty()) {
            if (handler) {
                handler({-1, commitOutput,
                         QByteArray("Changes are committed locally, but the current branch could "
                                    "not be determined; nothing was pushed.")});
            }
            return;
        }
        run(path, {QStringLiteral("rev-parse"), QStringLiteral("--abbrev-ref"),
                   QStringLiteral("--symbolic-full-name"), QStringLiteral("@{upstream}")},
            [this, path, branch, commitOutput, handler = std::move(handler)](
                const GitResult &upstream) mutable {
            if (upstream.ok()) {
                run(path, {QStringLiteral("push")}, std::move(handler));
                return;
            }
            run(path, {QStringLiteral("remote"), QStringLiteral("get-url"),
                       QStringLiteral("origin")},
                [this, path, branch, commitOutput, handler = std::move(handler)](
                    const GitResult &origin) mutable {
                if (!origin.ok()) {
                    if (handler) {
                        handler({-1, commitOutput,
                                 QByteArray("Changes are committed locally, but this branch has no "
                                            "upstream and the 'origin' remote is not configured; "
                                            "nothing was pushed.")});
                    }
                    return;
                }
                run(path, {QStringLiteral("push"), QStringLiteral("--set-upstream"),
                           QStringLiteral("origin"), branch}, std::move(handler));
            });
        });
    });
}

void GitService::createBranchCommitPush(const QString &path, const QString &branch,
                                        const QString &subject, const QString &body,
                                        const QString &remote, Handler handler)
{
    run(path, {QStringLiteral("switch"), QStringLiteral("-c"), branch},
        [this, path, branch, subject, body, remote, handler = std::move(handler)](
            const GitResult &checkout) mutable {
        if (!checkout.ok()) {
            if (handler)
                handler(checkout);
            return;
        }
        run(path, {QStringLiteral("add"), QStringLiteral("-A")},
            [this, path, branch, subject, body, remote, handler = std::move(handler)](
                const GitResult &add) mutable {
            if (!add.ok()) {
                if (handler)
                    handler(add);
                return;
            }
            QStringList arguments{
                QStringLiteral("commit"), QStringLiteral("-m"), subject.trimmed()
            };
            if (!body.trimmed().isEmpty())
                arguments << QStringLiteral("-m") << body.trimmed();
            run(path, arguments,
                [this, path, branch, remote, handler = std::move(handler)](
                    const GitResult &commit) mutable {
                if (!commit.ok()) {
                    if (handler)
                        handler(commit);
                    return;
                }
                run(path, {QStringLiteral("push"), QStringLiteral("--set-upstream"),
                           remote, branch}, std::move(handler));
            });
        });
    });
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
                }, environment);
        }, environment);
    }, environment);
}

} // namespace Artemis
