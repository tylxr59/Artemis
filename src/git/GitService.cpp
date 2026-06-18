#include "git/GitService.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTemporaryDir>

#include <memory>

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
    process.waitForFinished(120000);
    return {process.exitCode(), process.readAllStandardOutput(), process.readAllStandardError()};
}

void GitService::run(const QString &cwd, const QStringList &arguments, Handler handler,
                     const QProcessEnvironment &environment)
{
    auto *process = new QProcess(this);
    process->setWorkingDirectory(cwd);
    if (!environment.isEmpty())
        process->setProcessEnvironment(environment);
    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [process, handler = std::move(handler)](int code, QProcess::ExitStatus) {
        const GitResult result{code, process->readAllStandardOutput(), process->readAllStandardError()};
        process->deleteLater();
        if (handler)
            handler(result);
    });
    process->start(QStringLiteral("git"), arguments);
}

bool GitService::isRepository(const QString &path)
{
    return runSync(path, {QStringLiteral("rev-parse"), QStringLiteral("--is-inside-work-tree")}).ok();
}

QString GitService::canonicalProjectPath(const QString &path)
{
    QFileInfo info(path);
    const auto canonical = info.canonicalFilePath();
    return canonical.isEmpty() ? QDir::cleanPath(info.absoluteFilePath()) : canonical;
}

QString GitService::validateBranchName(const QString &path, const QString &branch)
{
    if (branch.trimmed().isEmpty())
        return QStringLiteral("Branch name is empty");
    const auto result = runSync(path, {QStringLiteral("check-ref-format"),
                                       QStringLiteral("--branch"), branch});
    return result.ok() ? QString() : QString::fromUtf8(result.error).trimmed();
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

void GitService::createWorktree(const QString &projectPath, const QString &destination, Handler handler)
{
    QDir().mkpath(QFileInfo(destination).absolutePath());
    run(projectPath, {QStringLiteral("worktree"), QStringLiteral("add"),
                      QStringLiteral("--detach"), destination, QStringLiteral("HEAD")},
        std::move(handler));
}

void GitService::commitAllAndPush(const QString &path, const QString &message, Handler handler)
{
    const auto add = runSync(path, {QStringLiteral("add"), QStringLiteral("-A")});
    if (!add.ok()) {
        if (handler)
            handler(add);
        return;
    }
    const auto commit = runSync(path, {QStringLiteral("commit"), QStringLiteral("-m"), message});
    if (!commit.ok()) {
        const auto status = runSync(path, {QStringLiteral("status"),
                                           QStringLiteral("--porcelain")});
        if (!status.ok() || !status.output.trimmed().isEmpty()) {
            if (handler)
                handler(commit);
            return;
        }
    }
    pushCurrentBranch(path, commit.output, std::move(handler));
}

void GitService::pushCurrentBranch(const QString &path, const QByteArray &commitOutput,
                                   Handler handler)
{
    const auto branchResult = runSync(path, {QStringLiteral("branch"),
                                             QStringLiteral("--show-current")});
    const auto branch = QString::fromUtf8(branchResult.output).trimmed();
    if (!branchResult.ok() || branch.isEmpty()) {
        if (handler) {
            handler({-1, commitOutput,
                     QByteArray("Changes are committed locally, but the current branch could not "
                                "be determined; nothing was pushed.")});
        }
        return;
    }

    const auto upstream = runSync(path, {QStringLiteral("rev-parse"),
                                         QStringLiteral("--abbrev-ref"),
                                         QStringLiteral("--symbolic-full-name"),
                                         QStringLiteral("@{upstream}")});
    if (upstream.ok()) {
        run(path, {QStringLiteral("push")}, std::move(handler));
        return;
    }

    const auto origin = runSync(path, {QStringLiteral("remote"), QStringLiteral("get-url"),
                                       QStringLiteral("origin")});
    if (!origin.ok()) {
        if (handler) {
            handler({-1, commitOutput,
                     QByteArray("Changes are committed locally, but this branch has no upstream "
                                "and the 'origin' remote is not configured; nothing was pushed.")});
        }
        return;
    }
    run(path, {QStringLiteral("push"), QStringLiteral("--set-upstream"),
               QStringLiteral("origin"), branch}, std::move(handler));
}

void GitService::createBranchCommitPush(const QString &path, const QString &branch,
                                        const QString &message, const QString &remote, Handler handler)
{
    const auto checkout = runSync(path, {QStringLiteral("switch"), QStringLiteral("-c"), branch});
    if (!checkout.ok()) {
        if (handler)
            handler(checkout);
        return;
    }
    const auto add = runSync(path, {QStringLiteral("add"), QStringLiteral("-A")});
    if (!add.ok()) {
        if (handler)
            handler(add);
        return;
    }
    const auto commit = runSync(path, {QStringLiteral("commit"), QStringLiteral("-m"), message});
    if (!commit.ok()) {
        if (handler)
            handler(commit);
        return;
    }
    run(path, {QStringLiteral("push"), QStringLiteral("--set-upstream"), remote, branch},
        std::move(handler));
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
    auto readTree = runSync(path, {QStringLiteral("read-tree"), QStringLiteral("HEAD")}, environment);
    if (!readTree.ok()) {
        if (handler)
            handler(readTree);
        return;
    }
    auto add = runSync(path, {QStringLiteral("add"), QStringLiteral("-A")}, environment);
    if (!add.ok()) {
        if (handler)
            handler(add);
        return;
    }
    run(path, {QStringLiteral("diff"), QStringLiteral("--cached"), QStringLiteral("--stat"),
               QStringLiteral("--patch"), QStringLiteral("--no-ext-diff")},
        [handler = std::move(handler), temp](const GitResult &result) {
            if (handler)
                handler(result);
        }, environment);
}

} // namespace Artemis
