#pragma once

#include "git/GitService.h"
#include "models/ConversationModel.h"
#include "models/ProjectTreeModel.h"
#include "persistence/Database.h"
#include "providers/codex/CodexClient.h"

#include <QElapsedTimer>
#include <QHash>
#include <QObject>
#include <QTimer>
#include <QVariantList>

namespace Artemis {

class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(ProjectTreeModel *projects READ projects CONSTANT)
    Q_PROPERTY(ConversationModel *conversation READ conversation CONSTANT)
    Q_PROPERTY(QVariantList threads READ threads NOTIFY threadsChanged)
    Q_PROPERTY(QVariantList models READ models NOTIFY modelsChanged)
    Q_PROPERTY(QString codingModelId READ codingModelId WRITE setCodingModelId NOTIFY settingsChanged)
    Q_PROPERTY(QString codingReasoningEffort READ codingReasoningEffort WRITE setCodingReasoningEffort NOTIFY settingsChanged)
    Q_PROPERTY(QString commitModelId READ commitModelId WRITE setCommitModelId NOTIFY settingsChanged)
    Q_PROPERTY(QString titleModelId READ titleModelId WRITE setTitleModelId NOTIFY settingsChanged)
    Q_PROPERTY(QVariantList editorOptions READ editorOptions CONSTANT)
    Q_PROPERTY(QString selectedEditorId READ selectedEditorId WRITE setSelectedEditorId NOTIFY settingsChanged)
    Q_PROPERTY(QVariantList terminalOptions READ terminalOptions CONSTANT)
    Q_PROPERTY(QString selectedTerminalId READ selectedTerminalId WRITE setSelectedTerminalId NOTIFY settingsChanged)
    Q_PROPERTY(int selectedProjectIndex READ selectedProjectIndex WRITE selectProject NOTIFY selectedProjectChanged)
    Q_PROPERTY(QString selectedProjectPath READ selectedProjectPath NOTIFY selectedProjectChanged)
    Q_PROPERTY(QString selectedProjectName READ selectedProjectName NOTIFY selectedProjectChanged)
    Q_PROPERTY(bool selectedProjectIsGit READ selectedProjectIsGit NOTIFY selectedProjectChanged)
    Q_PROPERTY(QString selectedThreadId READ selectedThreadId NOTIFY selectedThreadChanged)
    Q_PROPERTY(QString selectedThreadTitle READ selectedThreadTitle NOTIFY selectedThreadChanged)
    Q_PROPERTY(QVariantMap selectedThreadInfo READ selectedThreadInfo NOTIFY selectedThreadChanged)
    Q_PROPERTY(QString currentTasks READ currentTasks NOTIFY currentTasksChanged)
    Q_PROPERTY(QVariantList currentPlan READ currentPlan NOTIFY currentPlanChanged)
    Q_PROPERTY(QString currentPlanExplanation READ currentPlanExplanation NOTIFY currentPlanChanged)
    Q_PROPERTY(bool turnRunning READ turnRunning NOTIFY turnRunningChanged)
    Q_PROPERTY(QString turnElapsedText READ turnElapsedText NOTIFY turnElapsedChanged)
    Q_PROPERTY(bool providerReady READ providerReady NOTIFY providerReadyChanged)
    Q_PROPERTY(QString providerVersion READ providerVersion NOTIFY providerReadyChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(qint64 contextTokens READ contextTokens NOTIFY tokenUsageChanged)
    Q_PROPERTY(qint64 totalProcessedTokens READ totalProcessedTokens NOTIFY tokenUsageChanged)
    Q_PROPERTY(qint64 modelContextWindow READ modelContextWindow NOTIFY tokenUsageChanged)
    Q_PROPERTY(int contextUsagePercent READ contextUsagePercent NOTIFY tokenUsageChanged)
    Q_PROPERTY(bool hasTokenUsage READ hasTokenUsage NOTIFY tokenUsageChanged)
    Q_PROPERTY(QString diffText READ diffText NOTIFY diffChanged)
    Q_PROPERTY(QString gitStatusText READ gitStatusText NOTIFY diffChanged)
    Q_PROPERTY(bool hasGitChanges READ hasGitChanges NOTIFY diffChanged)
    Q_PROPERTY(QString databasePath READ databasePath CONSTANT)
public:
    explicit AppController(QObject *parent = nullptr);

    bool initialize();
    ProjectTreeModel *projects();
    ConversationModel *conversation();
    QVariantList threads() const;
    QVariantList models() const;
    QString codingModelId() const;
    QString codingReasoningEffort() const;
    QString commitModelId() const;
    QString titleModelId() const;
    QVariantList editorOptions() const;
    QString selectedEditorId() const;
    QVariantList terminalOptions() const;
    QString selectedTerminalId() const;
    int selectedProjectIndex() const;
    QString selectedProjectPath() const;
    QString selectedProjectName() const;
    bool selectedProjectIsGit() const;
    QString selectedThreadId() const;
    QString selectedThreadTitle() const;
    QVariantMap selectedThreadInfo() const;
    QString currentTasks() const;
    QVariantList currentPlan() const;
    QString currentPlanExplanation() const;
    bool turnRunning() const;
    QString turnElapsedText() const;
    bool providerReady() const;
    QString providerVersion() const;
    QString statusText() const;
    qint64 contextTokens() const;
    qint64 totalProcessedTokens() const;
    qint64 modelContextWindow() const;
    int contextUsagePercent() const;
    bool hasTokenUsage() const;
    QString diffText() const;
    QString gitStatusText() const;
    bool hasGitChanges() const;
    QString databasePath() const;

    Q_INVOKABLE void chooseProjectFolder();
    Q_INVOKABLE void addProject(const QString &input);
    Q_INVOKABLE void removeSelectedProject();
    Q_INVOKABLE void removeProject(int index);
    Q_INVOKABLE void removeThread(int index);
    Q_INVOKABLE void removeProjectThread(int projectIndex, const QString &threadId);
    Q_INVOKABLE void selectProject(int index);
    Q_INVOKABLE void selectProjectThread(int projectIndex, const QString &threadId);
    Q_INVOKABLE void selectThread(int index);
    Q_INVOKABLE void createThread(const QString &modelId, const QString &reasoningEffort,
                                  const QString &permissionMode);
    Q_INVOKABLE bool sendPrompt(const QString &text, const QVariantList &imageValues,
                                const QString &modelId, const QString &reasoningEffort,
                                const QString &permissionMode,
                                const QString &collaborationMode);
    Q_INVOKABLE void copyText(const QString &text);
    Q_INVOKABLE QString pasteClipboardImage();
    Q_INVOKABLE void interruptTurn();
    Q_INVOKABLE void refreshGit();
    Q_INVOKABLE void generateCommitMessage();
    Q_INVOKABLE void commitAllAndPush(const QString &subject, const QString &body);
    Q_INVOKABLE void commitFeatureBranch(const QString &subject, const QString &body,
                                         const QString &branch, const QString &remote);
    Q_INVOKABLE void retryLockedCommit();
    Q_INVOKABLE void removeCommitLockAndRetry();
    Q_INVOKABLE void cancelLockedCommit();
    Q_INVOKABLE QString suggestBranch(const QString &message) const;
    Q_INVOKABLE void openProjectFolder();
    Q_INVOKABLE void openProjectEditor();
    Q_INVOKABLE void openTerminal();
    void setCodingModelId(const QString &modelId);
    void setCodingReasoningEffort(const QString &reasoningEffort);
    void setCommitModelId(const QString &modelId);
    void setTitleModelId(const QString &modelId);
    void setSelectedEditorId(const QString &desktopId);
    void setSelectedTerminalId(const QString &desktopId);

signals:
    void threadsChanged();
    void projectThreadsLoaded(const QString &projectPath, const QVariantList &threads);
    void projectThreadRemoved(const QString &projectPath, const QString &threadId);
    void modelsChanged();
    void settingsChanged();
    void selectedProjectChanged();
    void selectedThreadChanged();
    void currentTasksChanged();
    void currentPlanChanged();
    void taskPanelRequested();
    void turnRunningChanged();
    void turnElapsedChanged();
    void providerReadyChanged();
    void statusTextChanged();
    void statusMessage(const QString &text);
    void tokenUsageChanged();
    void diffChanged();
    void commitDraftFinished(bool success, const QString &message);
    void commitFinished(bool success, const QString &message);
    void commitLockBlocked(const QString &message);
    void promptRestoreRequested(const QString &text, const QVariantList &images);

private:
    void loadProjects();
    void activateProject(int index, const QString &threadToSelect = {});
    void loadThreads(const QString &threadToSelect = {});
    void loadModels();
    void setStatus(const QString &text);
    void setTurnRunning(bool running);
    void handleDomainEvent(const QString &threadId, const QString &type,
                           const QString &title, const QString &content,
                           const QVariantMap &metadata);
    void beginThread(const QString &modelId, const QString &reasoningEffort,
                     PermissionProfile permissionProfile);
    void startPromptTurn(const QString &threadId, const QString &prompt,
                         const QStringList &images, const QString &modelId,
                         const QString &reasoningEffort, const QString &collaborationMode,
                         PermissionProfile permissionProfile, bool generateTitle = false);
    void generateThreadTitle(const QString &threadId, const QString &prompt);
    void applyThreadTitle(const QString &threadId, const QString &title);
    void setModelSetting(const QString &key, QString &storage, const QString &modelId);
    PermissionProfile permissionProfile(const QString &mode) const;
    QString selectedWorkspacePath() const;
    QString commitPrompt(const QByteArray &snapshot) const;
    QString cleanCommitDraft(const QString &raw) const;
    QString cleanTitleDraft(const QString &raw) const;
    void persistConversationEvent(const ConversationEvent &event,
                                  const QString &contentOverride = {});

    Database m_database;
    ProjectTreeModel m_projects;
    ConversationModel m_conversation;
    CodexClient m_codex;
    GitService m_git;
    QVariantList m_threads;
    QVariantList m_models;
    int m_selectedProject = -1;
    int m_selectedThread = -1;
    bool m_turnRunning = false;
    QElapsedTimer m_turnElapsedTimer;
    QTimer m_turnElapsedUpdateTimer;
    QString m_status;
    QHash<QString, QVariantMap> m_threadTokenUsage;
    QString m_diff;
    QString m_gitStatus;
    bool m_hasGitChanges = false;
    QString m_commitThreadId;
    QString m_commitDraftBuffer;
    QHash<QString, QString> m_titleTargets;
    QHash<QString, QString> m_titleDraftBuffers;
    QHash<QString, QString> m_assistantDraftBuffers;
    QString m_activeThreadId;
    QHash<QString, QVariantList> m_threadPlans;
    QHash<QString, QString> m_threadPlanExplanations;
    QHash<QString, QString> m_threadTasks;
    QString m_pendingPrompt;
    QStringList m_pendingImages;
    QString m_pendingModelId;
    QString m_pendingReasoningEffort;
    QString m_pendingCollaborationMode = QStringLiteral("default");
    PermissionProfile m_pendingPermissionProfile = PermissionProfile::FullAccess;
    bool m_threadCreationPending = false;
    QString m_codingModelId;
    QString m_codingReasoningEffort;
    QString m_commitModelId;
    QString m_titleModelId;
    QVariantList m_editorOptions;
    QString m_selectedEditorId;
    QVariantList m_terminalOptions;
    QString m_selectedTerminalId;
};

} // namespace Artemis
