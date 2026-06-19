#include "app/AppController.h"
#include "platform/Paths.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QProcess>
#include <QTime>
#include <QUrl>

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
        } else if (character == QChar(u'}') && depth > 0 && --depth == 0 && start >= 0) {
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

} // namespace

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    m_turnElapsedUpdateTimer.setInterval(1000);
    connect(&m_turnElapsedUpdateTimer, &QTimer::timeout,
            this, &AppController::turnElapsedChanged);
    connect(&m_codex, &CodexClient::readyChanged, this, [this](bool ready) {
        emit providerReadyChanged();
        if (ready) {
            setStatus(QStringLiteral("Codex connected"));
            loadModels();
            loadThreads();
        }
    });
    connect(&m_codex, &CodexClient::versionChanged, this, &AppController::providerReadyChanged);
    connect(&m_codex, &CodexClient::providerError, this, [this](const QString &message) {
        setStatus(message);
    });
    connect(&m_codex, &CodexClient::domainEvent, this, &AppController::handleDomainEvent);
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
    m_codingModelId = m_database.setting(QStringLiteral("coding_model"));
    m_commitModelId = m_database.setting(QStringLiteral("commit_model"));
    m_titleModelId = m_database.setting(QStringLiteral("title_model"));
    loadProjects();
    m_codex.start();
    return true;
}

ProjectTreeModel *AppController::projects() { return &m_projects; }
ConversationModel *AppController::conversation() { return &m_conversation; }
QVariantList AppController::threads() const { return m_threads; }
QVariantList AppController::models() const { return m_models; }
QString AppController::codingModelId() const { return m_codingModelId; }
QString AppController::commitModelId() const { return m_commitModelId; }
QString AppController::titleModelId() const { return m_titleModelId; }
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
QVariantList AppController::currentPlan() const
{
    return m_threadPlans.value(selectedThreadId());
}
QString AppController::currentPlanExplanation() const
{
    return m_threadPlanExplanations.value(selectedThreadId());
}
bool AppController::turnRunning() const { return m_turnRunning; }
QString AppController::turnElapsedText() const
{
    return elapsedText(m_turnElapsedTimer.isValid() ? m_turnElapsedTimer.elapsed() : 0);
}
bool AppController::providerReady() const { return m_codex.ready(); }
QString AppController::providerVersion() const { return m_codex.version(); }
QString AppController::statusText() const { return m_status; }
QString AppController::diffText() const { return m_diff; }
QString AppController::gitStatusText() const { return m_gitStatus; }
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
    for (const auto &project : m_database.projects()) {
        ProjectRow row;
        row.id = project.value(QStringLiteral("id")).toLongLong();
        row.path = project.value(QStringLiteral("path")).toString();
        row.name = project.value(QStringLiteral("name")).toString();
        row.git = GitService::isRepository(row.path);
        row.threadCount = m_database.threadBindings(row.id).size();
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
    m_threadPlans.remove(threadId);
    m_threadPlanExplanations.remove(threadId);
    if (removedSelected) {
        m_selectedThread = -1;
        m_conversation.setThread({});
    } else if (index < m_selectedThread) {
        --m_selectedThread;
    }
    emit threadsChanged();
    emit selectedThreadChanged();
    emit currentPlanChanged();

    if (removedSelected && !m_threads.isEmpty())
        selectThread(qMin(index, m_threads.size() - 1));
}

void AppController::selectProject(int index)
{
    if (index < 0 || index >= m_projects.rowCount())
        return;
    m_selectedProject = index;
    m_selectedThread = -1;
    m_conversation.setThread({});
    emit selectedProjectChanged();
    emit selectedThreadChanged();
    emit currentPlanChanged();
    loadThreads();
    refreshGit();
}

void AppController::loadThreads(const QString &threadToSelect)
{
    m_threads.clear();
    emit threadsChanged();
    const auto project = m_projects.row(m_selectedProject);
    if (project.id < 0 || !m_codex.ready())
        return;
    const auto bindings = m_database.threadBindings(project.id);
    const auto hiddenThreadIds = m_database.hiddenThreadIds(project.id);
    QHash<QString, QVariantMap> bindingById;
    for (const auto &binding : bindings)
        bindingById.insert(binding.value(QStringLiteral("threadId")).toString(), binding);
    m_codex.request(QStringLiteral("thread/list"),
        {{QStringLiteral("cwd"), QJsonArray{project.path}},
         {QStringLiteral("limit"), 100},
         {QStringLiteral("sortKey"), QStringLiteral("updated_at")},
         {QStringLiteral("sortDirection"), QStringLiteral("desc")}},
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
            if (!known)
                m_database.bindThread(project.id, id, project.path, true);
        }
        m_threads = rows;
        emit threadsChanged();
        if (!threadToSelect.isEmpty()) {
            for (int i = 0; i < m_threads.size(); ++i) {
                if (m_threads.at(i).toMap().value(QStringLiteral("id")).toString() != threadToSelect)
                    continue;
                if (threadToSelect == m_activeThreadId) {
                    m_selectedThread = i;
                    emit selectedThreadChanged();
                    emit currentPlanChanged();
                } else {
                    selectThread(i);
                }
                if (!m_pendingPrompt.isEmpty()) {
                    const QString prompt = std::exchange(m_pendingPrompt, {});
                    m_pendingModelId.clear();
                    startPromptTurn(selectedThreadId(), prompt, m_pendingPermissionProfile, true);
                }
                break;
            }
        }
    });
}

