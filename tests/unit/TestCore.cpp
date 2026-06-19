#include "git/GitService.h"
#include "persistence/Database.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

using namespace Artemis;

class TestCore : public QObject {
    Q_OBJECT
private slots:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void branchSuggestion()
    {
        QCOMPARE(GitService::suggestedBranch(QStringLiteral("Add project thread navigation")),
                 QStringLiteral("feature/add-project-thread-navigation"));
    }

    void gitRepositoryDetection()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QVERIFY(!GitService::isRepository(directory.path()));
        const auto init = GitService::runSync(directory.path(), {QStringLiteral("init")});
        QVERIFY2(init.ok(), init.error.constData());
        QVERIFY(GitService::isRepository(directory.path()));
    }

    void databaseMigration()
    {
        Database database;
        QString error;
        QVERIFY2(database.open(&error), qPrintable(error));
        const auto id = database.addProject(QDir::tempPath() + QStringLiteral("/artemis-test-project"),
                                            QStringLiteral("Test"), &error);
        QVERIFY2(id > 0, qPrintable(error));
        QVERIFY(!database.projects().isEmpty());
        QVERIFY(database.bindThread(id, QStringLiteral("thread-to-hide"),
                                    QDir::tempPath(), false, &error));
        QVERIFY(database.hideThread(id, QStringLiteral("thread-to-hide"), &error));
        QVERIFY(database.hiddenThreadIds(id).contains(QStringLiteral("thread-to-hide")));
        QVERIFY(database.threadBindings(id).isEmpty());
        QVERIFY(database.setSetting(QStringLiteral("test"), QStringLiteral("value"), &error));
        QCOMPARE(database.setting(QStringLiteral("test")), QStringLiteral("value"));
    }

    void commitAllPushesCurrentBranch()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const auto remotePath = root.filePath(QStringLiteral("remote.git"));
        const auto workPath = root.filePath(QStringLiteral("work"));
        QVERIFY(GitService::runSync(root.path(), {QStringLiteral("init"),
                                                  QStringLiteral("--bare"),
                                                  remotePath}).ok());
        QDir().mkpath(workPath);
        QVERIFY(GitService::runSync(workPath, {QStringLiteral("init")}).ok());
        QVERIFY(GitService::runSync(workPath, {QStringLiteral("config"),
                                               QStringLiteral("user.name"),
                                               QStringLiteral("Artemis Test")}).ok());
        QVERIFY(GitService::runSync(workPath, {QStringLiteral("config"),
                                               QStringLiteral("user.email"),
                                               QStringLiteral("artemis@example.invalid")}).ok());
        QVERIFY(GitService::runSync(workPath, {QStringLiteral("switch"),
                                               QStringLiteral("-c"),
                                               QStringLiteral("main")}).ok());
        QVERIFY(GitService::runSync(workPath, {QStringLiteral("remote"),
                                               QStringLiteral("add"),
                                               QStringLiteral("origin"),
                                               remotePath}).ok());

        QFile file(QDir(workPath).filePath(QStringLiteral("change.txt")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("published by Artemis\n");
        file.close();

        GitService service;
        GitResult result;
        QEventLoop loop;
        QTimer timeout;
        timeout.setSingleShot(true);
        connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
        timeout.start(10000);
        service.commitAllAndPush(workPath, QStringLiteral("Publish test change"),
                                 QStringLiteral("Explain why this test change is published."),
                                 [&result, &loop](const GitResult &value) {
            result = value;
            loop.quit();
        });
        loop.exec();

        QVERIFY2(timeout.isActive(), "Timed out waiting for Git push");
        QVERIFY2(result.ok(), result.error.constData());
        const auto message = GitService::runSync(
            workPath, {QStringLiteral("log"), QStringLiteral("-1"),
                       QStringLiteral("--pretty=%s%n%n%b")});
        QVERIFY(message.ok());
        QCOMPARE(QString::fromUtf8(message.output).trimmed(),
                 QStringLiteral("Publish test change\n\n"
                                "Explain why this test change is published."));
        const auto local = GitService::runSync(workPath, {QStringLiteral("rev-parse"),
                                                          QStringLiteral("HEAD")});
        const auto remote = GitService::runSync(remotePath, {QStringLiteral("rev-parse"),
                                                             QStringLiteral("refs/heads/main")});
        QVERIFY(local.ok());
        QVERIFY(remote.ok());
        QCOMPARE(local.output.trimmed(), remote.output.trimmed());

        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Append));
        file.write("already committed locally\n");
        file.close();
        QVERIFY(GitService::runSync(workPath, {QStringLiteral("add"),
                                               QStringLiteral("-A")}).ok());
        QVERIFY(GitService::runSync(workPath, {QStringLiteral("commit"),
                                               QStringLiteral("-m"),
                                               QStringLiteral("Local-only commit")}).ok());

        result = {};
        timeout.start(10000);
        service.commitAllAndPush(workPath, QStringLiteral("No new changes"), QString(),
                                 [&result, &loop](const GitResult &value) {
            result = value;
            loop.quit();
        });
        loop.exec();

        QVERIFY2(timeout.isActive(), "Timed out waiting to push an existing commit");
        QVERIFY2(result.ok(), result.error.constData());
        const auto recoveredLocal = GitService::runSync(
            workPath, {QStringLiteral("rev-parse"), QStringLiteral("HEAD")});
        const auto recoveredRemote = GitService::runSync(
            remotePath, {QStringLiteral("rev-parse"), QStringLiteral("refs/heads/main")});
        QCOMPARE(recoveredLocal.output.trimmed(), recoveredRemote.output.trimmed());
    }
};

QTEST_MAIN(TestCore)
#include "TestCore.moc"
