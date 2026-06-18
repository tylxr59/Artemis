#include "git/GitService.h"
#include "persistence/Database.h"

#include <QDir>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

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
        QVERIFY(database.setSetting(QStringLiteral("test"), QStringLiteral("value"), &error));
        QCOMPARE(database.setting(QStringLiteral("test")), QStringLiteral("value"));
    }
};

QTEST_MAIN(TestCore)
#include "TestCore.moc"
