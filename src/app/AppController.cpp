#include "app/AppController.h"
#include "platform/DesktopIntegration.h"
#include "platform/Paths.h"
#include "providers/codex/CodexClient.h"

#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QMimeData>
#include <QTextStream>
#include <QTime>
#include <QUrl>
#include <QUuid>

#include <utility>

namespace Artemis {

namespace {

QJsonObject commitDraftObject(const QString &text)
{
    bool inString = false;
    bool escaped = false;
    int depth = 0;
    qsizetype start = -1;
    QJsonObject result;

    for (qsizetype i = 0; i < text.size(); ++i) {
        const auto character = text.at(i);
        if (inString) {
            if (escaped) {
                escaped = false;
            } else if (character == QChar(u'\\')) {
                escaped = true;
            } else if (character == QChar(u'"')) {
                inString = false;
            }
            continue;
        }
        if (character == QChar(u'"')) {
            inString = true;
        } else if (character == QChar(u'{')) {
            if (depth++ == 0)
                start = i;
        } else if (character == QChar(u'}') && depth > 0) {
            --depth;
            if (depth != 0 || start < 0)
                continue;
            const auto candidate = text.mid(start, i - start + 1);
            const auto document = QJsonDocument::fromJson(candidate.toUtf8());
            if (document.isObject()
                && document.object().contains(QStringLiteral("subject"))) {
                result = document.object();
            }
            start = -1;
        }
    }
    return result;
}

QString elapsedText(qint64 elapsedMilliseconds)
{
    const qint64 totalSeconds = qMax<qint64>(0, elapsedMilliseconds / 1000);
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 seconds = totalSeconds % 60;
    if (hours > 0)
        return QStringLiteral("%1h %2m %3s").arg(hours).arg(minutes).arg(seconds);
    if (minutes > 0)
        return QStringLiteral("%1m %2s").arg(minutes).arg(seconds);
    return QStringLiteral("%1s").arg(seconds);
}

bool removeOrphanedAttachments(const Database &database, QString *error)
{
    const auto referenced = database.referencedAttachmentPaths(error);
    if (error && !error->isEmpty())
        return false;
    QDir directory(Paths::attachmentRoot());
    const auto cutoff = QDateTime::currentDateTimeUtc().addDays(-1);
    for (const auto &info : directory.entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
        if (!referenced.contains(info.absoluteFilePath())
            && info.lastModified().toUTC() < cutoff) {
            QFile::remove(info.absoluteFilePath());
        }
    }
    return true;
}

} // namespace

AppController::AppController(QObject *parent)
    : QObject(parent),
      m_ownedProvider(std::make_unique<CodexClient>()),
      m_provider(m_ownedProvider.get())
{
    connectProvider();
}

AppController::AppController(AgentProvider *provider, QObject *parent)
    : QObject(parent),
      m_provider(provider)
{
    Q_ASSERT(m_provider);
    connectProvider();
}

void AppController::connectProvider()
{
    m_turnElapsedUpdateTimer.setInterval(1000);
    connect(&m_turnElapsedUpdateTimer, &QTimer::timeout,
            this, &AppController::turnElapsedChanged);
    connect(m_provider, &AgentProvider::readyChanged, this, [this](bool ready) {
        emit providerReadyChanged();
        if (ready) {
            if (!m_providerIssue.isEmpty()) {
                m_providerIssue.clear();
                emit providerIssueChanged();
            }
            setStatus(QStringLiteral("Codex connected"));
            loadModels();
            loadThreads();
        }
    });
    connect(m_provider, &AgentProvider::setupChanged, this, [this] {
        emit providerSetupChanged();
    });
    connect(m_provider, &AgentProvider::versionChanged,
            this, &AppController::providerReadyChanged);
    connect(m_provider, &AgentProvider::providerError, this, [this](const QString &message) {
        if (m_providerIssue != message) {
            m_providerIssue = message;
            emit providerIssueChanged();
        }
        setStatus(message);
    });
    connect(m_provider, &AgentProvider::domainEvent, this, &AppController::handleDomainEvent);
    connect(m_provider, &AgentProvider::activeTurnStarted,
            this, &AppController::setActiveTurnId);
    connect(m_provider, &AgentProvider::userInputRequested,
            this, &AppController::handleUserInputRequest);
    connect(m_provider, &AgentProvider::tokenUsageUpdated, this,
            [this](const QString &threadId, qint64 contextTokens,
                   qint64 totalProcessedTokens, qint64 modelContextWindow) {
        if (threadId.isEmpty())
            return;
        m_threadTokenUsage.insert(threadId, {
            {QStringLiteral("contextTokens"), contextTokens},
            {QStringLiteral("totalProcessedTokens"), totalProcessedTokens},
            {QStringLiteral("modelContextWindow"), modelContextWindow}
        });
        if (threadId == selectedThreadId())
            emit tokenUsageChanged();
    });
}

bool AppController::initialize()
{
    QString error;
    if (!Paths::ensureRuntimeDirectories(&error)) {
        setStatus(error);
        return false;
    }
    if (!m_database.open(&error)) {
        setStatus(QStringLiteral("Database error: %1").arg(error));
        return false;
    }
    if (!removeOrphanedAttachments(m_database, &error)) {
        setStatus(QStringLiteral("Could not inspect stored attachments: %1").arg(error));
        return false;
    }
    m_codingModelId = m_database.setting(QStringLiteral("coding_model"));
    m_codingReasoningEffort = m_database.setting(QStringLiteral("coding_reasoning_effort"));
    m_commitModelId = m_database.setting(QStringLiteral("commit_model"));
    m_titleModelId = m_database.setting(QStringLiteral("title_model"));
    m_editorOptions = DesktopIntegration::availableEditors();
    m_selectedEditorId = m_database.setting(QStringLiteral("editor_desktop_id"));
    m_terminalOptions = DesktopIntegration::availableTerminals();
    m_selectedTerminalId = m_database.setting(QStringLiteral("terminal_desktop_id"));
    loadProjects();
    m_provider->start();
    return true;
}

ProjectTreeModel *AppController::projects() { return &m_projects; }
ConversationModel *AppController::conversation() { return &m_conversation; }
QVariantList AppController::threads() const { return m_threads; }
QVariantList AppController::models() const { return m_models; }
QString AppController::codingModelId() const { return m_codingModelId; }
QString AppController::codingReasoningEffort() const { return m_codingReasoningEffort; }
QString AppController::commitModelId() const { return m_commitModelId; }
QString AppController::titleModelId() const { return m_titleModelId; }
QVariantList AppController::editorOptions() const { return m_editorOptions; }
QString AppController::selectedEditorId() const { return m_selectedEditorId; }
QVariantList AppController::terminalOptions() const { return m_terminalOptions; }
QString AppController::selectedTerminalId() const { return m_selectedTerminalId; }
int AppController::selectedProjectIndex() const { return m_selectedProject; }
QString AppController::selectedProjectPath() const { return m_projects.row(m_selectedProject).path; }
QString AppController::selectedProjectName() const { return m_projects.row(m_selectedProject).name; }
bool AppController::selectedProjectIsGit() const { return m_projects.row(m_selectedProject).git; }
QString AppController::selectedThreadId() const {
    return m_selectedThread >= 0 && m_selectedThread < m_threads.size()
        ? m_threads.at(m_selectedThread).toMap().value(QStringLiteral("id")).toString() : QString();
}
QString AppController::selectedThreadTitle() const {
    return m_selectedThread >= 0 && m_selectedThread < m_threads.size()
        ? m_threads.at(m_selectedThread).toMap().value(QStringLiteral("title")).toString() : QString();
}
QVariantMap AppController::selectedThreadInfo() const
{
    return m_selectedThread >= 0 && m_selectedThread < m_threads.size()
        ? m_threads.at(m_selectedThread).toMap() : QVariantMap{};
}
QString AppController::currentTasks() const
{
    return m_threadTasks.value(selectedThreadId());
}
QVariantList AppController::currentPlan() const
{
    return m_threadPlans.value(selectedThreadId());
}
QString AppController::currentPlanExplanation() const
{
    return m_threadPlanExplanations.value(selectedThreadId());
}
bool AppController::turnRunning() const
{
    return m_activeTurns.contains(selectedThreadId());
}
bool AppController::hasPendingUserInput() const
{
    return m_pendingUserInputIndex >= 0
        && m_pendingUserInputIndex < m_pendingUserInputQuestions.size();
}
QVariantMap AppController::pendingUserInputQuestion() const
{
    if (!hasPendingUserInput())
        return {};
    auto question = m_pendingUserInputQuestions.at(m_pendingUserInputIndex).toMap();
    question.insert(QStringLiteral("threadId"), m_pendingUserInputThreadId);
    return question;
}
int AppController::pendingUserInputQuestionNumber() const
{
    return hasPendingUserInput() ? m_pendingUserInputIndex + 1 : 0;
}
int AppController::pendingUserInputQuestionCount() const
{
    return m_pendingUserInputQuestions.size();
}
QString AppController::turnElapsedText() const
{
    const auto activeTurn = m_activeTurns.constFind(selectedThreadId());
    if (activeTurn == m_activeTurns.cend())
        return elapsedText(0);
    return elapsedText(QDateTime::currentMSecsSinceEpoch() - activeTurn->startedAtMs);
}
bool AppController::providerReady() const { return m_provider->ready(); }
bool AppController::providerSetupRequired() const { return m_provider->setupRequired(); }
QString AppController::providerSetupInstructions() const
{
    return m_provider->setupInstructions();
}
QString AppController::providerIssueText() const { return m_providerIssue; }
QString AppController::providerVersion() const { return m_provider->version(); }
QString AppController::statusText() const { return m_status; }
qint64 AppController::contextTokens() const
{
    return m_threadTokenUsage.value(selectedThreadId())
        .value(QStringLiteral("contextTokens")).toLongLong();
}
qint64 AppController::totalProcessedTokens() const
{
    return m_threadTokenUsage.value(selectedThreadId())
        .value(QStringLiteral("totalProcessedTokens")).toLongLong();
}
qint64 AppController::modelContextWindow() const
{
    return m_threadTokenUsage.value(selectedThreadId())
        .value(QStringLiteral("modelContextWindow")).toLongLong();
}
int AppController::contextUsagePercent() const
{
    const auto window = modelContextWindow();
    if (window <= 0)
        return 0;
    const auto ratio = static_cast<double>(contextTokens())
        / static_cast<double>(window);
    return qBound(0, qRound(100.0 * ratio), 100);
}
bool AppController::hasTokenUsage() const
{
    return modelContextWindow() > 0;
}
QString AppController::diffText() const { return m_diff; }
QString AppController::gitStatusText() const { return m_gitStatus; }
bool AppController::hasGitChanges() const { return m_hasGitChanges; }
QString AppController::databasePath() const { return m_database.path(); }

void AppController::chooseProjectFolder()
{
    const QString path = QFileDialog::getExistingDirectory(
        nullptr,
        tr("Add project folder"),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly);
    if (!path.isEmpty())
        addProject(path);
}

void AppController::loadProjects()
{
    QVector<ProjectRow> rows;
    QString error;
    const auto projects = m_database.projects(&error);
    if (!error.isEmpty()) {
        setStatus(QStringLiteral("Could not load projects: %1").arg(error));
        return;
    }
    for (const auto &project : projects) {
        ProjectRow row;
        row.id = project.value(QStringLiteral("id")).toLongLong();
        row.path = project.value(QStringLiteral("path")).toString();
        row.name = project.value(QStringLiteral("name")).toString();
        row.git = GitService::isRepository(row.path);
        const auto bindings = m_database.threadBindings(row.id, &error);
        if (!error.isEmpty()) {
            setStatus(QStringLiteral("Could not load project threads: %1").arg(error));
            return;
        }
        row.threadCount = static_cast<int>(bindings.size());
        rows.push_back(row);
    }
    m_projects.setRows(std::move(rows));
    if (m_projects.rowCount() > 0 && m_selectedProject < 0)
        selectProject(0);
}

void AppController::addProject(const QString &input)
{
    QString path = input;
    if (path.startsWith(QStringLiteral("file:")))
        path = QUrl(path).toLocalFile();
    path = GitService::canonicalProjectPath(path);
    QFileInfo info(path);
    if (!info.exists() || !info.isDir()) {
        setStatus(QStringLiteral("Project folder does not exist"));
        return;
    }
    QString error;
    const auto id = m_database.addProject(path, info.fileName(), &error);
    if (id < 0) {
        setStatus(QStringLiteral("Could not add project: %1").arg(error));
        return;
    }
    loadProjects();
    selectProject(m_projects.indexOfId(id));
}

void AppController::removeSelectedProject()
{
    removeProject(m_selectedProject);
}

void AppController::removeProject(int index)
{
    const auto row = m_projects.row(index);
    if (row.id < 0)
        return;
    const auto selectedId = m_projects.row(m_selectedProject).id;
    const bool removedSelected = index == m_selectedProject;
    QString error;
    if (!m_database.removeProject(row.id, &error)) {
        setStatus(error);
        return;
    }
    if (removedSelected) {
        m_selectedProject = -1;
        m_selectedThread = -1;
        m_threads.clear();
        m_conversation.clear();
        emit tokenUsageChanged();
    }
    loadProjects();
    if (removedSelected) {
        if (m_projects.rowCount() == 0) {
            emit selectedProjectChanged();
            emit selectedThreadChanged();
            emit currentPlanChanged();
            emit threadsChanged();
        }
    } else {
        m_selectedProject = m_projects.indexOfId(selectedId);
        emit selectedProjectChanged();
    }
}

void AppController::removeThread(int index)
{
    if (index < 0 || index >= m_threads.size())
        return;
    const auto project = m_projects.row(m_selectedProject);
    const auto threadId = m_threads.at(index).toMap().value(QStringLiteral("id")).toString();
    QString error;
    if (!m_database.hideThread(project.id, threadId, &error)) {
        setStatus(error);
        return;
    }

    const bool removedSelected = index == m_selectedThread;
    m_threads.removeAt(index);
    emit projectThreadRemoved(project.path, threadId);
    m_threadPlans.remove(threadId);
    m_threadPlanExplanations.remove(threadId);
    m_threadTasks.remove(threadId);
    if (removedSelected) {
        m_selectedThread = -1;
        m_conversation.setThread({});
    } else if (index < m_selectedThread) {
        --m_selectedThread;
    }
    emit threadsChanged();
    emit selectedThreadChanged();
    emit currentTasksChanged();
    emit currentPlanChanged();

    if (removedSelected && !m_threads.isEmpty())
        selectThread(qMin(index, static_cast<int>(m_threads.size()) - 1));
}

void AppController::removeProjectThread(int projectIndex, const QString &threadId)
{
    if (projectIndex == m_selectedProject) {
        for (int i = 0; i < m_threads.size(); ++i) {
            if (m_threads.at(i).toMap().value(QStringLiteral("id")).toString() == threadId) {
                removeThread(i);
                return;
            }
        }
        return;
    }

    const auto project = m_projects.row(projectIndex);
    if (project.id < 0 || threadId.isEmpty())
        return;
    QString error;
    if (!m_database.hideThread(project.id, threadId, &error)) {
        setStatus(error);
        return;
    }
    emit projectThreadRemoved(project.path, threadId);
}

void AppController::selectProject(int index)
{
    activateProject(index);
}

void AppController::selectProjectThread(int projectIndex, const QString &threadId)
{
    if (projectIndex == m_selectedProject) {
        for (int i = 0; i < m_threads.size(); ++i) {
            if (m_threads.at(i).toMap().value(QStringLiteral("id")).toString() == threadId) {
                selectThread(i);
                return;
            }
        }
    }
    activateProject(projectIndex, threadId);
}

void AppController::activateProject(int index, const QString &threadToSelect)
{
    if (index < 0 || index >= m_projects.rowCount())
        return;
    m_selectedProject = index;
    m_selectedThread = -1;
    m_conversation.setThread({});
    emit selectedProjectChanged();
    emit selectedThreadChanged();
    emit turnRunningChanged();
    emit turnElapsedChanged();
    emit tokenUsageChanged();
    emit currentTasksChanged();
    emit currentPlanChanged();
    loadThreads(threadToSelect);
    refreshGit();
}

void AppController::loadThreads(const QString &threadToSelect)
{
    m_threads.clear();
    emit threadsChanged();
    const auto project = m_projects.row(m_selectedProject);
    if (project.id < 0 || !m_provider->ready())
        return;
    QString databaseError;
    const auto bindings = m_database.threadBindings(project.id, &databaseError);
    if (!databaseError.isEmpty()) {
        setStatus(QStringLiteral("Could not load thread bindings: %1").arg(databaseError));
        return;
    }
    const auto hiddenThreadIds = m_database.hiddenThreadIds(project.id, &databaseError);
    if (!databaseError.isEmpty()) {
        setStatus(QStringLiteral("Could not load hidden threads: %1").arg(databaseError));
        return;
    }
    QHash<QString, QVariantMap> bindingById;
    for (const auto &binding : bindings)
        bindingById.insert(binding.value(QStringLiteral("threadId")).toString(), binding);
    m_provider->listThreads(project.path,
        [this, project, bindingById, hiddenThreadIds, threadToSelect](
            const QJsonObject &result, const QString &error) {
        if (!error.isEmpty()) {
            setStatus(error);
            return;
        }
        QVariantList rows;
        for (const auto &entry : result.value(QStringLiteral("data")).toArray()) {
            const auto thread = entry.toObject();
            const auto id = thread.value(QStringLiteral("id")).toString();
            if (hiddenThreadIds.contains(id))
                continue;
            const auto name = thread.value(QStringLiteral("name")).toString();
            const auto title = name.isEmpty()
                ? thread.value(QStringLiteral("title")).toString(QStringLiteral("Untitled thread"))
                : name;
            const auto cwd = thread.value(QStringLiteral("cwd")).toString(project.path);
            const auto binding = bindingById.value(id);
            const bool known = !binding.isEmpty();
            rows.push_back(QVariantMap{{QStringLiteral("id"), id},
                                       {QStringLiteral("title"), title},
                                       {QStringLiteral("named"), !name.isEmpty()},
                                       {QStringLiteral("cwd"), cwd},
                                       {QStringLiteral("model"), thread.value(QStringLiteral("model")).toString()},
                                       {QStringLiteral("external"), known
                                            ? binding.value(QStringLiteral("external")).toBool() : true}});
            if (!known) {
                QString bindError;
                if (!m_database.bindThread(project.id, id, project.path, true, &bindError))
                    setStatus(QStringLiteral("Could not save thread binding: %1").arg(bindError));
            }
        }
        emit projectThreadsLoaded(project.path, rows);
        if (m_projects.row(m_selectedProject).id != project.id)
            return;
        m_threads = rows;
        emit threadsChanged();
        if (!threadToSelect.isEmpty()) {
            for (int i = 0; i < m_threads.size(); ++i) {
                if (m_threads.at(i).toMap().value(QStringLiteral("id")).toString() != threadToSelect)
                    continue;
                if (m_activeTurns.contains(threadToSelect)) {
                    m_selectedThread = i;
                    m_conversation.setThread(threadToSelect);
                    emit selectedThreadChanged();
                    emit turnRunningChanged();
                    emit turnElapsedChanged();
                    emit currentTasksChanged();
                    emit currentPlanChanged();
                } else {
                    selectThread(i);
                }
                if (!m_pendingPrompt.isEmpty() || !m_pendingImages.isEmpty()) {
                    const QString prompt = std::exchange(m_pendingPrompt, {});
                    const QString modelId = std::exchange(m_pendingModelId, {});
                    const QString reasoningEffort =
                        std::exchange(m_pendingReasoningEffort, {});
                    const QString collaborationMode =
                        std::exchange(m_pendingCollaborationMode, QStringLiteral("default"));
                    const auto images = std::exchange(m_pendingImages, {});
                    startPromptTurn(selectedThreadId(), prompt, images, modelId,
                                    reasoningEffort, collaborationMode,
                                    m_pendingPermissionProfile, true);
                }
                break;
            }
        }
    });
}

void AppController::loadModels()
{
    m_provider->listModels([this](const QJsonObject &result, const QString &error) {
        if (!error.isEmpty()) {
            setStatus(error);
            return;
        }
        QVariantList models;
        for (const auto &entry : result.value(QStringLiteral("data")).toArray()) {
            const auto model = entry.toObject();
            if (model.value(QStringLiteral("hidden")).toBool())
                continue;
            QVariantList efforts;
            for (const auto &effort : model.value(QStringLiteral("supportedReasoningEfforts")).toArray())
                efforts.push_back(effort.toObject().value(QStringLiteral("reasoningEffort")).toString());
            models.push_back(QVariantMap{
                {QStringLiteral("id"), model.value(QStringLiteral("model")).toString()},
                {QStringLiteral("name"), model.value(QStringLiteral("displayName")).toString()},
                {QStringLiteral("description"), model.value(QStringLiteral("description")).toString()},
                {QStringLiteral("isDefault"), model.value(QStringLiteral("isDefault")).toBool()},
                {QStringLiteral("defaultEffort"), model.value(QStringLiteral("defaultReasoningEffort")).toString()},
                {QStringLiteral("efforts"), efforts}});
        }
        m_models = models;
        emit modelsChanged();
    });
}

void AppController::selectThread(int index)
{
    if (index < 0 || index >= m_threads.size())
        return;
    m_selectedThread = index;
    m_conversation.setThread(selectedThreadId());
    emit selectedThreadChanged();
    emit turnRunningChanged();
    emit turnElapsedChanged();
    emit tokenUsageChanged();
    emit currentTasksChanged();
    emit currentPlanChanged();
    const auto threadId = selectedThreadId();
    m_provider->resumeThread(threadId, [this, threadId](const QJsonObject &result,
                                                        const QString &error) {
        if (!error.isEmpty()) {
            setStatus(error);
            return;
        }
        if (threadId != selectedThreadId())
            return;
        QString databaseError;
        const auto persistedEvents =
            m_database.conversationEvents(threadId, &databaseError);
        if (!databaseError.isEmpty()) {
            setStatus(QStringLiteral("Could not load conversation history: %1")
                          .arg(databaseError));
            return;
        }
        if (!persistedEvents.isEmpty()) {
            for (const auto &stored : persistedEvents) {
                const auto type = stored.value(QStringLiteral("type")).toString();
                const auto content = stored.value(QStringLiteral("content")).toString();
                const auto metadata = stored.value(QStringLiteral("metadata")).toMap();
                if (type != QStringLiteral("plan") && type != QStringLiteral("task")) {
                    m_conversation.append(
                        {threadId,
                         type,
                         stored.value(QStringLiteral("title")).toString(),
                         content,
                         metadata});
                }
                if (type == QStringLiteral("plan")) {
                    m_threadPlans.insert(threadId, metadata.value(QStringLiteral("plan")).toList());
                    m_threadPlanExplanations.insert(
                        threadId, metadata.value(QStringLiteral("explanation")).toString());
                } else if (type == QStringLiteral("task")) {
                    m_threadTasks.insert(threadId, content);
                } else if (type == QStringLiteral("diff")) {
                    m_diff = content;
                }
            }
            emit currentTasksChanged();
            emit currentPlanChanged();
            emit diffChanged();
            return;
        }
        const auto thread = result.value(QStringLiteral("thread")).toObject();
        for (const auto &turnValue : thread.value(QStringLiteral("turns")).toArray()) {
            const auto turn = turnValue.toObject();
            for (const auto &itemValue : turn.value(QStringLiteral("items")).toArray()) {
                const auto item = itemValue.toObject();
                const auto type = item.value(QStringLiteral("type")).toString();
                const auto content = m_provider->itemContent(item);
                if (content.isEmpty())
                    continue;
                QString eventType;
                QString title;
                if (type == QStringLiteral("userMessage")) {
                    eventType = QStringLiteral("user");
                    title = QStringLiteral("You");
                } else if (type == QStringLiteral("agentMessage")) {
                    eventType = QStringLiteral("assistant");
                    title = QStringLiteral("Artemis");
                } else if (type == QStringLiteral("reasoning")) {
                    eventType = QStringLiteral("reasoning");
                    title = QStringLiteral("Reasoning");
                } else if (type == QStringLiteral("commandExecution")) {
                    eventType = QStringLiteral("command");
                    title = QStringLiteral("Ran command");
                } else if (type == QStringLiteral("fileChange")) {
                    eventType = QStringLiteral("file");
                    title = QStringLiteral("File changes");
                } else if (type == QStringLiteral("plan")) {
                    eventType = QStringLiteral("task");
                    title = QStringLiteral("Tasks");
                } else {
                    continue;
                }
                ConversationEvent event{threadId, eventType, title, content,
                    {{QStringLiteral("lifecycle"), QStringLiteral("completed")},
                     {QStringLiteral("itemId"), item.value(QStringLiteral("id")).toString()},
                     {QStringLiteral("turnId"), turn.value(QStringLiteral("id")).toString()}}};
                if (eventType != QStringLiteral("task"))
                    m_conversation.append(event);
                persistConversationEvent(event);
                if (eventType == QStringLiteral("task"))
                    m_threadTasks.insert(threadId, content);
            }
        }
        emit currentTasksChanged();
    });
}

PermissionProfile AppController::permissionProfile(const QString &mode) const
{
    if (mode == QStringLiteral("approval-required"))
        return PermissionProfile::ReadOnly;
    if (mode == QStringLiteral("auto-accept-edits"))
        return PermissionProfile::WorkspaceWrite;
    return PermissionProfile::FullAccess;
}

void AppController::createThread(const QString &modelId, const QString &reasoningEffort,
                                 const QString &permissionMode)
{
    const auto project = m_projects.row(m_selectedProject);
    if (project.id < 0 || !m_provider->ready())
        return;
    if (m_threadCreationPending)
        return;
    m_threadCreationPending = true;
    beginThread(modelId, reasoningEffort, permissionProfile(permissionMode));
}

void AppController::beginThread(const QString &modelId, const QString &reasoningEffort,
                                PermissionProfile permissionProfile)
{
    const auto project = m_projects.row(m_selectedProject);
    ThreadConfiguration config{project.path, project.path, modelId, reasoningEffort,
                               permissionProfile, false};
    m_provider->startThread(
        config, [this, project, permissionProfile](const QJsonObject &result,
                                                   const QString &error) {
        m_threadCreationPending = false;
        if (!error.isEmpty()) {
            if (!m_pendingPrompt.isEmpty() || !m_pendingImages.isEmpty())
                emit promptRestoreRequested(m_pendingPrompt,
                                            QVariantList(m_pendingImages.cbegin(),
                                                         m_pendingImages.cend()));
            m_pendingPrompt.clear();
            m_pendingImages.clear();
            m_pendingModelId.clear();
            setStatus(error);
            return;
        }
        const auto thread = result.value(QStringLiteral("thread")).toObject();
        const auto id = thread.value(QStringLiteral("id")).toString();
        QString bindError;
        if (!m_database.bindThread(project.id, id, project.path, false, &bindError)) {
            setStatus(QStringLiteral("Could not save thread binding: %1").arg(bindError));
        }
        loadProjects();
        if (m_projects.row(m_selectedProject).id != project.id) {
            if (!m_pendingPrompt.isEmpty() || !m_pendingImages.isEmpty()) {
                emit promptRestoreRequested(
                    m_pendingPrompt,
                    QVariantList(m_pendingImages.cbegin(), m_pendingImages.cend()));
                m_pendingPrompt.clear();
                m_pendingImages.clear();
                m_pendingModelId.clear();
                m_pendingReasoningEffort.clear();
                m_pendingCollaborationMode = QStringLiteral("default");
            }
            return;
        }
        m_threads.prepend(QVariantMap{
            {QStringLiteral("id"), id},
            {QStringLiteral("title"), QStringLiteral("Untitled thread")},
            {QStringLiteral("named"), false},
            {QStringLiteral("cwd"), project.path},
            {QStringLiteral("model"), m_pendingModelId.isEmpty()
                ? m_codingModelId : m_pendingModelId},
            {QStringLiteral("external"), false}});
        m_selectedThread = 0;
        m_conversation.setThread(id);
        emit threadsChanged();
        emit selectedThreadChanged();
        emit turnRunningChanged();
        emit turnElapsedChanged();
        emit tokenUsageChanged();
        emit currentTasksChanged();
        emit currentPlanChanged();
        if (!m_pendingPrompt.isEmpty() || !m_pendingImages.isEmpty()) {
            const QString prompt = std::exchange(m_pendingPrompt, {});
            const QString modelId = std::exchange(m_pendingModelId, {});
            const QString reasoningEffort = std::exchange(m_pendingReasoningEffort, {});
            const QString collaborationMode =
                std::exchange(m_pendingCollaborationMode, QStringLiteral("default"));
            const auto images = std::exchange(m_pendingImages, {});
            startPromptTurn(id, prompt, images, modelId, reasoningEffort,
                            collaborationMode, permissionProfile, true);
        } else {
            setStatus(QStringLiteral("Thread created"));
        }
    });
}

bool AppController::sendPrompt(const QString &text, const QVariantList &imageValues,
                               const QString &modelId, const QString &reasoningEffort,
                               const QString &permissionMode,
                               const QString &collaborationMode)
{
    const QString prompt = text.trimmed();
    QStringList images;
    for (const auto &value : imageValues) {
        const auto path = value.toString();
        const QFileInfo info(path);
        if (info.exists() && info.isFile())
            images.push_back(info.absoluteFilePath());
    }
    images.removeDuplicates();
    if (prompt.isEmpty() && images.isEmpty())
        return false;
    if (selectedThreadId().isEmpty()) {
        if (selectedProjectPath().isEmpty() || !m_provider->ready()
            || m_threadCreationPending)
            return false;
        m_pendingPrompt = prompt;
        m_pendingImages = images;
        m_pendingModelId = modelId;
        m_pendingReasoningEffort = reasoningEffort;
        m_pendingCollaborationMode = collaborationMode;
        m_pendingPermissionProfile = permissionProfile(permissionMode);
        createThread(modelId, reasoningEffort, permissionMode);
        return true;
    }
    if (m_activeTurns.contains(selectedThreadId())) {
        const ConversationEvent userEvent{
            selectedThreadId(), QStringLiteral("user"), QStringLiteral("You"), prompt,
            {{QStringLiteral("images"), images}}};
        m_conversation.append(userEvent);
        persistConversationEvent(userEvent);
        m_pendingSteers[selectedThreadId()].push_back({prompt, images});
        sendPendingSteers(selectedThreadId());
        return true;
    }
    const bool generateTitle = m_selectedThread >= 0
        && !m_threads.at(m_selectedThread).toMap().value(QStringLiteral("named")).toBool();
    startPromptTurn(selectedThreadId(), prompt, images, modelId, reasoningEffort,
                    collaborationMode,
                    permissionProfile(permissionMode), generateTitle);
    return true;
}

void AppController::sendPendingSteers(const QString &threadId)
{
    const auto activeTurn = m_activeTurns.constFind(threadId);
    auto pending = m_pendingSteers.find(threadId);
    if (activeTurn == m_activeTurns.cend() || activeTurn->turnId.isEmpty()
        || pending == m_pendingSteers.end() || pending->isEmpty()) {
        return;
    }
    const auto steer = pending->takeFirst();
    if (pending->isEmpty())
        m_pendingSteers.erase(pending);
    m_provider->steerTurn(threadId, activeTurn->turnId, steer.prompt, steer.images,
                      [this, threadId, steer](const QJsonObject &, const QString &error) {
            if (!error.isEmpty())
                setStatus(error);
            if (!error.isEmpty()) {
                emit promptRestoreRequested(
                    steer.prompt,
                    QVariantList(steer.images.cbegin(), steer.images.cend()));
            }
            sendPendingSteers(threadId);
        });
}

void AppController::restorePendingSteers(const QString &threadId)
{
    auto steers = m_pendingSteers.take(threadId);
    while (!steers.isEmpty()) {
        const auto steer = steers.takeFirst();
        emit promptRestoreRequested(
            steer.prompt, QVariantList(steer.images.cbegin(), steer.images.cend()));
    }
}

void AppController::setActiveTurnId(const QString &threadId, const QString &turnId)
{
    if (m_titleTargets.contains(threadId) || threadId == m_commitThreadId)
        return;
    if (!m_activeTurns.contains(threadId))
        setTurnRunning(threadId, true);
    auto activeTurn = m_activeTurns.find(threadId);
    if (turnId.isEmpty())
        return;
    activeTurn->turnId = turnId;
    sendPendingSteers(threadId);
}

void AppController::handleUserInputRequest(const QString &threadId, const QString &turnId,
                                           const QString &itemId,
                                           const QVariantList &questions)
{
    if (itemId.isEmpty() || questions.isEmpty())
        return;
    if (hasPendingUserInput()) {
        setStatus(QStringLiteral("Codex requested new input before the previous question was answered."));
        return;
    }
    m_pendingUserInputThreadId = threadId;
    m_pendingUserInputTurnId = turnId;
    m_pendingUserInputItemId = itemId;
    m_pendingUserInputQuestions = questions;
    m_pendingUserInputAnswers.clear();
    m_pendingUserInputIndex = 0;
    emit pendingUserInputChanged();
}

bool AppController::answerPendingUserInput(const QString &answer)
{
    const auto trimmedAnswer = answer.trimmed();
    if (!hasPendingUserInput() || trimmedAnswer.isEmpty()
        || selectedThreadId() != m_pendingUserInputThreadId) {
        return false;
    }

    const auto question = m_pendingUserInputQuestions.at(m_pendingUserInputIndex).toMap();
    const auto questionId = question.value(QStringLiteral("id")).toString();
    if (questionId.isEmpty())
        return false;
    m_pendingUserInputAnswers.insert(
        questionId, QVariantMap{{QStringLiteral("answers"),
                                 QStringList{trimmedAnswer}}});

    ++m_pendingUserInputIndex;
    if (hasPendingUserInput()) {
        emit pendingUserInputChanged();
        return true;
    }

    const auto itemId = m_pendingUserInputItemId;
    const auto answers = m_pendingUserInputAnswers;
    m_provider->respondToUserInput(itemId, answers,
        [this](const QJsonObject &, const QString &error) {
            if (!error.isEmpty()) {
                setStatus(error);
            }
            clearPendingUserInput();
        });
    return true;
}

void AppController::clearPendingUserInput()
{
    const bool hadPendingInput = !m_pendingUserInputItemId.isEmpty()
        || !m_pendingUserInputQuestions.isEmpty();
    m_pendingUserInputThreadId.clear();
    m_pendingUserInputTurnId.clear();
    m_pendingUserInputItemId.clear();
    m_pendingUserInputQuestions.clear();
    m_pendingUserInputAnswers.clear();
    m_pendingUserInputIndex = -1;
    if (hadPendingInput)
        emit pendingUserInputChanged();
}

void AppController::copyText(const QString &text)
{
    QApplication::clipboard()->setText(text);
}

void AppController::startPromptTurn(const QString &threadId, const QString &prompt,
                                    const QStringList &images, const QString &modelId,
                                    const QString &reasoningEffort,
                                    const QString &collaborationMode,
                                    PermissionProfile permissionProfile, bool generateTitle)
{
    const auto titleProjectPath = selectedProjectPath();
    const auto titleWorkspacePath = selectedWorkspacePath();
    const auto titleModelId = m_titleModelId;
    m_conversation.setThread(threadId);
    const ConversationEvent userEvent{
        threadId, QStringLiteral("user"), QStringLiteral("You"), prompt,
        {{QStringLiteral("images"), images}}};
    m_conversation.append(userEvent);
    persistConversationEvent(userEvent);
    setTurnRunning(threadId, true);
    m_provider->sendTurn(threadId, prompt, images, modelId, reasoningEffort,
                     collaborationMode, permissionProfile,
                     [this, threadId, prompt, images, generateTitle,
                      titleProjectPath, titleWorkspacePath, titleModelId](
                         const QJsonObject &result, const QString &error) {
        if (!error.isEmpty()) {
            restorePendingSteers(threadId);
            setTurnRunning(threadId, false);
            setStatus(error);
            emit promptRestoreRequested(prompt,
                                        QVariantList(images.cbegin(), images.cend()));
            return;
        }
        setActiveTurnId(threadId, result.value(QStringLiteral("turn")).toObject()
                                     .value(QStringLiteral("id")).toString());
        if (generateTitle) {
            generateThreadTitle(
                threadId,
                prompt.isEmpty() ? QStringLiteral("Image attachment") : prompt,
                titleProjectPath, titleWorkspacePath, titleModelId);
        }
        loadThreads(threadId);
    });
}

QString AppController::pasteClipboardImage()
{
    const auto *clipboard = QApplication::clipboard();
    const auto *mime = clipboard->mimeData();
    QImage image = clipboard->image();
    if (image.isNull() && mime->hasImage()) {
        const auto imageData = mime->imageData();
        if (imageData.canConvert<QImage>())
            image = imageData.value<QImage>();
    }
    if (image.isNull() && mime->hasUrls()) {
        for (const auto &url : mime->urls()) {
            if (!url.isLocalFile())
                continue;
            QImageReader reader(url.toLocalFile());
            if (reader.canRead()) {
                image = reader.read();
                if (!image.isNull())
                    break;
            }
        }
    }
    if (image.isNull())
        return {};
    if (image.width() > 12000 || image.height() > 12000
        || qint64(image.width()) * image.height() > 40000000) {
        setStatus(QStringLiteral("Clipboard image is too large"));
        return {};
    }
    auto path = QDir(Paths::attachmentRoot()).filePath(
        QUuid::createUuid().toString(QUuid::WithoutBraces) + QStringLiteral(".png"));
    if (!image.save(path, "PNG")) {
        setStatus(QStringLiteral("Could not save the clipboard image"));
        return {};
    }
    setStatus(QStringLiteral("Image attached"));
    return path;
}

void AppController::interruptTurn()
{
    const auto threadId = selectedThreadId();
    const auto activeTurn = m_activeTurns.constFind(threadId);
    if (activeTurn == m_activeTurns.cend() || activeTurn->turnId.isEmpty())
        return;
    m_provider->interruptTurn(threadId, activeTurn->turnId,
                          [this](const QJsonObject &, const QString &error) {
        if (!error.isEmpty())
            setStatus(error);
    });
}

void AppController::handleDomainEvent(const QString &threadId, const QString &type,
                                      const QString &title, const QString &content,
                                      const QVariantMap &metadata)
{
    if (m_titleTargets.contains(threadId)) {
        if (type == QStringLiteral("assistant")) {
            if (metadata.value(QStringLiteral("lifecycle")).toString()
                == QStringLiteral("completed")) {
                m_titleDraftBuffers[threadId] = content;
            } else {
                m_titleDraftBuffers[threadId] += content;
            }
        }
        if (type == QStringLiteral("status")) {
            const auto targetThreadId = m_titleTargets.take(threadId);
            const auto title = cleanTitleDraft(m_titleDraftBuffers.take(threadId));
            if (!title.isEmpty())
                applyThreadTitle(targetThreadId, title);
        }
        return;
    }
    if (threadId == m_commitThreadId) {
        if (type == QStringLiteral("assistant")) {
            if (metadata.value(QStringLiteral("lifecycle")).toString()
                == QStringLiteral("completed")) {
                m_commitDraftBuffer = content;
            } else {
                m_commitDraftBuffer += content;
            }
        }
        if (type == QStringLiteral("status")) {
            const auto draft = cleanCommitDraft(m_commitDraftBuffer);
            if (draft.isEmpty()) {
                emit commitDraftFinished(
                    false, QStringLiteral("Codex returned an empty commit message."));
            } else {
                emit commitDraftFinished(true, draft);
            }
            m_commitThreadId.clear();
            m_commitDraftBuffer.clear();
        }
        return;
    }
    ConversationEvent event{threadId, type, title, content, metadata};
    if (type == QStringLiteral("status") && content == QStringLiteral("completed")) {
        const auto activeTurn = m_activeTurns.constFind(threadId);
        const auto elapsed = activeTurn == m_activeTurns.cend()
            ? 0 : QDateTime::currentMSecsSinceEpoch() - activeTurn->startedAtMs;
        event.content = QStringLiteral("Complete · %1 · %2 total")
                            .arg(QLocale().toString(QTime::currentTime(), QLocale::ShortFormat),
                                 elapsedText(elapsed));
    }
    const bool isAssistantDelta = type == QStringLiteral("assistant")
        && metadata.value(QStringLiteral("delta")).toBool();
    QString persistedContent = event.content;
    const auto itemId = metadata.value(QStringLiteral("itemId")).toString();
    const auto draftKey = threadId + QLatin1Char(':') + itemId;
    if (isAssistantDelta && !itemId.isEmpty()) {
        persistedContent = m_assistantDraftBuffers[draftKey] += event.content;
    } else if (type == QStringLiteral("assistant") && !itemId.isEmpty()) {
        m_assistantDraftBuffers.remove(draftKey);
    }
    if (!isAssistantDelta || !itemId.isEmpty()) {
        persistConversationEvent(event, persistedContent);
    }
    if (type == QStringLiteral("plan")) {
        m_threadPlans.insert(threadId, metadata.value(QStringLiteral("plan")).toList());
        m_threadPlanExplanations.insert(
            threadId, metadata.value(QStringLiteral("explanation")).toString());
        if (threadId == selectedThreadId()) {
            emit currentPlanChanged();
            emit taskPanelRequested();
        }
    } else if (type == QStringLiteral("task")) {
        m_threadTasks.insert(threadId, content);
        if (threadId == selectedThreadId()) {
            emit currentTasksChanged();
            emit taskPanelRequested();
        }
    } else if (type == QStringLiteral("assistant"))
        m_conversation.appendOrMergeDelta(event);
    else
        m_conversation.append(event);
    if (type == QStringLiteral("diff")) {
        if (threadId == selectedThreadId()) {
            m_diff = content;
            emit diffChanged();
        }
    }
    if (type == QStringLiteral("status")) {
        restorePendingSteers(threadId);
        if (m_pendingUserInputThreadId == threadId)
            clearPendingUserInput();
        setTurnRunning(threadId, false);
        if (threadId == selectedThreadId())
            refreshGit();
    }
}

void AppController::refreshGit()
{
    const auto path = selectedWorkspacePath();
    m_hasGitChanges = false;
    if (path.isEmpty() || !selectedProjectIsGit()) {
        m_diff.clear();
        m_gitStatus = QStringLiteral("This folder is not a Git repository.");
        emit diffChanged();
        return;
    }
    emit diffChanged();
    m_git.status(path, [this, path](const GitResult &result) {
        if (path != selectedWorkspacePath())
            return;
        m_gitStatus = result.ok() ? QString::fromUtf8(result.output).replace(QChar(u'\0'), QChar(u'\n'))
                                  : QString::fromUtf8(result.error);
        m_hasGitChanges = result.ok() && GitService::statusHasChanges(result.output);
        emit diffChanged();
    });
    m_git.diff(path, [this, path](const GitResult &result) {
        if (path != selectedWorkspacePath())
            return;
        m_diff = result.ok() ? QString::fromUtf8(result.output) : QString::fromUtf8(result.error);
        emit diffChanged();
    });
}

void AppController::generateCommitMessage()
{
    if (!selectedProjectIsGit()) {
        emit commitDraftFinished(false, QStringLiteral("Select a Git repository first."));
        return;
    }
    if (!m_provider->ready()) {
        emit commitDraftFinished(false, QStringLiteral("Codex is offline."));
        return;
    }
    const auto projectId = m_projects.row(m_selectedProject).id;
    const auto projectPath = selectedProjectPath();
    const auto projectName = selectedProjectName();
    const auto workspace = selectedWorkspacePath();
    const auto modelId = m_commitModelId;
    QString branch;
    const auto firstStatusLine = m_gitStatus.section(QChar(u'\n'), 0, 0);
    if (firstStatusLine.startsWith(QStringLiteral("## ")))
        branch = firstStatusLine.mid(3).section(QStringLiteral("..."), 0, 0).trimmed();
    setStatus(QStringLiteral("Preparing commit snapshot…"));
    m_git.generateCommitSnapshot(
        workspace,
        [this, projectId, projectPath, projectName, workspace, modelId, branch](
            const GitResult &snapshot) {
        if (!snapshot.ok()) {
            const auto message = QString::fromUtf8(snapshot.error).trimmed();
            setStatus(message);
            emit commitDraftFinished(false, message);
            return;
        }
        ThreadConfiguration config{projectPath, workspace, modelId, {},
                                   PermissionProfile::ReadOnly, true};
        m_provider->startThread(
            config,
            [this, projectId, projectName, modelId, branch, snapshot](
                const QJsonObject &result, const QString &error) {
            if (!error.isEmpty()) {
                setStatus(error);
                emit commitDraftFinished(false, error);
                return;
            }
            m_commitThreadId = result.value(QStringLiteral("thread")).toObject()
                                   .value(QStringLiteral("id")).toString();
            m_commitDraftBuffer.clear();
            m_provider->sendTurn(
                m_commitThreadId,
                commitPrompt(snapshot.output, projectName, branch), {},
                modelId, {}, QStringLiteral("default"), PermissionProfile::ReadOnly,
                [this](const QJsonObject &, const QString &turnError) {
                if (!turnError.isEmpty()) {
                    setStatus(turnError);
                    m_commitThreadId.clear();
                    emit commitDraftFinished(false, turnError);
                }
            });
            if (m_projects.row(m_selectedProject).id == projectId)
                setStatus(QStringLiteral("Generating commit message…"));
        });
    });
}

void AppController::generateThreadTitle(const QString &threadId, const QString &prompt,
                                        const QString &projectPath,
                                        const QString &workspacePath,
                                        const QString &modelId)
{
    ThreadConfiguration config{projectPath, workspacePath, modelId, {},
                               PermissionProfile::ReadOnly, true};
    m_provider->startThread(config, [this, threadId, prompt, modelId](
                                        const QJsonObject &result, const QString &error) {
        if (!error.isEmpty()) {
            setStatus(QStringLiteral("Could not generate thread title: %1").arg(error));
            return;
        }
        const auto titleThreadId = result.value(QStringLiteral("thread")).toObject()
                                       .value(QStringLiteral("id")).toString();
        if (titleThreadId.isEmpty())
            return;
        m_titleTargets.insert(titleThreadId, threadId);
        m_titleDraftBuffers.insert(titleThreadId, {});
        const auto titlePrompt = QStringLiteral(
            "Create a concise title for a software-development conversation from the user's "
            "first message below. Return JSON only as {\"title\":\"...\"}. Use 3 to 7 words, "
            "sentence case, no punctuation at the end, and describe the concrete task.\n\n%1")
            .arg(prompt.left(12000));
        m_provider->sendTurn(titleThreadId, titlePrompt, {}, modelId, {},
            QStringLiteral("default"), PermissionProfile::ReadOnly,
            [this, titleThreadId](const QJsonObject &, const QString &turnError) {
                if (turnError.isEmpty())
                    return;
                m_titleTargets.remove(titleThreadId);
                m_titleDraftBuffers.remove(titleThreadId);
                setStatus(QStringLiteral("Could not generate thread title: %1").arg(turnError));
            });
    });
}

void AppController::applyThreadTitle(const QString &threadId, const QString &title)
{
    m_provider->setThreadName(
        threadId, title,
        [this, threadId, title](const QJsonObject &, const QString &error) {
        if (!error.isEmpty()) {
            setStatus(QStringLiteral("Could not name thread: %1").arg(error));
            return;
        }
        for (int i = 0; i < m_threads.size(); ++i) {
            auto row = m_threads.at(i).toMap();
            if (row.value(QStringLiteral("id")).toString() != threadId)
                continue;
            row.insert(QStringLiteral("title"), title);
            row.insert(QStringLiteral("named"), true);
            m_threads[i] = row;
            emit threadsChanged();
            if (i == m_selectedThread)
                emit selectedThreadChanged();
            break;
        }
        loadThreads(threadId);
    });
}

QString AppController::commitPrompt(const QByteArray &snapshot, const QString &projectName,
                                    const QString &branch) const
{
    return QStringLiteral(
        "You write concise Git commit messages.\n"
        "Return JSON only with exactly these string keys: "
        "{\"subject\":\"...\",\"body\":\"...\"}.\n"
        "Rules:\n"
        "- subject must use imperative mood, be at most 72 characters, and have no trailing period\n"
        "- body must be an empty string or 1-4 concise Markdown bullet points\n"
        "- capture the primary user-visible or developer-visible change\n"
        "- explain what changed and why; do not produce a file-by-file inventory\n"
        "- omit details already clear from the subject\n"
        "- mention important behavioral, architectural, or compatibility effects\n"
        "- do not claim tests were run unless the snapshot proves it\n"
        "- never invent issue numbers or unsupported context\n"
        "\nRepository: %1\n"
        "Branch: %2\n"
        "\nStaged snapshot:\n%3")
        .arg(projectName, branch,
             QString::fromUtf8(snapshot).left(120000));
}

QString AppController::cleanCommitDraft(const QString &raw) const
{
    QString text = raw.trimmed();
    if (text.startsWith(QStringLiteral("```"))) {
        text.remove(QRegularExpression(QStringLiteral("^```(?:json)?\\s*|\\s*```$")));
    }
    auto object = commitDraftObject(text);
    if (object.isEmpty()) {
        const auto document = QJsonDocument::fromJson(text.toUtf8());
        if (document.isObject())
            object = document.object();
    }
    if (!object.isEmpty()) {
        auto subject = object.value(QStringLiteral("subject")).toString().trimmed()
                           .section(QChar(u'\n'), 0, 0).trimmed();
        subject.remove(QRegularExpression(QStringLiteral("\\.+$")));
        subject = subject.left(72).trimmed();
        const auto body = object.value(QStringLiteral("body")).toString().trimmed();
        if (!subject.isEmpty())
            return body.isEmpty() ? subject : subject + QStringLiteral("\n\n") + body;
    }
    return text;
}

QString AppController::cleanTitleDraft(const QString &raw) const
{
    QString text = raw.trimmed();
    if (text.startsWith(QStringLiteral("```")))
        text.remove(QRegularExpression(QStringLiteral("^```(?:json)?\\s*|\\s*```$")));
    const auto document = QJsonDocument::fromJson(text.toUtf8());
    if (document.isObject())
        text = document.object().value(QStringLiteral("title")).toString().trimmed();
    text = text.section(QChar(u'\n'), 0, 0).trimmed();
    if (text.startsWith(QChar(u'"')) && text.endsWith(QChar(u'"')) && text.size() > 1)
        text = text.mid(1, text.size() - 2).trimmed();
    return text.left(80);
}

void AppController::setModelSetting(const QString &key, QString &storage,
                                    const QString &modelId)
{
    if (storage == modelId)
        return;
    QString error;
    if (!m_database.setSetting(key, modelId, &error)) {
        setStatus(QStringLiteral("Could not save settings: %1").arg(error));
        return;
    }
    storage = modelId;
    emit settingsChanged();
}

void AppController::setCodingModelId(const QString &modelId)
{
    setModelSetting(QStringLiteral("coding_model"), m_codingModelId, modelId);
}

void AppController::setCodingReasoningEffort(const QString &reasoningEffort)
{
    setModelSetting(QStringLiteral("coding_reasoning_effort"),
                    m_codingReasoningEffort, reasoningEffort);
}

void AppController::setCommitModelId(const QString &modelId)
{
    setModelSetting(QStringLiteral("commit_model"), m_commitModelId, modelId);
}

void AppController::setTitleModelId(const QString &modelId)
{
    setModelSetting(QStringLiteral("title_model"), m_titleModelId, modelId);
}

void AppController::setSelectedEditorId(const QString &desktopId)
{
    setModelSetting(QStringLiteral("editor_desktop_id"), m_selectedEditorId, desktopId);
}

void AppController::setSelectedTerminalId(const QString &desktopId)
{
    setModelSetting(QStringLiteral("terminal_desktop_id"), m_selectedTerminalId, desktopId);
}

void AppController::commitAllAndPush(const QString &subject, const QString &body)
{
    m_git.commitAllAndPush(selectedWorkspacePath(), subject, body, [this](const GitResult &result) {
        const auto output = QString::fromUtf8(result.ok()
            ? result.output + result.error : result.error).trimmed();
        if (result.failure == GitFailure::IndexLocked) {
            const auto message = QStringLiteral(
                "Git's index is locked. Another Git operation may still be running. "
                "Close other Git clients, then retry. Remove the lock only if you are sure "
                "it is stale.");
            emit commitLockBlocked(message);
            setStatus(message);
            return;
        }
        emit commitFinished(result.ok(), output);
        setStatus(output);
        refreshGit();
    });
}

void AppController::commitFeatureBranch(const QString &subject, const QString &body,
                                        const QString &branch, const QString &remote)
{
    if (branch.trimmed().isEmpty()) {
        emit commitFinished(false, QStringLiteral("Branch name is empty"));
        return;
    }
    m_git.createBranchCommitPush(selectedWorkspacePath(), branch, subject, body,
                                 remote.isEmpty() ? QStringLiteral("origin") : remote,
                                 [this](const GitResult &result) {
        const auto output = QString::fromUtf8(result.ok()
            ? result.output + result.error : result.error).trimmed();
        if (result.failure == GitFailure::IndexLocked) {
            const auto message = QStringLiteral(
                "Git's index is locked. Another Git operation may still be running. "
                "Close other Git clients, then retry. Remove the lock only if you are sure "
                "it is stale.");
            emit commitLockBlocked(message);
            setStatus(message);
            return;
        }
        emit commitFinished(result.ok(), output);
        setStatus(output);
        refreshGit();
    });
}

void AppController::retryLockedCommit()
{
    setStatus(QStringLiteral("Retrying Git operation…"));
    m_git.retryLockedOperation();
}

void AppController::removeCommitLockAndRetry()
{
    m_git.removeIndexLockAndRetry([this](const GitResult &result) {
        const auto message = QString::fromUtf8(
            result.ok() ? result.output : result.error).trimmed();
        setStatus(message);
        if (!result.ok())
            emit commitFinished(false, message);
    });
}

void AppController::cancelLockedCommit()
{
    m_git.cancelLockedOperation();
    emit commitFinished(false, QStringLiteral("Commit operation canceled."));
}

QString AppController::suggestBranch(const QString &message) const
{
    return GitService::suggestedBranch(message.section(QChar(u'\n'), 0, 0));
}

QString AppController::selectedWorkspacePath() const
{
    if (m_selectedThread >= 0 && m_selectedThread < m_threads.size()) {
        const auto cwd = m_threads.at(m_selectedThread).toMap()
                             .value(QStringLiteral("cwd")).toString();
        if (!cwd.isEmpty())
            return cwd;
    }
    return selectedProjectPath();
}

void AppController::openProjectFolder()
{
    if (!DesktopIntegration::openFolder(selectedProjectPath()))
        setStatus(QStringLiteral("Could not open the project folder."));
}

void AppController::openProjectEditor()
{
    QString error;
    if (!DesktopIntegration::openEditor(
            m_selectedEditorId, selectedProjectPath(), &error) && !error.isEmpty()) {
        setStatus(error);
    }
}

void AppController::openTerminal()
{
    QString error;
    if (!DesktopIntegration::openTerminal(
            m_selectedTerminalId, selectedWorkspacePath(), &error)
        && !error.isEmpty()) {
        setStatus(error);
    }
}

void AppController::setStatus(const QString &text)
{
    QTextStream(stdout) << text << Qt::endl;
    emit statusMessage(text);
    if (m_status == text)
        return;
    m_status = text;
    emit statusTextChanged();
}

void AppController::persistConversationEvent(const ConversationEvent &event,
                                             const QString &contentOverride)
{
    QString error;
    const auto content = contentOverride.isNull() ? event.content : contentOverride;
    if (!m_database.saveConversationEvent(
            event.threadId, event.type, event.title, content, event.metadata, &error)) {
        setStatus(QStringLiteral("Could not save conversation history: %1").arg(error));
    }
}

void AppController::setTurnRunning(const QString &threadId, bool running)
{
    if (threadId.isEmpty() || m_activeTurns.contains(threadId) == running)
        return;
    if (running) {
        m_activeTurns.insert(
            threadId, ActiveTurn{{}, QDateTime::currentMSecsSinceEpoch()});
    } else {
        m_activeTurns.remove(threadId);
    }
    if (m_activeTurns.isEmpty())
        m_turnElapsedUpdateTimer.stop();
    else if (!m_turnElapsedUpdateTimer.isActive())
        m_turnElapsedUpdateTimer.start();
    if (threadId == selectedThreadId()) {
        emit turnRunningChanged();
        emit turnElapsedChanged();
    }
}

} // namespace Artemis
