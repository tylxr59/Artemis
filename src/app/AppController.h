#pragma once

#include "git/GitService.h"
#include "models/ConversationModel.h"
#include "models/ProjectTreeModel.h"
#include "persistence/Database.h"
#include "domain/AgentProvider.h"

#include <QElapsedTimer>
#include <QHash>
#include <QObject>
#include <QSet>
#include <QTimer>
#include <QVariantList>

#include <memory>

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
    Q_PROPERTY(bool threadCreationPending READ threadCreationPending NOTIFY threadCreationPendingChanged)
    Q_PROPERTY(QString currentTasks READ currentTasks NOTIFY currentTasksChanged)
    Q_PROPERTY(QVariantList currentPlan READ currentPlan NOTIFY currentPlanChanged)
    Q_PROPERTY(QString currentPlanExplanation READ currentPlanExplanation NOTIFY currentPlanChanged)
    Q_PROPERTY(bool turnRunning READ turnRunning NOTIFY turnRunningChanged)
    Q_PROPERTY(QStringList workingThreadIds READ workingThreadIds NOTIFY workingThreadsChanged)
    Q_PROPERTY(QStringList completedThreadIds READ completedThreadIds NOTIFY completedThreadsChanged)
    Q_PROPERTY(bool hasPendingUserInput READ hasPendingUserInput NOTIFY pendingUserInputChanged)
    Q_PROPERTY(QVariantMap pendingUserInputQuestion READ pendingUserInputQuestion NOTIFY pendingUserInputChanged)
    Q_PROPERTY(int pendingUserInputQuestionNumber READ pendingUserInputQuestionNumber NOTIFY pendingUserInputChanged)
    Q_PROPERTY(int pendingUserInputQuestionCount READ pendingUserInputQuestionCount NOTIFY pendingUserInputChanged)
    Q_PROPERTY(QString turnElapsedText READ turnElapsedText NOTIFY turnElapsedChanged)
    Q_PROPERTY(bool providerReady READ providerReady NOTIFY providerReadyChanged)
    Q_PROPERTY(bool providerSetupRequired READ providerSetupRequired NOTIFY providerSetupChanged)
    Q_PROPERTY(QString providerSetupInstructions READ providerSetupInstructions NOTIFY providerSetupChanged)
    Q_PROPERTY(QString providerIssueText READ providerIssueText NOTIFY providerIssueChanged)
    Q_PROPERTY(QString providerVersion READ providerVersion NOTIFY providerReadyChanged)
    Q_PROPERTY(QVariantList mcpServers READ mcpServers NOTIFY mcpServersChanged)
    Q_PROPERTY(bool mcpBusy READ mcpBusy NOTIFY mcpServersChanged)
    Q_PROPERTY(QString mcpIssueText READ mcpIssueText NOTIFY mcpServersChanged)
    Q_PROPERTY(QString mcpLoginUrl READ mcpLoginUrl NOTIFY mcpServersChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(qint64 contextTokens READ contextTokens NOTIFY tokenUsageChanged)
    Q_PROPERTY(qint64 totalProcessedTokens READ totalProcessedTokens NOTIFY tokenUsageChanged)
    Q_PROPERTY(qint64 modelContextWindow READ modelContextWindow NOTIFY tokenUsageChanged)
    Q_PROPERTY(int contextUsagePercent READ contextUsagePercent NOTIFY tokenUsageChanged)
    Q_PROPERTY(bool hasTokenUsage READ hasTokenUsage NOTIFY tokenUsageChanged)
    Q_PROPERTY(QString diffText READ diffText NOTIFY diffChanged)
    Q_PROPERTY(QString gitStatusText READ gitStatusText NOTIFY diffChanged)
    Q_PROPERTY(QString gitRepositoryUrl READ gitRepositoryUrl NOTIFY gitRepositoryUrlChanged)
    Q_PROPERTY(bool hasGitChanges READ hasGitChanges NOTIFY diffChanged)
    Q_PROPERTY(QString databasePath READ databasePath CONSTANT)
public:
    explicit AppController(QObject *parent = nullptr);
    explicit AppController(AgentProvider *provider, QObject *parent = nullptr);

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
    bool threadCreationPending() const;
    QString currentTasks() const;
    QVariantList currentPlan() const;
    QString currentPlanExplanation() const;
    bool turnRunning() const;
    QStringList workingThreadIds() const;
    QStringList completedThreadIds() const;
    bool hasPendingUserInput() const;
    QVariantMap pendingUserInputQuestion() const;
    int pendingUserInputQuestionNumber() const;
    int pendingUserInputQuestionCount() const;
    QString turnElapsedText() const;
    bool providerReady() const;
    bool providerSetupRequired() const;
    QString providerSetupInstructions() const;
    QString providerIssueText() const;
    QString providerVersion() const;
    QVariantList mcpServers() const;
    bool mcpBusy() const;
    QString mcpIssueText() const;
    QString mcpLoginUrl() const;
    QString statusText() const;
    qint64 contextTokens() const;
    qint64 totalProcessedTokens() const;
    qint64 modelContextWindow() const;
    int contextUsagePercent() const;
    bool hasTokenUsage() const;
    QString diffText() const;
    QString gitStatusText() const;
    QString gitRepositoryUrl() const;
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
    Q_INVOKABLE bool answerPendingUserInput(const QString &answer);
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
    Q_INVOKABLE void refreshMcpServers();
    Q_INVOKABLE void reloadMcpServers();
    Q_INVOKABLE void loginMcpServer(const QString &name);
    Q_INVOKABLE void addMcpServer(const QString &name, const QString &transport,
                                  const QString &target);
    Q_INVOKABLE void removeMcpServer(const QString &name);
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
    void threadCreationPendingChanged();
    void currentTasksChanged();
    void currentPlanChanged();
    void taskPanelRequested();
    void turnRunningChanged();
    void workingThreadsChanged();
    void completedThreadsChanged();
    void pendingUserInputChanged();
    void turnElapsedChanged();
    void providerReadyChanged();
    void providerSetupChanged();
    void providerIssueChanged();
    void mcpServersChanged();
    void statusTextChanged();
    void statusMessage(const QString &text);
    void tokenUsageChanged();
    void diffChanged();
    void gitRepositoryUrlChanged();
    void commitDraftFinished(bool success, const QString &message);
    void commitFinished(bool success, const QString &message);
    void commitLockBlocked(const QString &message);
    void promptRestoreRequested(const QString &text, const QVariantList &images);

private:
    void connectProvider();
    void loadProjects();
    void activateProject(int index, const QString &threadToSelect = {});
    void loadThreads(const QString &threadToSelect = {});
    void loadModels();
    bool loadPersistedConversationEvents(const QString &threadId);
    void setStatus(const QString &text);
    void setTurnRunning(const QString &threadId, bool running);
    void markThreadViewed(const QString &threadId);
    void handleDomainEvent(const QString &threadId, const QString &type,
                           const QString &title, const QString &content,
                           const QVariantMap &metadata);
    QString createPendingThread(const QString &modelId, const QString &prompt,
                                const QStringList &images);
    void clearPendingThread(const QString &threadId);
    bool isPendingThreadId(const QString &threadId) const;
    void beginThread(const QString &modelId, const QString &reasoningEffort,
                     PermissionProfile permissionProfile);
    void startPromptTurn(const QString &threadId, const QString &prompt,
                         const QStringList &images, const QString &modelId,
                         const QString &reasoningEffort, const QString &collaborationMode,
                         PermissionProfile permissionProfile, bool generateTitle = false);
    void setActiveTurnId(const QString &threadId, const QString &turnId);
    void handleUserInputRequest(const QString &threadId, const QString &turnId,
                                const QString &itemId, const QVariantList &questions);
    void clearPendingUserInput();
    void sendPendingSteers(const QString &threadId);
    void restorePendingSteers(const QString &threadId);
    void generateThreadTitle(const QString &threadId, const QString &prompt,
                             const QString &projectPath, const QString &workspacePath,
                             const QString &modelId);
    void applyThreadTitle(const QString &threadId, const QString &title);
    void setModelSetting(const QString &key, QString &storage, const QString &modelId);
    PermissionProfile permissionProfile(const QString &mode) const;
    QString selectedWorkspacePath() const;
    QString commitPrompt(const QByteArray &snapshot, const QString &projectName,
                         const QString &branch) const;
    QString cleanCommitDraft(const QString &raw) const;
    QString cleanTitleDraft(const QString &raw) const;
    void persistConversationEvent(const ConversationEvent &event,
                                  const QString &contentOverride = {});
    void runCodexMcpCommand(const QStringList &arguments, const QString &successMessage);

    Database m_database;
    ProjectTreeModel m_projects;
    ConversationModel m_conversation;
    std::unique_ptr<AgentProvider> m_ownedProvider;
    AgentProvider *m_provider = nullptr;
    bool m_notificationsEnabled = true;
    GitService m_git;
    QVariantList m_threads;
    QVariantList m_models;
    int m_selectedProject = -1;
    int m_selectedThread = -1;
    QTimer m_turnElapsedUpdateTimer;
    QString m_status;
    QString m_providerIssue;
    QVariantList m_mcpServers;
    bool m_mcpBusy = false;
    QString m_mcpIssue;
    QString m_mcpLoginUrl;
    QHash<QString, QVariantMap> m_threadTokenUsage;
    QString m_diff;
    QString m_gitStatus;
    QString m_gitRepositoryUrl;
    bool m_hasGitChanges = false;
    QString m_commitThreadId;
    QString m_commitDraftBuffer;
    bool m_commitDraftRunning = false;
    QHash<QString, QString> m_titleTargets;
    QHash<QString, QString> m_titleDraftBuffers;
    QHash<QString, QString> m_assistantDraftBuffers;
    struct ActiveTurn {
        QString turnId;
        QElapsedTimer elapsedTimer;
        QString threadTitle;
        QString projectName;
    };
    QHash<QString, ActiveTurn> m_activeTurns;
    QSet<QString> m_completedThreads;
    QString m_pendingUserInputThreadId;
    QString m_pendingUserInputTurnId;
    QString m_pendingUserInputItemId;
    QVariantList m_pendingUserInputQuestions;
    QVariantMap m_pendingUserInputAnswers;
    int m_pendingUserInputIndex = -1;
    struct PendingSteer {
        QString prompt;
        QStringList images;
    };
    QHash<QString, QList<PendingSteer>> m_pendingSteers;
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
    QString m_pendingThreadId;
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
