#pragma once

#include <QSqlDatabase>
#include <QSet>
#include <QVariantMap>
#include <QVector>

namespace Artemis {

class Database {
public:
    Database();
    ~Database();

    bool open(QString *error = nullptr);
    bool migrate(QString *error = nullptr);

    QVector<QVariantMap> projects() const;
    qint64 addProject(const QString &path, const QString &name, QString *error = nullptr);
    bool removeProject(qint64 id, QString *error = nullptr);
    bool bindThread(qint64 projectId, const QString &threadId, const QString &workspacePath,
                    bool external, QString *error = nullptr);
    QVector<QVariantMap> threadBindings(qint64 projectId) const;
    QSet<QString> hiddenThreadIds(qint64 projectId) const;
    bool hideThread(qint64 projectId, const QString &threadId, QString *error = nullptr);
    QVector<QVariantMap> conversationEvents(const QString &threadId) const;
    bool saveConversationEvent(const QString &threadId, const QString &type,
                               const QString &title, const QString &content,
                               const QVariantMap &metadata, QString *error = nullptr);
    QSet<QString> referencedAttachmentPaths() const;
    QString setting(const QString &key, const QString &fallback = {}) const;
    bool setSetting(const QString &key, const QString &value, QString *error = nullptr);

    int schemaVersion() const;
    QString path() const;

private:
    bool execute(const QString &sql, QString *error = nullptr) const;
    QSqlDatabase m_db;
    QString m_connectionName;
};

} // namespace Artemis