void AppController::loadModels()
{
    m_codex.listModels([this](const QJsonObject &result, const QString &error) {
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
    emit currentPlanChanged();
    const auto threadId = selectedThreadId();
    m_codex.resumeThread(threadId, [this, threadId](const QJsonObject &result, const QString &error) {
        if (!error.isEmpty()) {
            setStatus(error);
            return;
        }
        if (threadId != selectedThreadId())
            return;
        const auto persistedEvents = m_database.conversationEvents(threadId);
        if (!persistedEvents.isEmpty()) {
            for (const auto &stored : persistedEvents) {
                const auto type = stored.value(QStringLiteral("type")).toString();
                const auto content = stored.value(QStringLiteral("content")).toString();
                const auto metadata = stored.value(QStringLiteral("metadata")).toMap();
                m_conversation.append(
                    {threadId,
                     type,
                     stored.value(QStringLiteral("title")).toString(),
                     content,
                     metadata});
                if (type == QStringLiteral("plan")) {
                    m_threadPlans.insert(threadId, metadata.value(QStringLiteral("plan")).toList());
                    m_threadPlanExplanations.insert(
                        threadId, metadata.value(QStringLiteral("explanation")).toString());
                } else if (type == QStringLiteral("diff")) {
                    m_diff = content;
                }
            }
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
                const auto content = CodexClient::itemContent(item);
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
                } else {
                    continue;
                }
                ConversationEvent event{threadId, eventType, title, content,
                    {{QStringLiteral("lifecycle"), QStringLiteral("completed")},
                     {QStringLiteral("itemId"), item.value(QStringLiteral("id")).toString()},
                     {QStringLiteral("turnId"), turn.value(QStringLiteral("id")).toString()}}};
                m_conversation.append(event);
                m_database.saveConversationEvent(
                    event.threadId, event.type, event.title, event.content, event.metadata);
            }
        }
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

void AppController::createThread(const QString &modelId, const QString &permissionMode)
{
    const auto project = m_projects.row(m_selectedProject);
    if (project.id < 0 || !m_codex.ready())
        return;
    if (m_threadCreationPending)
        return;
    m_threadCreationPending = true;
    beginThread(modelId, permissionProfile(permissionMode));
}

void AppController::beginThread(const QString &modelId, PermissionProfile permissionProfile)
{
    const auto project = m_projects.row(m_selectedProject);
    ThreadConfiguration config{project.path, project.path, modelId, {}, permissionProfile, false};
    m_codex.startThread(config, [this, project, permissionProfile](const QJsonObject &result, const QString &error) {
        m_threadCreationPending = false;
        if (!error.isEmpty()) {
            if (!m_pendingPrompt.isEmpty())
                emit promptRestoreRequested(m_pendingPrompt);
            m_pendingPrompt.clear();
            m_pendingModelId.clear();
            setStatus(error);
            return;
        }
        const auto thread = result.value(QStringLiteral("thread")).toObject();
        const auto id = thread.value(QStringLiteral("id")).toString();
        m_database.bindThread(project.id, id, project.path, false);
        loadProjects();
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
        emit currentPlanChanged();
        if (!m_pendingPrompt.isEmpty()) {
            const QString prompt = std::exchange(m_pendingPrompt, {});
            m_pendingModelId.clear();
            startPromptTurn(id, prompt, permissionProfile, true);
        } else {
            setStatus(QStringLiteral("Thread created"));
        }
    });
}

void AppController::sendPrompt(const QString &text, const QString &modelId,
                               const QString &permissionMode)
{
    const QString prompt = text.trimmed();
    if (prompt.isEmpty())
        return;
    if (selectedThreadId().isEmpty()) {
        if (selectedProjectPath().isEmpty() || !m_codex.ready() || m_threadCreationPending)
            return;
        m_pendingPrompt = prompt;
        m_pendingModelId = modelId;
        m_pendingPermissionProfile = permissionProfile(permissionMode);
        createThread(modelId, permissionMode);
        return;
    }
    if (m_turnRunning) {
        if (selectedThreadId() != m_activeThreadId) {
            setStatus(QStringLiteral("Another thread is active in this Artemis session. Stop it before starting this turn."));
            return;
        }
        m_codex.steerTurn(selectedThreadId(), text, [this](const QJsonObject &, const QString &error) {
            if (!error.isEmpty())
                setStatus(error);
        });
        return;
    }
    const bool generateTitle = m_selectedThread >= 0
        && !m_threads.at(m_selectedThread).toMap().value(QStringLiteral("named")).toBool();
    startPromptTurn(selectedThreadId(), prompt, permissionProfile(permissionMode), generateTitle);
}

void AppController::startPromptTurn(const QString &threadId, const QString &prompt,
                                    PermissionProfile permissionProfile, bool generateTitle)
{
    m_conversation.setThread(threadId);
    const ConversationEvent userEvent{
        threadId, QStringLiteral("user"), QStringLiteral("You"), prompt, {}};
    m_conversation.append(userEvent);
    m_database.saveConversationEvent(userEvent.threadId, userEvent.type, userEvent.title,
                                     userEvent.content, userEvent.metadata);
    m_activeThreadId = threadId;
    setTurnRunning(true);
    m_codex.sendTurn(threadId, prompt, {}, permissionProfile,
                     [this, threadId, prompt, generateTitle](const QJsonObject &,
                                                             const QString &error) {
        if (!error.isEmpty()) {
            setTurnRunning(false);
            setStatus(error);
            emit promptRestoreRequested(prompt);
            return;
        }
        if (generateTitle)
            generateThreadTitle(threadId, prompt);
        loadThreads(threadId);
    });
}

void AppController::interruptTurn()
{
    if (!m_turnRunning || m_activeThreadId.isEmpty())
        return;
    m_codex.interruptTurn(m_activeThreadId, [this](const QJsonObject &, const QString &error) {
        if (!error.isEmpty())
            setStatus(error);
        setTurnRunning(false);
        m_activeThreadId.clear();
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
            emit commitDraftReady(cleanCommitDraft(m_commitDraftBuffer));
            m_commitThreadId.clear();
            m_commitDraftBuffer.clear();
        }
        return;
    }
    ConversationEvent event{threadId, type, title, content, metadata};
    if (type == QStringLiteral("status") && content == QStringLiteral("completed")) {
        event.content = QStringLiteral("Complete · %1 · %2 total")
                            .arg(QLocale().toString(QTime::currentTime(), QLocale::ShortFormat),
                                 turnElapsedText());
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
        m_database.saveConversationEvent(
            event.threadId, event.type, event.title, persistedContent, event.metadata);
    }
    if (type == QStringLiteral("plan")) {
        m_threadPlans.insert(threadId, metadata.value(QStringLiteral("plan")).toList());
        m_threadPlanExplanations.insert(
            threadId, metadata.value(QStringLiteral("explanation")).toString());
        if (threadId == selectedThreadId())
            emit currentPlanChanged();
    } else if (type == QStringLiteral("assistant"))
        m_conversation.appendOrMergeDelta(event);
    else
        m_conversation.append(event);
    if (type == QStringLiteral("diff")) {
        m_diff = content;
        emit diffChanged();
    }
    if (type == QStringLiteral("status")) {
        setTurnRunning(false);
        m_activeThreadId.clear();
        refreshGit();
    }
}

void AppController::refreshGit()
{
    const auto path = selectedWorkspacePath();
    if (path.isEmpty() || !selectedProjectIsGit()) {
        m_diff.clear();
        m_gitStatus = QStringLiteral("This folder is not a Git repository.");
        emit diffChanged();
        return;
    }
    m_git.status(path, [this](const GitResult &result) {
        m_gitStatus = result.ok() ? QString::fromUtf8(result.output).replace(QChar(u'\0'), QChar(u'\n'))
                                  : QString::fromUtf8(result.error);
        emit diffChanged();
    });
    m_git.diff(path, [this](const GitResult &result) {
        m_diff = result.ok() ? QString::fromUtf8(result.output) : QString::fromUtf8(result.error);
        emit diffChanged();
    });
}

void AppController::generateCommitMessage()
{
    if (!selectedProjectIsGit())
        return;
    setStatus(QStringLiteral("Preparing commit snapshot…"));
    m_git.generateCommitSnapshot(selectedWorkspacePath(), [this](const GitResult &snapshot) {
        if (!snapshot.ok()) {
            setStatus(QString::fromUtf8(snapshot.error));
            return;
        }
        const auto workspace = selectedWorkspacePath();
        ThreadConfiguration config{selectedProjectPath(), workspace, m_commitModelId, {},
                                   PermissionProfile::ReadOnly, true};
        m_codex.startThread(config, [this, snapshot](const QJsonObject &result, const QString &error) {
            if (!error.isEmpty()) {
                setStatus(error);
                return;
            }
            m_commitThreadId = result.value(QStringLiteral("thread")).toObject()
                                   .value(QStringLiteral("id")).toString();
            m_commitDraftBuffer.clear();
            m_codex.sendTurn(m_commitThreadId, commitPrompt(snapshot.output), {},
                PermissionProfile::ReadOnly,
                [this](const QJsonObject &, const QString &turnError) {
                if (!turnError.isEmpty()) {
                    setStatus(turnError);
                    m_commitThreadId.clear();
                }
            });
            setStatus(QStringLiteral("Generating commit message…"));
        });
    });
}

void AppController::generateThreadTitle(const QString &threadId, const QString &prompt)
{
    ThreadConfiguration config{selectedProjectPath(), selectedWorkspacePath(), m_titleModelId, {},
                               PermissionProfile::ReadOnly, true};
    m_codex.startThread(config, [this, threadId, prompt](const QJsonObject &result,
                                                         const QString &error) {
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
        m_codex.sendTurn(titleThreadId, titlePrompt, {}, PermissionProfile::ReadOnly,
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
    m_codex.request(QStringLiteral("thread/name/set"),
                    {{QStringLiteral("threadId"), threadId},
                     {QStringLiteral("name"), title}},
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

QString AppController::commitPrompt(const QByteArray &snapshot) const
{
    const auto branch = GitService::runSync(selectedWorkspacePath(),
        {QStringLiteral("branch"), QStringLiteral("--show-current")});
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
        .arg(selectedProjectName(), QString::fromUtf8(branch.output).trimmed(),
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

void AppController::setCommitModelId(const QString &modelId)
{
    setModelSetting(QStringLiteral("commit_model"), m_commitModelId, modelId);
}

void AppController::setTitleModelId(const QString &modelId)
{
    setModelSetting(QStringLiteral("title_model"), m_titleModelId, modelId);
}

void AppController::commitAllAndPush(const QString &subject, const QString &body)
{
    m_git.commitAllAndPush(selectedWorkspacePath(), subject, body, [this](const GitResult &result) {
        const auto output = QString::fromUtf8(result.ok()
            ? result.output + result.error : result.error).trimmed();
        emit commitFinished(result.ok(), output);
        setStatus(output);
        refreshGit();
    });
}

void AppController::commitFeatureBranch(const QString &subject, const QString &body,
                                        const QString &branch, const QString &remote)
{
    const auto validation = validateBranch(branch);
    if (!validation.isEmpty()) {
        emit commitFinished(false, validation);
        return;
    }
    m_git.createBranchCommitPush(selectedWorkspacePath(), branch, subject, body,
                                 remote.isEmpty() ? QStringLiteral("origin") : remote,
                                 [this](const GitResult &result) {
        const auto output = QString::fromUtf8(result.ok()
            ? result.output + result.error : result.error).trimmed();
        emit commitFinished(result.ok(), output);
        setStatus(output);
        refreshGit();
    });
}

QString AppController::suggestBranch(const QString &message) const
{
    return GitService::suggestedBranch(message.section(QChar(u'\n'), 0, 0));
}

QString AppController::validateBranch(const QString &branch) const
{
    return GitService::validateBranchName(selectedWorkspacePath(), branch);
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
    QDesktopServices::openUrl(QUrl::fromLocalFile(selectedProjectPath()));
}

void AppController::openTerminal()
{
    if (selectedWorkspacePath().isEmpty())
        return;
    QProcess::startDetached(QStringLiteral("xdg-terminal-exec"), {},
                            selectedWorkspacePath());
}

void AppController::setStatus(const QString &text)
{
    if (m_status == text)
        return;
    m_status = text;
    emit statusTextChanged();
}

void AppController::setTurnRunning(bool running)
{
    if (m_turnRunning == running)
        return;
    m_turnRunning = running;
    if (running) {
        m_turnElapsedTimer.restart();
        m_turnElapsedUpdateTimer.start();
    } else {
        m_turnElapsedUpdateTimer.stop();
    }
    emit turnRunningChanged();
    emit turnElapsedChanged();
}

} // namespace Artemis
