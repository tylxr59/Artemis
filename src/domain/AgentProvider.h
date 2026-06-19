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

struct AgentThread {
    QString id;
    QString title;
    QString cwd;
    QString model;
    QString updatedAt;
    bool external = false;
};

struct ModelInfo {
    QString id;
    QString displayName;
    QString description;
    QString defaultEffort;
    QStringList efforts;
    bool isDefault = false;
};

class AgentProvider : public QObject {
    Q_OBJECT
public:
    using ResultHandler = std::function<void(const QJsonObject &, const QString &)>;

    explicit AgentProvider(QObject *parent = nullptr) : QObject(parent) {}
    ~AgentProvider() override = default;

    virtual ProviderCapabilities capabilities() const = 0;
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
    virtual void steerTurn(const QString &threadId, const QString &text,
                           const QStringList &images, ResultHandler handler) = 0;
    virtual void interruptTurn(const QString &threadId, ResultHandler handler) = 0;

signals:
    void readyChanged(bool ready);
    void domainEvent(const QString &threadId, const QString &type,
                     const QString &title, const QString &content,
                     const QVariantMap &metadata);
    void providerError(const QString &message);
};

} // namespace Artemis
