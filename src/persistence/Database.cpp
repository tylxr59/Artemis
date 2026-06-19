#include "persistence/Database.h"
#include "platform/Paths.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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
    if (!execute(QStringLiteral("PRAGMA foreign_keys = ON"), error)
        || !execute(QStringLiteral("PRAGMA journal_mode = WAL"), error)) {
        return false;
    }
    return migrate(error);
}

bool Database::migrate(QString *error)
{
    if (!execute(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS schema_migrations ("
            "version INTEGER PRIMARY KEY, "
            "applied_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)"), error)) {
        return false;
    }

    QSqlQuery versionQuery(m_db);
    if (!versionQuery.exec(QStringLiteral(
            "SELECT COALESCE(MAX(version), 0) FROM schema_migrations"))
        || !versionQuery.next()) {
        if (error)
            *error = versionQuery.lastError().text();
        return false;
    }
    const int currentVersion = versionQuery.value(0).toInt();

    struct Migration {
        int version;
        QStringList statements;
    };
    const QVector<Migration> migrations = {
        {1, {
            QStringLiteral(
                "CREATE TABLE IF NOT EXISTS projects ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT NOT NULL UNIQUE, "
                "name TEXT NOT NULL, created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)"),
            QStringLiteral(
                "CREATE TABLE IF NOT EXISTS thread_bindings ("
                "provider_thread_id TEXT PRIMARY KEY, project_id INTEGER NOT NULL "
                "REFERENCES projects(id) ON DELETE CASCADE, workspace_path TEXT NOT NULL, "
                "external INTEGER NOT NULL DEFAULT 0, "
                "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)"),
            QStringLiteral(
                "CREATE TABLE IF NOT EXISTS hidden_threads ("
                "provider_thread_id TEXT NOT NULL, project_id INTEGER NOT NULL "
                "REFERENCES projects(id) ON DELETE CASCADE, "
                "hidden_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP, "
                "PRIMARY KEY(provider_thread_id, project_id))")
        }},
        {2, {
            QStringLiteral(
                "CREATE TABLE IF NOT EXISTS application_settings ("
                "key TEXT PRIMARY KEY, value TEXT NOT NULL)")
        }},
        {3, {
            QStringLiteral(
                "CREATE TABLE IF NOT EXISTS conversation_events ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, provider_thread_id TEXT NOT NULL, "
                "event_key TEXT, type TEXT NOT NULL, title TEXT NOT NULL, "
                "content TEXT NOT NULL, metadata TEXT NOT NULL DEFAULT '{}', "
                "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)"),
            QStringLiteral(
                "CREATE UNIQUE INDEX IF NOT EXISTS conversation_events_thread_key "
                "ON conversation_events(provider_thread_id, event_key) "
                "WHERE event_key IS NOT NULL"),
            QStringLiteral(
                "CREATE INDEX IF NOT EXISTS conversation_events_thread_order "
                "ON conversation_events(provider_thread_id, id)")
        }}
    };

    for (const auto &migration : migrations) {
        if (migration.version <= currentVersion)
            continue;
        if (!m_db.transaction()) {
            if (error)
                *error = m_db.lastError().text();
            return false;
        }
        for (const auto &statement : migration.statements) {
            if (execute(statement, error))
                continue;
            m_db.rollback();
            return false;
        }
        QSqlQuery record(m_db);
        record.prepare(QStringLiteral(
            "INSERT INTO schema_migrations(version) VALUES (?)"));
        record.addBindValue(migration.version);
        if (!record.exec()) {
            if (error)
                *error = record.lastError().text();
            m_db.rollback();
            return false;
        }
        if (!m_db.commit()) {
            if (error)
                *error = m_db.lastError().text();
            return false;
        }
    }
    return true;
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
                          bool external, QString *error)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO thread_bindings(provider_thread_id, project_id, workspace_path, external) "
        "VALUES (?, ?, ?, ?) ON CONFLICT(provider_thread_id) DO UPDATE SET "
        "project_id=excluded.project_id, workspace_path=excluded.workspace_path, "
        "external=excluded.external, updated_at=CURRENT_TIMESTAMP"));
    query.addBindValue(threadId);
    query.addBindValue(projectId);
    query.addBindValue(workspacePath);
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
    query.prepare(QStringLiteral(
        "SELECT b.provider_thread_id, b.workspace_path, b.external "
        "FROM thread_bindings b JOIN projects p ON p.id=b.project_id "
        "WHERE b.project_id=? AND b.workspace_path=p.path ORDER BY b.updated_at DESC"));
    query.addBindValue(projectId);
    query.exec();
    while (query.next())
        result.push_back({{QStringLiteral("threadId"), query.value(0)},
                          {QStringLiteral("workspacePath"), query.value(1)},
                          {QStringLiteral("external"), query.value(2)}});
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

