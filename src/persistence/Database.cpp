#include "persistence/Database.h"
#include "platform/Paths.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

namespace Artemis {

Database::Database()
    : m_connectionName(QStringLiteral("artemis-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)))
{
}

Database::~Database()
{
    if (m_db.isValid())
        m_db.close();
    m_db = {};
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool Database::open(QString *error)
{
    if (!QDir().mkpath(QFileInfo(Paths::databasePath()).absolutePath())) {
        if (error)
            *error = QStringLiteral("Could not create database directory");
        return false;
    }
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_db.setDatabaseName(Paths::databasePath());
    if (!m_db.open()) {
        if (error)
            *error = m_db.lastError().text();
        return false;
    }
    execute(QStringLiteral("PRAGMA foreign_keys = ON"));
    execute(QStringLiteral("PRAGMA journal_mode = WAL"));
    return migrate(error);
}

bool Database::migrate(QString *error)
{
    const QStringList statements = {
        QStringLiteral("CREATE TABLE IF NOT EXISTS schema_migrations (version INTEGER PRIMARY KEY, applied_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS projects (id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT NOT NULL UNIQUE, name TEXT NOT NULL, created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS thread_bindings (provider_thread_id TEXT PRIMARY KEY, project_id INTEGER NOT NULL REFERENCES projects(id) ON DELETE CASCADE, workspace_path TEXT NOT NULL, location TEXT NOT NULL DEFAULT 'local', external INTEGER NOT NULL DEFAULT 0, updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS hidden_threads (provider_thread_id TEXT NOT NULL, project_id INTEGER NOT NULL REFERENCES projects(id) ON DELETE CASCADE, hidden_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP, PRIMARY KEY(provider_thread_id, project_id))"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS managed_worktrees (provider_thread_id TEXT PRIMARY KEY, project_id INTEGER NOT NULL REFERENCES projects(id) ON DELETE CASCADE, path TEXT NOT NULL UNIQUE, base_commit TEXT NOT NULL, stale INTEGER NOT NULL DEFAULT 0, created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS project_preferences (project_id INTEGER NOT NULL REFERENCES projects(id) ON DELETE CASCADE, key TEXT NOT NULL, value TEXT NOT NULL, PRIMARY KEY(project_id, key))"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS application_settings (key TEXT PRIMARY KEY, value TEXT NOT NULL)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS recent_models (model_id TEXT PRIMARY KEY, used_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)"),
        QStringLiteral("INSERT OR IGNORE INTO schema_migrations(version) VALUES (1)")
    };
    if (!m_db.transaction()) {
        if (error)
            *error = m_db.lastError().text();
        return false;
    }
    for (const auto &statement : statements) {
        if (!execute(statement, error)) {
            m_db.rollback();
            return false;
        }
    }
    return m_db.commit();
}

bool Database::execute(const QString &sql, QString *error) const
{
    QSqlQuery query(m_db);
    if (!query.exec(sql)) {
        if (error)
            *error = query.lastError().text();
        return false;
    }
    return true;
}

QVector<QVariantMap> Database::projects() const
{
    QVector<QVariantMap> result;
    QSqlQuery query(m_db);
    query.exec(QStringLiteral("SELECT id, path, name FROM projects ORDER BY name COLLATE NOCASE"));
    while (query.next())
        result.push_back({{QStringLiteral("id"), query.value(0)},
                          {QStringLiteral("path"), query.value(1)},
                          {QStringLiteral("name"), query.value(2)}});
    return result;
}

qint64 Database::addProject(const QString &path, const QString &name, QString *error)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("INSERT INTO projects(path, name) VALUES (?, ?) ON CONFLICT(path) DO UPDATE SET name=excluded.name RETURNING id"));
    query.addBindValue(path);
    query.addBindValue(name);
    if (!query.exec() || !query.next()) {
        if (error)
            *error = query.lastError().text();
        return -1;
    }
    return query.value(0).toLongLong();
}

bool Database::removeProject(qint64 id, QString *error)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("DELETE FROM projects WHERE id=?"));
    query.addBindValue(id);
    if (!query.exec()) {
        if (error)
            *error = query.lastError().text();
        return false;
    }
    return true;
}

bool Database::bindThread(qint64 projectId, const QString &threadId, const QString &workspacePath,
                          const QString &location, bool external, QString *error)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO thread_bindings(provider_thread_id, project_id, workspace_path, location, external) "
        "VALUES (?, ?, ?, ?, ?) ON CONFLICT(provider_thread_id) DO UPDATE SET "
        "project_id=excluded.project_id, workspace_path=excluded.workspace_path, location=excluded.location, "
        "external=excluded.external, updated_at=CURRENT_TIMESTAMP"));
    query.addBindValue(threadId);
    query.addBindValue(projectId);
    query.addBindValue(workspacePath);
    query.addBindValue(location);
    query.addBindValue(external);
    if (!query.exec()) {
        if (error)
            *error = query.lastError().text();
        return false;
    }
    return true;
}

