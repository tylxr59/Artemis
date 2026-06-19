#pragma once

#include <QObject>
#include <QProcessEnvironment>
#include <QVariantList>

#include <functional>

namespace Artemis {

struct GitResult {
    int exitCode = -1;
    QByteArray output;
    QByteArray error;
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
    static QString canonicalProjectPath(const QString &path);
    static QString validateBranchName(const QString &path, const QString &branch);
    static QString suggestedBranch(const QString &subject);

    void status(const QString &path, Handler handler);
    void diff(const QString &path, Handler handler);
    void commitAllAndPush(const QString &path, const QString &subject,
                          const QString &body, Handler handler);
    void createBranchCommitPush(const QString &path, const QString &branch,
                                const QString &subject, const QString &body,
                                const QString &remote, Handler handler);
    void generateCommitSnapshot(const QString &path, Handler handler);

private:
    void run(const QString &cwd, const QStringList &arguments, Handler handler,
             const QProcessEnvironment &environment = {});
    void pushCurrentBranch(const QString &path, const QByteArray &commitOutput,
                           Handler handler);
};

} // namespace Artemis