QVector<QVariantMap> Database::conversationEvents(const QString &threadId) const
{
    QVector<QVariantMap> result;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT type, title, content, metadata FROM conversation_events "
        "WHERE provider_thread_id=? ORDER BY id"));
    query.addBindValue(threadId);
    if (!query.exec())
        return result;
    while (query.next()) {
        const auto metadataDocument = QJsonDocument::fromJson(query.value(3).toByteArray());
        result.push_back({
            {QStringLiteral("type"), query.value(0)},
            {QStringLiteral("title"), query.value(1)},
            {QStringLiteral("content"), query.value(2)},
            {QStringLiteral("metadata"), metadataDocument.object().toVariantMap()}
        });
    }
    return result;
}

bool Database::saveConversationEvent(const QString &threadId, const QString &type,
                                     const QString &title, const QString &content,
                                     const QVariantMap &metadata, QString *error)
{
    // QJsonValue::toString() returns a null QString when an optional protocol
    // field is absent. QSQLITE binds that as SQL NULL rather than empty text.
    const auto storedContent = content.isNull() ? QStringLiteral("") : content;
    QString eventKey;
    const auto itemId = metadata.value(QStringLiteral("itemId")).toString();
    const auto turnId = metadata.value(QStringLiteral("turnId")).toString();
    if (!itemId.isEmpty()) {
        eventKey = QStringLiteral("item:%1").arg(itemId);
    } else if (!turnId.isEmpty()
               && (type == QStringLiteral("diff") || type == QStringLiteral("plan")
                   || type == QStringLiteral("status"))) {
        eventKey = QStringLiteral("%1:%2").arg(type, turnId);
    }

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO conversation_events(provider_thread_id, event_key, type, title, content, metadata) "
        "VALUES (?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(provider_thread_id, event_key) WHERE event_key IS NOT NULL DO UPDATE SET "
        "type=excluded.type, title=excluded.title, content=excluded.content, metadata=excluded.metadata"));
    query.addBindValue(threadId);
    query.addBindValue(eventKey.isEmpty() ? QVariant{} : QVariant{eventKey});
    query.addBindValue(type);
    query.addBindValue(title);
    query.addBindValue(storedContent);
    query.addBindValue(QString::fromUtf8(
        QJsonDocument(QJsonObject::fromVariantMap(metadata)).toJson(QJsonDocument::Compact)));
    if (!query.exec()) {
        if (error)
            *error = query.lastError().text();
        return false;
    }
    return true;
}

QSet<QString> Database::referencedAttachmentPaths() const
{
    QSet<QString> result;
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral(
            "SELECT metadata FROM conversation_events "
            "WHERE metadata LIKE '%\"images\"%'"))) {
        return result;
    }
    while (query.next()) {
        const auto document = QJsonDocument::fromJson(query.value(0).toByteArray());
        const auto images = document.object().value(QStringLiteral("images")).toArray();
        for (const auto &image : images) {
            const auto path = image.toString();
            if (!path.isEmpty())
                result.insert(path);
        }
    }
    return result;
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

int Database::schemaVersion() const
{
    QSqlQuery query(m_db);
    return query.exec(QStringLiteral(
               "SELECT COALESCE(MAX(version), 0) FROM schema_migrations"))
            && query.next()
        ? query.value(0).toInt()
        : 0;
}

QString Database::path() const
{
    return m_db.databaseName();
}

} // namespace Artemis
