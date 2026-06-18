#pragma once

#include <QSqlDatabase>
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
                    const QString &location, bool external, QString *error = nullptr);
    QVector<QVariantMap> threadBindings(qint64 projectId) const;
    bool saveWorktree(qint64 projectId, const QString &threadId, const QString &path,
                      const QString &baseCommit, QString *error = nullptr);
    QString setting(const QString &key, const QString &fallback = {}) const;
    bool setSetting(const QString &key, const QString &value, QString *error = nullptr);
    QString projectPreference(qint64 projectId, const QString &key) const;
    bool setProjectPreference(qint64 projectId, const QString &key, const QString &value,
                              QString *error = nullptr);

    QString path() const;

private:
    bool execute(const QString &sql, QString *error = nullptr) const;
    QSqlDatabase m_db;
    QString m_connectionName;
};

} // namespace Artemis
