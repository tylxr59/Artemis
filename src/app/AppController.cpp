#include "app/AppController.h"
#include "platform/Paths.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
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

} // namespace

AppController::AppController(QObject *parent)
    : QObject(parent)
{
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
    loadProjects();
    m_codex.start();
    return true;
}

ProjectTreeModel *AppController::projects() { return &m_projects; }
ConversationModel *AppController::conversation() { return &m_conversation; }
QVariantList AppController::threads() const { return m_threads; }
QVariantList AppController::models() const { return m_models; }
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
bool AppController::turnRunning() const { return m_turnRunning; }
bool AppController::providerReady() const { return m_codex.ready(); }
QString AppController::providerVersion() const { return m_codex.version(); }
QString AppController::statusText() const { return m_status; }
QString AppController::diffText() const { return m_diff; }
QString AppController::gitStatusText() const { return m_gitStatus; }
QString AppController::databasePath() const { return m_database.path(); }
QString AppController::worktreeRoot() const { return Paths::worktreeRoot(); }

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
    const auto row = m_projects.row(m_selectedProject);
    if (row.id < 0)
        return;
    QString error;
    if (!m_database.removeProject(row.id, &error)) {
        setStatus(error);
        return;
    }
    m_selectedProject = -1;
    m_selectedThread = -1;
    m_threads.clear();
    m_conversation.clear();
    loadProjects();
    emit selectedProjectChanged();
    emit selectedThreadChanged();
    emit threadsChanged();
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
    QJsonArray workspaces;
    workspaces.append(project.path);
    QHash<QString, QVariantMap> bindingById;
    for (const auto &binding : bindings) {
        bindingById.insert(binding.value(QStringLiteral("threadId")).toString(), binding);
        const auto workspace = binding.value(QStringLiteral("workspacePath")).toString();
        bool present = false;
        for (const auto &existing : workspaces)
            present = present || existing.toString() == workspace;
        if (!present)
            workspaces.append(workspace);
    }
    m_codex.request(QStringLiteral("thread/list"),
        {{QStringLiteral("cwd"), workspaces},
         {QStringLiteral("limit"), 100},
         {QStringLiteral("sortKey"), QStringLiteral("updated_at")},
         {QStringLiteral("sortDirection"), QStringLiteral("desc")}},
        [this, project, bindingById, threadToSelect](const QJsonObject &result, const QString &error) {
        if (!error.isEmpty()) {
            setStatus(error);
            return;
        }
        QVariantList rows;
        for (const auto &entry : result.value(QStringLiteral("data")).toArray()) {
            const auto thread = entry.toObject();
            const auto id = thread.value(QStringLiteral("id")).toString();
            const auto title = thread.value(QStringLiteral("name")).toString(
                thread.value(QStringLiteral("title")).toString(QStringLiteral("Untitled thread")));
            const auto cwd = thread.value(QStringLiteral("cwd")).toString(project.path);
            const auto binding = bindingById.value(id);
            const bool known = !binding.isEmpty();
            rows.push_back(QVariantMap{{QStringLiteral("id"), id},
                                       {QStringLiteral("title"), title},
                                       {QStringLiteral("cwd"), cwd},
                                       {QStringLiteral("model"), thread.value(QStringLiteral("model")).toString()},
                                       {QStringLiteral("external"), known
                                            ? binding.value(QStringLiteral("external")).toBool() : true},
                                       {QStringLiteral("location"), known
                                            ? binding.value(QStringLiteral("location")).toString()
                                            : (cwd == project.path ? QStringLiteral("local") : QStringLiteral("worktree"))}});
            m_database.bindThread(project.id, id, cwd,
                                  cwd == project.path ? QStringLiteral("local") : QStringLiteral("worktree"),
                                  true);
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
                } else {
                    selectThread(i);
                }
                if (!m_pendingPrompt.isEmpty()) {
                    const QString prompt = std::exchange(m_pendingPrompt, {});
                    m_pendingModelId.clear();
                    startPromptTurn(selectedThreadId(), prompt, m_pendingPermissionProfile);
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
    m_codex.resumeThread(selectedThreadId(), [this](const QJsonObject &result, const QString &error) {
        if (!error.isEmpty()) {
            setStatus(error);
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
                m_conversation.append({selectedThreadId(), eventType, title, content,
                    {{QStringLiteral("lifecycle"), QStringLiteral("completed")},
                     {QStringLiteral("itemId"), item.value(QStringLiteral("id")).toString()}}});
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

void AppController::createThread(bool worktree, const QString &modelId,
                                 const QString &permissionMode)
{
    const auto project = m_projects.row(m_selectedProject);
    if (project.id < 0 || !m_codex.ready())
        return;
    if (worktree && !project.git) {
        setStatus(QStringLiteral("Managed worktrees require a Git repository"));
        return;
    }
    if (m_threadCreationPending)
        return;
    m_threadCreationPending = true;
    const auto profile = permissionProfile(permissionMode);
    if (!worktree) {
        beginThread(project.path, QStringLiteral("local"), modelId, profile, false);
        return;
    }
    const auto token = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const auto destination = QDir(Paths::worktreeRoot())
        .filePath(QStringLiteral("%1/%2").arg(project.id).arg(token));
    setStatus(QStringLiteral("Creating managed worktree…"));
    m_git.createWorktree(project.path, destination,
                         [this, destination, modelId, profile](const GitResult &result) {
        if (!result.ok()) {
            m_threadCreationPending = false;
            setStatus(QString::fromUtf8(result.error));
            return;
        }
        beginThread(destination, QStringLiteral("worktree"), modelId, profile, true);
    });
}

void AppController::beginThread(const QString &workspace, const QString &location,
                                const QString &modelId, PermissionProfile permissionProfile,
                                bool saveWorktree)
{
    const auto project = m_projects.row(m_selectedProject);
    ThreadConfiguration config{project.path, workspace, modelId, {}, permissionProfile, false};
    m_codex.startThread(config, [this, project, workspace, location, permissionProfile, saveWorktree](const QJsonObject &result, const QString &error) {
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
        m_database.bindThread(project.id, id, workspace, location, false);
        if (saveWorktree) {
            const auto head = GitService::runSync(workspace, {QStringLiteral("rev-parse"), QStringLiteral("HEAD")});
            m_database.saveWorktree(project.id, id, workspace,
                                    QString::fromUtf8(head.output).trimmed());
        }
        loadProjects();
        if (!m_pendingPrompt.isEmpty()) {
            const QString prompt = std::exchange(m_pendingPrompt, {});
            m_pendingModelId.clear();
            startPromptTurn(id, prompt, permissionProfile);
        } else {
            loadThreads(id);
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
        createThread(false, modelId, permissionMode);
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
    startPromptTurn(selectedThreadId(), prompt, permissionProfile(permissionMode));
}

void AppController::startPromptTurn(const QString &threadId, const QString &prompt,
                                    PermissionProfile permissionProfile)
{
    m_conversation.setThread(threadId);
    m_conversation.append({threadId, QStringLiteral("user"), QStringLiteral("You"), prompt, {}});
    m_activeThreadId = threadId;
    setTurnRunning(true);
    m_codex.sendTurn(threadId, prompt, {}, permissionProfile,
                     [this, threadId, prompt](const QJsonObject &, const QString &error) {
        if (!error.isEmpty()) {
            setTurnRunning(false);
            setStatus(error);
            emit promptRestoreRequested(prompt);
            return;
        }
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
    if (type == QStringLiteral("assistant"))
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

void AppController::generateCommitMessage(const QString &modelId)
{
    if (!selectedProjectIsGit())
        return;
    setStatus(QStringLiteral("Preparing commit snapshot…"));
    m_git.generateCommitSnapshot(selectedWorkspacePath(), [this, modelId](const GitResult &snapshot) {
        if (!snapshot.ok()) {
            setStatus(QString::fromUtf8(snapshot.error));
            return;
        }
    const auto workspace = selectedWorkspacePath();
    ThreadConfiguration config{selectedProjectPath(), workspace, modelId, {},
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

QString AppController::commitPrompt(const QByteArray &snapshot) const
{
    const auto branch = GitService::runSync(selectedWorkspacePath(),
        {QStringLiteral("branch"), QStringLiteral("--show-current")});
    return QStringLiteral(
        "Generate a Git commit message for the following staged snapshot.\n"
        "Return JSON only with this schema: {\"subject\":\"...\",\"body\":\"...\"}.\n"
        "Use imperative mood, keep the subject concise, describe intent, and never invent issue numbers.\n"
        "Repository: %1\nBranch: %2\n\n%3")
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
        const auto subject = object.value(QStringLiteral("subject")).toString().trimmed();
        const auto body = object.value(QStringLiteral("body")).toString().trimmed();
        if (!subject.isEmpty())
            return body.isEmpty() ? subject : subject + QStringLiteral("\n\n") + body;
    }
    return text;
}

void AppController::commitAll(const QString &message)
{
    m_git.commitAll(selectedWorkspacePath(), message, [this](const GitResult &result) {
        const auto output = QString::fromUtf8(result.ok() ? result.output : result.error).trimmed();
        emit commitFinished(result.ok(), output);
        setStatus(output);
        refreshGit();
    });
}

void AppController::commitFeatureBranch(const QString &message, const QString &branch,
                                        const QString &remote)
{
    const auto validation = validateBranch(branch);
    if (!validation.isEmpty()) {
        emit commitFinished(false, validation);
        return;
    }
    m_git.createBranchCommitPush(selectedWorkspacePath(), branch, message,
                                 remote.isEmpty() ? QStringLiteral("origin") : remote,
                                 [this](const GitResult &result) {
        const auto output = QString::fromUtf8(result.ok() ? result.output : result.error).trimmed();
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
    emit turnRunningChanged();
}

} // namespace Artemis
