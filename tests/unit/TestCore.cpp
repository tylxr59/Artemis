#include "app/AppController.h"
#include "git/GitService.h"
#include "persistence/Database.h"
#include "platform/DesktopIntegration.h"
#include "platform/Paths.h"
#include "providers/codex/CodexClient.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QStandardPaths>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QUuid>

using namespace Artemis;

class FakeAgentProvider final : public AgentProvider {
public:
    ProviderCapabilities capabilities() const override { return {}; }
    bool ready() const override { return m_ready; }
    QString version() const override { return QStringLiteral("test-provider"); }

    void start() override
    {
        m_ready = true;
        emit readyChanged(true);
    }

    void listModels(ResultHandler handler) override
    {
        handler({{QStringLiteral("data"), QJsonArray{}}}, {});
    }

    void listThreads(const QString &, ResultHandler handler) override
    {
        handler({{QStringLiteral("data"), QJsonArray{}}}, {});
    }

    void startThread(const ThreadConfiguration &configuration,
                     ResultHandler handler) override
    {
        lastThreadConfiguration = configuration;
        pendingStartThread = std::move(handler);
    }

    void resumeThread(const QString &, ResultHandler handler) override
    {
        handler({{QStringLiteral("thread"),
                  QJsonObject{{QStringLiteral("turns"), QJsonArray{}}}}}, {});
    }

    void sendTurn(const QString &, const QString &text, const QStringList &,
                  const QString &modelId, const QString &, const QString &,
                  PermissionProfile, ResultHandler handler) override
    {
        lastTurnText = text;
        lastTurnModelId = modelId;
        handler({{QStringLiteral("turn"),
                  QJsonObject{{QStringLiteral("id"), QStringLiteral("turn-1")}}}}, {});
    }

    void steerTurn(const QString &, const QString &, const QString &,
                   const QStringList &, ResultHandler handler) override
    {
        handler({}, {});
    }

    void interruptTurn(const QString &, const QString &, ResultHandler handler) override
    {
        handler({}, {});
    }

    void setThreadName(const QString &, const QString &, ResultHandler handler) override
    {
        handler({}, {});
    }

    QString itemContent(const QJsonObject &item) const override
    {
        return item.value(QStringLiteral("text")).toString();
    }

    void completeThreadStart(const QString &threadId)
    {
        auto handler = std::move(pendingStartThread);
        QVERIFY(handler);
        handler({{QStringLiteral("thread"),
                  QJsonObject{{QStringLiteral("id"), threadId}}}}, {});
    }

    ThreadConfiguration lastThreadConfiguration;
    ResultHandler pendingStartThread;
    QString lastTurnText;
    QString lastTurnModelId;

private:
    bool m_ready = false;
};

