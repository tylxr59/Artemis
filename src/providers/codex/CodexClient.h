#pragma once

#include "domain/AgentProvider.h"

#include <QHash>
#include <QJsonObject>
#include <QProcess>
#include <QTimer>

#include <functional>

namespace Artemis {

class CodexClient final : public AgentProvider {
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(QString version READ version NOTIFY versionChanged)
public:
    explicit CodexClient(QObject *parent = nullptr);
    ~CodexClient() override;

    ProviderCapabilities capabilities() const override;
    bool ready() const override;
    bool setupRequired() const override;
    QString setupInstructions() const override;
    QString version() const override;
    void start() override;
    void listModels(ResultHandler handler) override;
    void listThreads(const QString &workspacePath, ResultHandler handler) override;
    void startThread(const ThreadConfiguration &configuration, ResultHandler handler) override;
    void resumeThread(const QString &threadId, ResultHandler handler) override;
    void sendTurn(const QString &threadId, const QString &text,
                  const QStringList &images, const QString &modelId,
                  const QString &reasoningEffort, const QString &collaborationMode,
                  PermissionProfile permissionProfile,
                  ResultHandler handler) override;
    void steerTurn(const QString &threadId, const QString &turnId,
                   const QString &text,
                   const QStringList &images, ResultHandler handler) override;
    void interruptTurn(const QString &threadId, const QString &turnId,
                       ResultHandler handler) override;
    void respondToUserInput(const QString &itemId, const QVariantMap &answers,
                            ResultHandler handler) override;
    void setThreadName(const QString &threadId, const QString &name,
                       ResultHandler handler) override;
    void listMcpServers(ResultHandler handler) override;
    void reloadMcpServers(ResultHandler handler) override;
    void loginMcpServer(const QString &name, ResultHandler handler) override;
    void addMcpServer(const QString &name, const QString &transport,
                      const QString &target, const QString &bearerToken,
                      ResultHandler handler) override;
    void removeMcpServer(const QString &name, ResultHandler handler) override;
    QString itemContent(const QJsonObject &item) const override;

    void request(const QString &method, const QJsonObject &params, ResultHandler handler = {});

private:
    struct Pending {
        ResultHandler handler;
        QTimer *timeout = nullptr;
    };

    void startVersionProbe();
    void startProcess();
    void initializeProcess();
    void refreshAccountState();
    void requireAuthentication();
    void scheduleRestart(const QString &reason);
    bool isAuthenticationError(const QString &message) const;
    void handleLine(const QByteArray &line);
    void handleNotification(const QString &method, const QJsonObject &params);
    void handleServerRequest(const QJsonValue &id, const QString &method,
                             const QJsonObject &params);
    void normalizeItem(const QString &lifecycle, const QJsonObject &params);
    void writeResponse(const QJsonValue &id, const QJsonObject &result);
    void runMcpCommand(const QStringList &arguments, ResultHandler handler,
                       std::function<bool(QString *)> afterSuccess = {});
    void setReady(bool ready);
    void setSetupRequired(bool required, const QString &instructions = {});

    QProcess m_process;
    QTimer m_restartTimer;
    QByteArray m_buffer;
    QHash<qint64, Pending> m_pending;
    QHash<QString, QJsonValue> m_pendingUserInputRequests;
    QHash<QString, QJsonValue> m_pendingMcpElicitationRequests;
    QHash<QString, QJsonObject> m_pendingMcpElicitationParams;
    qint64 m_nextId = 1;
    bool m_ready = false;
    bool m_setupRequired = false;
    bool m_stopping = false;
    bool m_restartScheduled = false;
    int m_restartAttempt = 0;
    QString m_version;
    QString m_setupInstructions;
};

} // namespace Artemis