QVector<QVariantMap> Database::threadBindings(qint64 projectId) const
{
    QVector<QVariantMap> result;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT provider_thread_id, workspace_path, location, external FROM thread_bindings WHERE project_id=? ORDER BY updated_at DESC"));
    query.addBindValue(projectId);
    query.exec();
    while (query.next())
        result.push_back({{QStringLiteral("threadId"), query.value(0)},
                          {QStringLiteral("workspacePath"), query.value(1)},
                          {QStringLiteral("location"), query.value(2)},
                          {QStringLiteral("external"), query.value(3)}});
    return result;
}

QSet<QString> Database::hiddenThreadIds(qint64 projectId) const
{
    QSet<QString> result;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT provider_thread_id FROM hidden_threads WHERE project_id=?"));
    query.addBindValue(projectId);
    query.exec();
    while (query.next())
        result.insert(query.value(0).toString());
    return result;
}

bool Database::hideThread(qint64 projectId, const QString &threadId, QString *error)
{
    if (!m_db.transaction()) {
        if (error)
            *error = m_db.lastError().text();
        return false;
    }

    QSqlQuery hideQuery(m_db);
    hideQuery.prepare(QStringLiteral(
        "INSERT OR REPLACE INTO hidden_threads(provider_thread_id, project_id) VALUES (?, ?)"));
    hideQuery.addBindValue(threadId);
    hideQuery.addBindValue(projectId);
    if (!hideQuery.exec()) {
        if (error)
            *error = hideQuery.lastError().text();
        m_db.rollback();
        return false;
    }

    QSqlQuery bindingQuery(m_db);
    bindingQuery.prepare(QStringLiteral(
        "DELETE FROM thread_bindings WHERE provider_thread_id=? AND project_id=?"));
    bindingQuery.addBindValue(threadId);
    bindingQuery.addBindValue(projectId);
    if (!bindingQuery.exec()) {
        if (error)
            *error = bindingQuery.lastError().text();
        m_db.rollback();
        return false;
    }
    return m_db.commit();
}

bool Database::saveWorktree(qint64 projectId, const QString &threadId, const QString &path,
                            const QString &baseCommit, QString *error)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("INSERT OR REPLACE INTO managed_worktrees(provider_thread_id, project_id, path, base_commit, stale) VALUES (?, ?, ?, ?, 0)"));
    query.addBindValue(threadId);
    query.addBindValue(projectId);
    query.addBindValue(path);
    query.addBindValue(baseCommit);
    if (!query.exec()) {
        if (error)
            *error = query.lastError().text();
        return false;
    }
    return true;
}

QString Database::setting(const QString &key, const QString &fallback) const
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT value FROM application_settings WHERE key=?"));
    query.addBindValue(key);
    return query.exec() && query.next() ? query.value(0).toString() : fallback;
}

bool Database::setSetting(const QString &key, const QString &value, QString *error)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("INSERT INTO application_settings(key, value) VALUES (?, ?) ON CONFLICT(key) DO UPDATE SET value=excluded.value"));
    query.addBindValue(key);
    query.addBindValue(value);
    if (!query.exec()) {
        if (error)
            *error = query.lastError().text();
        return false;
    }
    return true;
}

QString Database::projectPreference(qint64 projectId, const QString &key) const
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT value FROM project_preferences WHERE project_id=? AND key=?"));
    query.addBindValue(projectId);
    query.addBindValue(key);
    return query.exec() && query.next() ? query.value(0).toString() : QString();
}

bool Database::setProjectPreference(qint64 projectId, const QString &key, const QString &value,
                                    QString *error)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("INSERT INTO project_preferences(project_id, key, value) VALUES (?, ?, ?) ON CONFLICT(project_id, key) DO UPDATE SET value=excluded.value"));
    query.addBindValue(projectId);
    query.addBindValue(key);
    query.addBindValue(value);
    if (!query.exec()) {
        if (error)
            *error = query.lastError().text();
        return false;
    }
    return true;
}

QString Database::path() const
{
    return m_db.databaseName();
}

} // namespace Artemis
