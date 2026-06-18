#pragma once

#include "domain/AgentProvider.h"

#include <QHash>
#include <QJsonObject>
#include <QProcess>
#include <QTimer>

namespace Artemis {

class CodexClient final : public AgentProvider {
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(QString version READ version NOTIFY versionChanged)
public:
    explicit CodexClient(QObject *parent = nullptr);
    ~CodexClient() override;

    ProviderCapabilities capabilities() const override;
    void start() override;
    void listModels(ResultHandler handler) override;
    void listThreads(const QString &workspacePath, ResultHandler handler) override;
    void startThread(const ThreadConfiguration &configuration, ResultHandler handler) override;
    void resumeThread(const QString &threadId, ResultHandler handler) override;
    void sendTurn(const QString &threadId, const QString &text,
                  const QStringList &images, PermissionProfile permissionProfile,
                  ResultHandler handler) override;
    void steerTurn(const QString &threadId, const QString &text, ResultHandler handler) override;
    void interruptTurn(const QString &threadId, ResultHandler handler) override;

    bool ready() const;
    QString version() const;

    void request(const QString &method, const QJsonObject &params, ResultHandler handler = {});
    static QString itemContent(const QJsonObject &item);

signals:
    void versionChanged();
    void rawNotification(const QString &method, const QJsonObject &params);

private:
    struct Pending {
        ResultHandler handler;
        QTimer *timeout = nullptr;
    };

    void startVersionProbe();
    void startProcess();
    void scheduleRestart(const QString &reason);
    void handleLine(const QByteArray &line);
    void handleNotification(const QString &method, const QJsonObject &params);
    void normalizeItem(const QString &lifecycle, const QJsonObject &params);
    void setReady(bool ready);

    QProcess m_process;
    QByteArray m_buffer;
    QHash<qint64, Pending> m_pending;
    qint64 m_nextId = 1;
    bool m_ready = false;
    bool m_stopping = false;
    int m_restartAttempt = 0;
    QString m_version;
};

} // namespace Artemis