class TestCore : public QObject {
    Q_OBJECT
private slots:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        QFile::remove(Paths::databasePath());
        QFile::remove(Paths::databasePath() + QStringLiteral("-wal"));
        QFile::remove(Paths::databasePath() + QStringLiteral("-shm"));
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
        QVERIFY(QDir().mkpath(directory.filePath(QStringLiteral("nested/project"))));
        QVERIFY(GitService::isRepository(
            directory.filePath(QStringLiteral("nested/project"))));
    }

    void gitStatusChangeDetection()
    {
        const auto status = [](std::initializer_list<QByteArray> records) {
            QByteArray output;
            for (const auto &record : records) {
                output.append(record);
                output.append('\0');
            }
            return output;
        };

        QVERIFY(!GitService::statusHasChanges(status({"## main"})));
        QVERIFY(GitService::statusHasChanges(status({"## main", " M tracked.txt"})));
        QVERIFY(GitService::statusHasChanges(status({"## main", "?? untracked.txt"})));
        QVERIFY(GitService::statusHasChanges(
            status({"## main", "R  renamed.txt", "old-name.txt"})));
    }

    void gitIndexLockErrorDetection()
    {
        QVERIFY(GitService::isIndexLockError(
            "fatal: Unable to create '/tmp/repo/.git/index.lock': File exists.\n"));
        QVERIFY(!GitService::isIndexLockError("fatal: unable to access remote"));
    }

    void databaseMigration()
    {
        Database database;
        QString error;
        QVERIFY2(database.open(&error), qPrintable(error));
        QCOMPARE(database.schemaVersion(), 3);
        const auto id = database.addProject(QDir::tempPath() + QStringLiteral("/artemis-test-project"),
                                            QStringLiteral("Test"), &error);
        QVERIFY2(id > 0, qPrintable(error));
        QVERIFY(!database.projects().isEmpty());
        QVERIFY(database.bindThread(id, QStringLiteral("thread-to-hide"),
                                    QDir::tempPath(), false, &error));
        QVERIFY(database.hideThread(id, QStringLiteral("thread-to-hide"), &error));
        QVERIFY(database.hiddenThreadIds(id).contains(QStringLiteral("thread-to-hide")));
        QVERIFY(database.threadBindings(id).isEmpty());
        const auto historyThread = QStringLiteral("thread-history-%1").arg(
            QUuid::createUuid().toString(QUuid::WithoutBraces));
        QVERIFY(database.saveConversationEvent(
            historyThread, QStringLiteral("command"),
            QStringLiteral("Running command"), QStringLiteral("first"),
            {{QStringLiteral("itemId"), QStringLiteral("item-1")},
             {QStringLiteral("lifecycle"), QStringLiteral("started")}}, &error));
        QVERIFY2(error.isEmpty(), qPrintable(error));
        QVERIFY(database.saveConversationEvent(
            historyThread, QStringLiteral("command"),
            QStringLiteral("Ran command"), QStringLiteral("completed output"),
            {{QStringLiteral("itemId"), QStringLiteral("item-1")},
             {QStringLiteral("lifecycle"), QStringLiteral("completed")}}, &error));
        QVERIFY(database.saveConversationEvent(
            historyThread, QStringLiteral("assistant"),
            QStringLiteral("Artemis"), QStringLiteral("Final answer"),
            {{QStringLiteral("itemId"), QStringLiteral("item-2")},
             {QStringLiteral("images"),
              QStringList{QStringLiteral("/tmp/artemis-reference.png")}}}, &error));
        const auto events = database.conversationEvents(historyThread);
        QCOMPARE(events.size(), 2);
        QCOMPARE(events.at(0).value(QStringLiteral("title")).toString(),
                 QStringLiteral("Ran command"));
        QCOMPARE(events.at(0).value(QStringLiteral("content")).toString(),
                 QStringLiteral("completed output"));
        QCOMPARE(events.at(1).value(QStringLiteral("content")).toString(),
                 QStringLiteral("Final answer"));
        const auto emptyContentThread = QStringLiteral("thread-null-content-%1").arg(
            QUuid::createUuid().toString(QUuid::WithoutBraces));
        QVERIFY(database.saveConversationEvent(
            emptyContentThread, QStringLiteral("status"),
            QStringLiteral("Turn completed"), QString{},
            {{QStringLiteral("turnId"), QStringLiteral("turn-without-status")}}, &error));
        const auto emptyContentEvents = database.conversationEvents(emptyContentThread);
        QCOMPARE(emptyContentEvents.size(), 1);
        QCOMPARE(emptyContentEvents.constFirst().value(QStringLiteral("content")).toString(),
                 QStringLiteral(""));
        QVERIFY(database.referencedAttachmentPaths().contains(
            QStringLiteral("/tmp/artemis-reference.png")));
        QVERIFY(database.setSetting(QStringLiteral("test"), QStringLiteral("value"), &error));
        QCOMPARE(database.setting(QStringLiteral("test")), QStringLiteral("value"));
    }

    void databaseMigrationIsIdempotent()
    {
        Database database;
        QString error;
        QVERIFY2(database.open(&error), qPrintable(error));
        QCOMPARE(database.schemaVersion(), 3);
        QVERIFY2(database.migrate(&error), qPrintable(error));
        QCOMPARE(database.schemaVersion(), 3);
    }

    void asynchronousGitStartupFailureCompletes()
    {
        const auto previousPath = qgetenv("PATH");
        qputenv("PATH", QByteArray());

        GitService service;
        GitResult result;
        bool completed = false;
        QEventLoop loop;
        QTimer timeout;
        timeout.setSingleShot(true);
        connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
        timeout.start(3000);
        service.status(QDir::tempPath(), [&result, &completed, &loop](const GitResult &value) {
            result = value;
            completed = true;
            loop.quit();
        });
        loop.exec();
        qputenv("PATH", previousPath);

        QVERIFY2(timeout.isActive(), "Timed out waiting for Git startup failure");
        QVERIFY(completed);
        QVERIFY(!result.ok());
        QVERIFY(!result.error.isEmpty());
    }

    void terminalLauncherFallsBackToInstalledEmulator()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto executable = directory.filePath(QStringLiteral("konsole"));
        const auto output = directory.filePath(QStringLiteral("terminal-cwd"));
        QFile script(executable);
        QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));
        script.write("#!/bin/sh\npwd > \"$ARTEMIS_TEST_TERMINAL_OUTPUT\"\n");
        script.close();
        QVERIFY(script.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                      | QFileDevice::ExeOwner));

        const auto previousPath = qgetenv("PATH");
        qputenv("PATH", directory.path().toUtf8());
        qputenv("ARTEMIS_TEST_TERMINAL_OUTPUT", output.toUtf8());
        QString error;
        const auto launched = DesktopIntegration::openTerminal(
            QString{}, directory.path(), &error);
        QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(output), 2000);
        qunsetenv("ARTEMIS_TEST_TERMINAL_OUTPUT");
        qputenv("PATH", previousPath);

        QVERIFY2(launched, qPrintable(error));
        QFile result(output);
        QVERIFY(result.open(QIODevice::ReadOnly | QIODevice::Text));
        QCOMPARE(QString::fromUtf8(result.readAll()).trimmed(), directory.path());
    }

    void selectedTerminalUsesDesktopWorkingDirectoryArgument()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto executable = directory.filePath(QStringLiteral("fake-terminal"));
        const auto output = directory.filePath(QStringLiteral("terminal-argument"));
        QFile script(executable);
        QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));
        script.write("#!/bin/sh\nprintf '%s' \"$1\" > \"$ARTEMIS_TEST_TERMINAL_OUTPUT\"\n");
        script.close();
        QVERIFY(script.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                      | QFileDevice::ExeOwner));

        const auto applicationsPath = QDir(
            QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
                                          .filePath(QStringLiteral("applications"));
        QVERIFY(QDir().mkpath(applicationsPath));
        const auto desktopId = QStringLiteral("artemis-test-terminal.desktop");
        const auto desktopPath = QDir(applicationsPath).filePath(desktopId);
        QFile desktopFile(desktopPath);
        QVERIFY(desktopFile.open(QIODevice::WriteOnly | QIODevice::Text));
        desktopFile.write(QStringLiteral(
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=Artemis Test Terminal\n"
            "Exec=%1\n"
            "Categories=System;TerminalEmulator;\n"
            "X-TerminalArgDir=--cwd=\n").arg(executable).toUtf8());
        desktopFile.close();

        qputenv("ARTEMIS_TEST_TERMINAL_OUTPUT", output.toUtf8());
        QString error;
        const auto launched = DesktopIntegration::openTerminal(
            desktopId, directory.path(), &error);
        QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(output), 2000);
        qunsetenv("ARTEMIS_TEST_TERMINAL_OUTPUT");
        QFile::remove(desktopPath);

        QVERIFY2(launched, qPrintable(error));
        QFile result(output);
        QVERIFY(result.open(QIODevice::ReadOnly | QIODevice::Text));
        QCOMPARE(QString::fromUtf8(result.readAll()), QStringLiteral("--cwd=") + directory.path());
    }

    void codexStartupFailureIsReported()
    {
        qputenv("ARTEMIS_CODEX_EXECUTABLE",
                QByteArray("/definitely/missing/artemis-codex"));
        CodexClient client;
        QSignalSpy errors(&client, &CodexClient::providerError);
        client.start();
        QTRY_VERIFY_WITH_TIMEOUT(!errors.isEmpty(), 2000);
        QVERIFY(errors.constFirst().constFirst().toString().contains(
            QStringLiteral("not found"), Qt::CaseInsensitive));
        qunsetenv("ARTEMIS_CODEX_EXECUTABLE");
    }

    void codexRestartsAfterCrash()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto executable = directory.filePath(QStringLiteral("fake-codex"));
        QFile script(executable);
        QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));
        script.write(
            "#!/bin/sh\n"
            "if [ \"$1\" = \"--version\" ]; then\n"
            "  echo 'codex-cli 0.141.0'\n"
            "  exit 0\n"
            "fi\n"
            "IFS= read -r request\n"
            "id=$(printf '%s' \"$request\" | sed -n 's/.*\"id\":\\([0-9][0-9]*\\).*/\\1/p')\n"
            "printf '{\"id\":%s,\"result\":{}}\\n' \"$id\"\n"
            "IFS= read -r initialized\n"
            "sleep 0.1\n"
            "exit 1\n");
        script.close();
        QVERIFY(script.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                      | QFileDevice::ExeOwner));

        qputenv("ARTEMIS_CODEX_EXECUTABLE", executable.toUtf8());
        CodexClient client;
        QSignalSpy readyChanges(&client, &CodexClient::readyChanged);
        client.start();

        QTRY_VERIFY_WITH_TIMEOUT(readyChanges.count() >= 3, 5000);
        int readyCount = 0;
        for (const auto &arguments : readyChanges) {
            if (arguments.constFirst().toBool())
                ++readyCount;
        }
        QVERIFY2(readyCount >= 2, "Codex did not become ready after restarting");
        qunsetenv("ARTEMIS_CODEX_EXECUTABLE");
    }

    void codexDisconnectCallbacksAreNotReentrant()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto executable = directory.filePath(QStringLiteral("fake-codex"));
        QFile script(executable);
        QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));
        script.write(
            "#!/bin/sh\n"
            "if [ \"$1\" = \"--version\" ]; then\n"
            "  echo 'codex-cli 0.141.0'\n"
            "  exit 0\n"
            "fi\n"
            "IFS= read -r request\n"
            "exit 1\n");
        script.close();
        QVERIFY(script.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                      | QFileDevice::ExeOwner));

        qputenv("ARTEMIS_CODEX_EXECUTABLE", executable.toUtf8());
        CodexClient client;
        QSignalSpy errors(&client, &CodexClient::providerError);
        client.start();
        QTRY_VERIFY_WITH_TIMEOUT(errors.count() >= 2, 3000);
        QVERIFY(errors.count() < 20);
        qunsetenv("ARTEMIS_CODEX_EXECUTABLE");
    }

    void delayedThreadCreationDoesNotChangeAnotherProject()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const auto firstPath = root.filePath(QStringLiteral("a-project"));
        const auto secondPath = root.filePath(QStringLiteral("b-project"));
        QVERIFY(QDir().mkpath(firstPath));
        QVERIFY(QDir().mkpath(secondPath));

        FakeAgentProvider provider;
        AppController controller(&provider);
        QVERIFY(controller.initialize());
        controller.addProject(firstPath);
        controller.addProject(secondPath);

        int firstIndex = -1;
        int secondIndex = -1;
        for (int row = 0; row < controller.projects()->rowCount(); ++row) {
            const auto index = controller.projects()->index(row);
            const auto path = controller.projects()
                                  ->data(index, ProjectTreeModel::PathRole).toString();
            if (path == firstPath)
                firstIndex = row;
            else if (path == secondPath)
                secondIndex = row;
        }
        QVERIFY(firstIndex >= 0);
        QVERIFY(secondIndex >= 0);

        controller.selectProject(firstIndex);
        QSignalSpy restored(&controller, &AppController::promptRestoreRequested);
        QVERIFY(controller.sendPrompt(
            QStringLiteral("Create a file"), {}, QStringLiteral("test-model"), {},
            QStringLiteral("full-access"), QStringLiteral("default")));
        QVERIFY(provider.pendingStartThread);
        QCOMPARE(provider.lastThreadConfiguration.projectPath, firstPath);

        controller.selectProject(secondIndex);
        provider.completeThreadStart(QStringLiteral("thread-for-first-project"));

        QCOMPARE(controller.selectedProjectPath(), secondPath);
        QVERIFY(controller.selectedThreadId().isEmpty());
        QVERIFY(controller.threads().isEmpty());
        QCOMPARE(restored.count(), 1);
    }

    void commitGenerationKeepsOriginalRepositoryContext()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const auto firstPath = root.filePath(QStringLiteral("a-repository"));
        const auto secondPath = root.filePath(QStringLiteral("b-repository"));
        QVERIFY(QDir().mkpath(firstPath));
        QVERIFY(QDir().mkpath(secondPath));

        QVERIFY(GitService::runSync(firstPath, {QStringLiteral("init")}).ok());
        QVERIFY(GitService::runSync(
            firstPath, {QStringLiteral("config"), QStringLiteral("user.name"),
                        QStringLiteral("Artemis Test")}).ok());
        QVERIFY(GitService::runSync(
            firstPath, {QStringLiteral("config"), QStringLiteral("user.email"),
                        QStringLiteral("artemis@example.invalid")}).ok());
        QVERIFY(GitService::runSync(
            firstPath, {QStringLiteral("switch"), QStringLiteral("-c"),
                        QStringLiteral("main")}).ok());
        QFile change(QDir(firstPath).filePath(QStringLiteral("context.txt")));
        QVERIFY(change.open(QIODevice::WriteOnly | QIODevice::Text));
        change.write("original\n");
        change.close();
        QVERIFY(GitService::runSync(firstPath, {QStringLiteral("add"),
                                               QStringLiteral("-A")}).ok());
        QVERIFY(GitService::runSync(
            firstPath, {QStringLiteral("commit"), QStringLiteral("-m"),
                        QStringLiteral("Initial commit")}).ok());
        QVERIFY(change.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text));
        change.write("change from first repository\n");
        change.close();

        FakeAgentProvider provider;
        AppController controller(&provider);
        QVERIFY(controller.initialize());
        controller.addProject(firstPath);
        controller.addProject(secondPath);
        controller.setCommitModelId(QStringLiteral("commit-test-model"));

        int firstIndex = -1;
        int secondIndex = -1;
        for (int row = 0; row < controller.projects()->rowCount(); ++row) {
            const auto index = controller.projects()->index(row);
            const auto path = controller.projects()
                                  ->data(index, ProjectTreeModel::PathRole).toString();
            if (path == firstPath)
                firstIndex = row;
            else if (path == secondPath)
                secondIndex = row;
        }
        QVERIFY(firstIndex >= 0);
        QVERIFY(secondIndex >= 0);

        controller.selectProject(firstIndex);
        controller.generateCommitMessage();
        QTRY_VERIFY_WITH_TIMEOUT(provider.pendingStartThread, 3000);
        QCOMPARE(provider.lastThreadConfiguration.projectPath, firstPath);
        QCOMPARE(provider.lastThreadConfiguration.workspacePath, firstPath);

        controller.selectProject(secondIndex);
        provider.completeThreadStart(QStringLiteral("commit-thread"));

        QCOMPARE(provider.lastTurnModelId, QStringLiteral("commit-test-model"));
        QVERIFY(provider.lastTurnText.contains(QStringLiteral("Repository: a-repository")));
        QVERIFY(provider.lastTurnText.contains(
            QStringLiteral("change from first repository")));
        QVERIFY(!provider.lastTurnText.contains(QStringLiteral("Repository: b-repository")));
    }

    void codexActiveTurnRequestsIncludeTurnId()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto executable = directory.filePath(QStringLiteral("fake-codex"));
        const auto requestsPath = directory.filePath(QStringLiteral("requests.jsonl"));
        QFile script(executable);
        QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));
        script.write(
            "#!/bin/sh\n"
            "if [ \"$1\" = \"--version\" ]; then\n"
            "  echo 'codex-cli 0.141.0'\n"
            "  exit 0\n"
            "fi\n"
            "while IFS= read -r request; do\n"
            "  printf '%s\\n' \"$request\" >> \"$ARTEMIS_TEST_CODEX_REQUESTS\"\n"
            "  id=$(printf '%s' \"$request\" | sed -n 's/.*\"id\":\\([0-9][0-9]*\\).*/\\1/p')\n"
            "  if [ -n \"$id\" ]; then\n"
            "    printf '{\"id\":%s,\"result\":{}}\\n' \"$id\"\n"
            "  fi\n"
            "done\n");
        script.close();
        QVERIFY(script.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                      | QFileDevice::ExeOwner));

        qputenv("ARTEMIS_CODEX_EXECUTABLE", executable.toUtf8());
        qputenv("ARTEMIS_TEST_CODEX_REQUESTS", requestsPath.toUtf8());
        CodexClient client;
        QSignalSpy readyChanges(&client, &CodexClient::readyChanged);
        client.start();
        QTRY_VERIFY_WITH_TIMEOUT(!readyChanges.isEmpty()
                                     && readyChanges.constLast().constFirst().toBool(),
                                 3000);

        bool steerCompleted = false;
        bool interruptCompleted = false;
        client.steerTurn(QStringLiteral("thread-1"), QStringLiteral("turn-1"),
                         QStringLiteral("Focus on tests"), {},
                         [&steerCompleted](const QJsonObject &, const QString &error) {
            QVERIFY2(error.isEmpty(), qPrintable(error));
            steerCompleted = true;
        });
        client.interruptTurn(
            QStringLiteral("thread-1"), QStringLiteral("turn-1"),
            [&interruptCompleted](const QJsonObject &, const QString &error) {
                QVERIFY2(error.isEmpty(), qPrintable(error));
                interruptCompleted = true;
            });
        QTRY_VERIFY_WITH_TIMEOUT(steerCompleted && interruptCompleted, 3000);

        QFile requests(requestsPath);
        QVERIFY(requests.open(QIODevice::ReadOnly | QIODevice::Text));
        QJsonObject steerParams;
        QJsonObject interruptParams;
        while (!requests.atEnd()) {
            const auto document = QJsonDocument::fromJson(requests.readLine());
            const auto request = document.object();
            const auto method = request.value(QStringLiteral("method")).toString();
            if (method == QStringLiteral("turn/steer"))
                steerParams = request.value(QStringLiteral("params")).toObject();
            else if (method == QStringLiteral("turn/interrupt"))
                interruptParams = request.value(QStringLiteral("params")).toObject();
        }
        QCOMPARE(steerParams.value(QStringLiteral("threadId")).toString(),
                 QStringLiteral("thread-1"));
        QCOMPARE(steerParams.value(QStringLiteral("expectedTurnId")).toString(),
                 QStringLiteral("turn-1"));
        QCOMPARE(interruptParams.value(QStringLiteral("threadId")).toString(),
                 QStringLiteral("thread-1"));
        QCOMPARE(interruptParams.value(QStringLiteral("turnId")).toString(),
                 QStringLiteral("turn-1"));

        qunsetenv("ARTEMIS_TEST_CODEX_REQUESTS");
        qunsetenv("ARTEMIS_CODEX_EXECUTABLE");
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

    void commitRecoversAfterExplicitLockRemoval()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const auto remotePath = root.filePath(QStringLiteral("remote.git"));
        const auto workPath = root.filePath(QStringLiteral("work"));
        QVERIFY(GitService::runSync(
            root.path(), {QStringLiteral("init"), QStringLiteral("--bare"), remotePath}).ok());
        QVERIFY(QDir().mkpath(workPath));
        QVERIFY(GitService::runSync(workPath, {QStringLiteral("init")}).ok());
        QVERIFY(GitService::runSync(
            workPath, {QStringLiteral("config"), QStringLiteral("user.name"),
                       QStringLiteral("Artemis Test")}).ok());
        QVERIFY(GitService::runSync(
            workPath, {QStringLiteral("config"), QStringLiteral("user.email"),
                       QStringLiteral("artemis@example.invalid")}).ok());
        QVERIFY(GitService::runSync(
            workPath, {QStringLiteral("switch"), QStringLiteral("-c"),
                       QStringLiteral("main")}).ok());
        QVERIFY(GitService::runSync(
            workPath, {QStringLiteral("remote"), QStringLiteral("add"),
                       QStringLiteral("origin"), remotePath}).ok());

        QFile change(QDir(workPath).filePath(QStringLiteral("change.txt")));
        QVERIFY(change.open(QIODevice::WriteOnly));
        change.write("recover from a stale lock\n");
        change.close();

        QFile lock(QDir(workPath).filePath(QStringLiteral(".git/index.lock")));
        QVERIFY(lock.open(QIODevice::WriteOnly));
        lock.close();

        GitService service;
        GitResult finalResult;
        GitResult removalResult;
        int callbackCount = 0;
        QEventLoop loop;
        QTimer timeout;
        timeout.setSingleShot(true);
        connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
        timeout.start(10000);

        service.commitAllAndPush(
            workPath, QStringLiteral("Recover stale lock"), {},
            [&service, &finalResult, &removalResult, &callbackCount, &loop](
                const GitResult &result) {
            ++callbackCount;
            if (result.failure == GitFailure::IndexLocked) {
                service.removeIndexLockAndRetry(
                    [&removalResult](const GitResult &removal) {
                    removalResult = removal;
                });
                return;
            }
            finalResult = result;
            loop.quit();
        });
        loop.exec();

        QVERIFY2(timeout.isActive(), "Timed out recovering from stale Git lock");
        QCOMPARE(callbackCount, 2);
        QVERIFY2(removalResult.ok(), removalResult.error.constData());
        QVERIFY2(finalResult.ok(), finalResult.error.constData());
        QVERIFY(!lock.exists());
        const auto remote = GitService::runSync(
            remotePath, {QStringLiteral("rev-parse"), QStringLiteral("refs/heads/main")});
        QVERIFY(remote.ok());
    }
};

QTEST_MAIN(TestCore)
#include "TestCore.moc"
