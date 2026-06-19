#pragma once

#include <QObject>
#include <QProcessEnvironment>

#include <functional>
#include <memory>

namespace Artemis {

enum class GitFailure {
    None,
    IndexLocked,
    MutationInProgress,
};

struct GitResult {
    int exitCode = -1;
    QByteArray output;
    QByteArray error;
    GitFailure failure = GitFailure::None;
    bool ok() const { return exitCode == 0; }
};

class GitService : public QObject {
    Q_OBJECT
public:
    using Handler = std::function<void(const GitResult &)>;

    explicit GitService(QObject *parent = nullptr);
    static GitResult runSync(const QString &cwd, const QStringList &arguments,
                             const QProcessEnvironment &environment = {});
    static bool isRepository(const QString &path);
    static bool statusHasChanges(const QByteArray &porcelainStatus);
    static QString canonicalProjectPath(const QString &path);
    static QString suggestedBranch(const QString &subject);
    static bool isIndexLockError(const QByteArray &error);

    void status(const QString &path, Handler handler);
    void diff(const QString &path, Handler handler);
    void remoteUrl(const QString &path, const QString &remote, Handler handler);
    void commitAllAndPush(const QString &path, const QString &subject,
                          const QString &body, Handler handler);
    void createBranchCommitPush(const QString &path, const QString &branch,
                                const QString &subject, const QString &body,
                                const QString &remote, Handler handler);
    void generateCommitSnapshot(const QString &path, Handler handler);
    void retryLockedOperation();
    void removeIndexLockAndRetry(Handler handler);
    void cancelLockedOperation();

private:
    struct MutationWorkflow;
    using Workflow = std::shared_ptr<MutationWorkflow>;
    using StepHandler = std::function<void(const GitResult &)>;

    void run(const QString &cwd, const QStringList &arguments, Handler handler,
             const QProcessEnvironment &environment = {}, int timeoutMs = 120000);
    void startMutation(const QString &path, Handler handler,
                       std::function<void(const Workflow &)> start);
    void runMutationStep(const Workflow &workflow, const QStringList &arguments,
                         StepHandler handler, int retry = 0);
    void finishMutation(const Workflow &workflow, const GitResult &result);
    void commitStagedAndPush(const Workflow &workflow, const QString &subject,
                             const QString &body);
    void pushCurrentBranch(const Workflow &workflow, const QByteArray &commitOutput);

    Workflow m_activeMutation;
};

} // namespace Artemis
