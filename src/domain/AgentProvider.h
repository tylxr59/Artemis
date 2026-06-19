#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>

#include <functional>

namespace Artemis {

struct ProviderCapabilities {
    bool persistentThreads = false;
    bool approvals = false;
    bool imageInput = false;
    bool modelSelection = false;
    bool reasoningEffort = false;
    bool streamedDiffs = false;
    bool interruption = false;
};

enum class PermissionProfile {
    ReadOnly,
    WorkspaceWrite,
    FullAccess,
};

struct ThreadConfiguration {
    QString projectPath;
    QString workspacePath;
    QString modelId;
    QString reasoningEffort;
    PermissionProfile permissionProfile = PermissionProfile::FullAccess;
    bool ephemeral = false;
};

class AgentProvider : public QObject {
    Q_OBJECT
public:
    using ResultHandler = std::function<void(const QJsonObject &, const QString &)>;

    explicit AgentProvider(QObject *parent = nullptr) : QObject(parent) {}
    ~AgentProvider() override = default;

    virtual ProviderCapabilities capabilities() const = 0;
    virtual bool ready() const = 0;
    virtual bool setupRequired() const { return false; }
    virtual QString setupInstructions() const { return {}; }
    virtual QString version() const = 0;
    virtual void start() = 0;
    virtual void listModels(ResultHandler handler) = 0;
    virtual void listThreads(const QString &workspacePath, ResultHandler handler) = 0;
    virtual void startThread(const ThreadConfiguration &configuration, ResultHandler handler) = 0;
    virtual void resumeThread(const QString &threadId, ResultHandler handler) = 0;
    virtual void sendTurn(const QString &threadId, const QString &text,
                          const QStringList &images, const QString &modelId,
                          const QString &reasoningEffort, const QString &collaborationMode,
                          PermissionProfile permissionProfile,
                          ResultHandler handler) = 0;
    virtual void steerTurn(const QString &threadId, const QString &turnId,
                           const QString &text,
                           const QStringList &images, ResultHandler handler) = 0;
    virtual void interruptTurn(const QString &threadId, const QString &turnId,
                               ResultHandler handler) = 0;
    virtual void respondToUserInput(const QString &itemId, const QVariantMap &answers,
                                    ResultHandler handler) = 0;
    virtual void setThreadName(const QString &threadId, const QString &name,
                               ResultHandler handler) = 0;
    virtual QString itemContent(const QJsonObject &item) const = 0;

signals:
    void readyChanged(bool ready);
    void setupChanged();
    void versionChanged();
    void activeTurnStarted(const QString &threadId, const QString &turnId);
    void tokenUsageUpdated(const QString &threadId, qint64 contextTokens,
                           qint64 totalProcessedTokens, qint64 modelContextWindow);
    void userInputRequested(const QString &threadId, const QString &turnId,
                            const QString &itemId, const QVariantList &questions);
    void domainEvent(const QString &threadId, const QString &type,
                     const QString &title, const QString &content,
                     const QVariantMap &metadata);
    void providerError(const QString &message);
};

} // namespace Artemis
