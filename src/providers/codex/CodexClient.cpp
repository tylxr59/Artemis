#include "providers/codex/CodexClient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>

#include <memory>
#include <utility>

namespace Artemis {

namespace {

QString codexExecutable()
{
    const auto override = qEnvironmentVariable("ARTEMIS_CODEX_EXECUTABLE").trimmed();
    return override.isEmpty() ? QStringLiteral("codex") : override;
}

QString approvalPolicy(PermissionProfile profile)
{
    switch (profile) {
    case PermissionProfile::ReadOnly:
        return QStringLiteral("untrusted");
    case PermissionProfile::WorkspaceWrite:
        return QStringLiteral("on-request");
    case PermissionProfile::FullAccess:
        return QStringLiteral("never");
    }
    return QStringLiteral("never");
}

QString sandboxMode(PermissionProfile profile)
{
    switch (profile) {
    case PermissionProfile::ReadOnly:
        return QStringLiteral("read-only");
    case PermissionProfile::WorkspaceWrite:
        return QStringLiteral("workspace-write");
    case PermissionProfile::FullAccess:
        return QStringLiteral("danger-full-access");
    }
    return QStringLiteral("danger-full-access");
}

QJsonObject sandboxPolicy(PermissionProfile profile)
{
    switch (profile) {
    case PermissionProfile::ReadOnly:
        return {{QStringLiteral("type"), QStringLiteral("readOnly")}};
    case PermissionProfile::WorkspaceWrite:
        return {{QStringLiteral("type"), QStringLiteral("workspaceWrite")}};
    case PermissionProfile::FullAccess:
        return {{QStringLiteral("type"), QStringLiteral("dangerFullAccess")}};
    }
    return {{QStringLiteral("type"), QStringLiteral("dangerFullAccess")}};
}

void appendText(const QJsonValue &value, QStringList &parts)
{
    if (value.isString()) {
        const auto text = value.toString().trimmed();
        if (!text.isEmpty())
            parts.push_back(text);
        return;
    }
    if (value.isArray()) {
        for (const auto &entry : value.toArray())
            appendText(entry, parts);
        return;
    }
    if (!value.isObject())
        return;

    const auto object = value.toObject();
    for (const auto &key : {QStringLiteral("text"), QStringLiteral("output_text"),
                            QStringLiteral("summary"), QStringLiteral("content")}) {
        if (object.contains(key))
            appendText(object.value(key), parts);
    }
}

} // namespace

CodexClient::CodexClient(QObject *parent)
    : AgentProvider(parent)
{
    m_restartTimer.setSingleShot(true);
    connect(&m_restartTimer, &QTimer::timeout, this, [this] {
        if (m_process.state() != QProcess::NotRunning) {
            m_restartTimer.start(100);
            return;
        }
        m_restartScheduled = false;
        startProcess();
    });
    connect(&m_process, &QProcess::readyReadStandardOutput, this, [this] {
        m_buffer += m_process.readAllStandardOutput();
        while (true) {
            const auto newline = m_buffer.indexOf('\n');
            if (newline < 0)
                break;
            const auto line = m_buffer.left(newline).trimmed();
            m_buffer.remove(0, newline + 1);
            if (!line.isEmpty())
                handleLine(line);
        }
    });
    connect(&m_process, &QProcess::started, this, &CodexClient::initializeProcess);
    connect(&m_process, &QProcess::readyReadStandardError, this, [this] {
        const auto text = QString::fromUtf8(m_process.readAllStandardError()).trimmed();
        if (!text.isEmpty())
            emit providerError(QStringLiteral("Codex: %1").arg(text));
    });
    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
        scheduleRestart(m_process.errorString());
    });
    connect(&m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus) {
        if (!m_stopping)
            scheduleRestart(QStringLiteral("Codex app-server exited with code %1").arg(code));
    });
}

CodexClient::~CodexClient()
{
    m_stopping = true;

    const auto stopProcess = [this](QProcess *process) {
        disconnect(process, nullptr, this, nullptr);
        if (process->state() == QProcess::NotRunning)
            return;

        process->closeWriteChannel();
        if (process->waitForFinished(1000))
            return;

        process->terminate();
        if (process->waitForFinished(1000))
            return;

        process->kill();
        process->waitForFinished(1000);
    };

    stopProcess(&m_process);
    const auto childProcesses = findChildren<QProcess *>(QString(), Qt::FindDirectChildrenOnly);
    for (auto *process : childProcesses)
        stopProcess(process);

    for (auto &pending : m_pending) {
        if (pending.timeout)
            pending.timeout->stop();
    }
    m_pending.clear();
}

ProviderCapabilities CodexClient::capabilities() const
{
    return {true, true, true, true, true, true, true};
}

void CodexClient::start()
{
    startVersionProbe();
}

void CodexClient::startVersionProbe()
{
    auto *probe = new QProcess(this);
    auto *timeout = new QTimer(probe);
    timeout->setSingleShot(true);
    const auto finishWithError = [this, probe, timeout,
                                  completed = std::make_shared<bool>(false)](
                                     const QString &message) {
        if (std::exchange(*completed, true))
            return false;
        timeout->stop();
        probe->deleteLater();
        emit providerError(message);
        return true;
    };
    connect(probe, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, probe, timeout, finishWithError](int code, QProcess::ExitStatus status) {
        if (status != QProcess::NormalExit || code != 0) {
            finishWithError(QStringLiteral(
                "Codex CLI was not found. Install it and run codex login."));
            return;
        }
        timeout->stop();
        const auto output = QString::fromUtf8(probe->readAllStandardOutput()).trimmed();
        probe->deleteLater();
        m_version = output;
        emit versionChanged();
        const QRegularExpression expression(QStringLiteral("(\\d+)\\.(\\d+)\\.(\\d+)"));
        const auto match = expression.match(output);
        if (!match.hasMatch()
            || (match.captured(1).toInt() == 0 && match.captured(2).toInt() < 141)) {
            emit providerError(QStringLiteral("Artemis requires Codex CLI 0.141.0 or newer; found %1").arg(output));
            return;
        }
        startProcess();
    });
    connect(probe, &QProcess::errorOccurred, this,
            [finishWithError](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            finishWithError(QStringLiteral(
                "Codex CLI was not found. Install it and run codex login."));
        }
    });
    connect(timeout, &QTimer::timeout, this, [probe, finishWithError] {
        probe->kill();
        finishWithError(QStringLiteral("Timed out while checking the Codex CLI version."));
    });
    probe->start(codexExecutable(), {QStringLiteral("--version")});
    timeout->start(5000);
}

void CodexClient::startProcess()
{
    if (m_process.state() != QProcess::NotRunning)
        return;
    m_stopping = false;
    m_process.setProgram(codexExecutable());
    m_process.setArguments({QStringLiteral("app-server"), QStringLiteral("--listen"), QStringLiteral("stdio://")});
    m_process.start();
}

void CodexClient::initializeProcess()
{
    request(QStringLiteral("initialize"),
            {{QStringLiteral("clientInfo"),
              QJsonObject{{QStringLiteral("name"), QStringLiteral("artemis")},
                          {QStringLiteral("title"), QStringLiteral("Artemis")},
                          {QStringLiteral("version"), QStringLiteral(ARTEMIS_VERSION)}}},
             {QStringLiteral("capabilities"),
              QJsonObject{{QStringLiteral("experimentalApi"), true}}}},
            [this](const QJsonObject &, const QString &error) {
        if (!error.isEmpty()) {
            scheduleRestart(error);
            return;
        }
        QJsonObject initialized{{QStringLiteral("method"), QStringLiteral("initialized")},
                                {QStringLiteral("params"), QJsonObject{}}};
        m_process.write(QJsonDocument(initialized).toJson(QJsonDocument::Compact) + '\n');
        m_restartAttempt = 0;
        refreshAccountState();
    });
}

void CodexClient::refreshAccountState()
{
    request(QStringLiteral("account/read"),
            {{QStringLiteral("refreshToken"), false}},
            [this](const QJsonObject &result, const QString &error) {
        if (!error.isEmpty()) {
            scheduleRestart(error);
            return;
        }

        const bool requiresOpenAiAuth =
            result.value(QStringLiteral("requiresOpenaiAuth")).toBool(true);
        const bool hasAccount = result.value(QStringLiteral("account")).isObject();
        if (requiresOpenAiAuth && !hasAccount) {
            setReady(false);
            const auto instructions = QStringLiteral(
                "Codex is installed but is not signed in. Run `codex login` in a terminal, "
                "complete sign-in, then restart Artemis.");
            setSetupRequired(true, instructions);
            emit providerError(instructions);
            return;
        }

        setSetupRequired(false);
        setReady(true);
    });
}

void CodexClient::scheduleRestart(const QString &reason)
{
    setReady(false);
    if (m_restartScheduled)
        return;
    m_restartScheduled = true;
    auto pendingRequests = std::exchange(m_pending, {});
    for (auto &pending : pendingRequests) {
        if (pending.timeout)
            pending.timeout->deleteLater();
        if (pending.handler)
            pending.handler({}, QStringLiteral("Codex disconnected: %1").arg(reason));
    }
    if (!reason.isEmpty())
        emit providerError(reason);
    if (m_stopping)
        return;
    const int delay = qMin(16000, 500 * (1 << qMin(m_restartAttempt++, 5)));
    if (m_process.state() != QProcess::NotRunning)
        m_process.kill();
    m_buffer.clear();
    m_restartTimer.start(delay);
}

void CodexClient::request(const QString &method, const QJsonObject &params, ResultHandler handler)
{
    if (m_process.state() != QProcess::Running) {
        if (handler)
            handler({}, QStringLiteral("Codex app-server is not running"));
        return;
    }
    const qint64 id = m_nextId++;
    QJsonObject message{{QStringLiteral("id"), id},
                        {QStringLiteral("method"), method},
                        {QStringLiteral("params"), params}};
    auto *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this, id] {
        auto pending = m_pending.take(id);
        if (pending.handler)
            pending.handler({}, QStringLiteral("Codex request timed out"));
        if (pending.timeout)
            pending.timeout->deleteLater();
    });
    timer->start(60000);
    m_pending.insert(id, {std::move(handler), timer});
    m_process.write(QJsonDocument(message).toJson(QJsonDocument::Compact) + '\n');
}

void CodexClient::handleLine(const QByteArray &line)
{
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(line, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        emit providerError(QStringLiteral("Invalid Codex protocol message: %1").arg(parseError.errorString()));
        return;
    }
    const auto object = document.object();
    if (object.contains(QStringLiteral("id"))) {
        const auto method = object.value(QStringLiteral("method")).toString();
        if (!method.isEmpty()) {
            handleServerRequest(object.value(QStringLiteral("id")), method,
                                object.value(QStringLiteral("params")).toObject());
            return;
        }
        const auto id = object.value(QStringLiteral("id")).toInteger();
        auto pending = m_pending.take(id);
        if (pending.timeout)
            pending.timeout->deleteLater();
        if (pending.handler) {
            const auto error = object.value(QStringLiteral("error")).toObject();
            pending.handler(object.value(QStringLiteral("result")).toObject(),
                            error.value(QStringLiteral("message")).toString());
        }
        return;
    }
    handleNotification(object.value(QStringLiteral("method")).toString(),
                       object.value(QStringLiteral("params")).toObject());
}

void CodexClient::handleServerRequest(const QJsonValue &id, const QString &method,
                                      const QJsonObject &params)
{
    if (method != QStringLiteral("item/tool/requestUserInput")) {
        QJsonObject response{{QStringLiteral("id"), id},
                             {QStringLiteral("error"),
                              QJsonObject{{QStringLiteral("code"), -32601},
                                          {QStringLiteral("message"),
                                           QStringLiteral("Unsupported server request: %1")
                                               .arg(method)}}}};
        m_process.write(QJsonDocument(response).toJson(QJsonDocument::Compact) + '\n');
        return;
    }

    const auto itemId = params.value(QStringLiteral("itemId")).toString();
    if (itemId.isEmpty()) {
        QJsonObject response{{QStringLiteral("id"), id},
                             {QStringLiteral("error"),
                              QJsonObject{{QStringLiteral("code"), -32602},
                                          {QStringLiteral("message"),
                                           QStringLiteral("Missing user-input item id")}}}};
        m_process.write(QJsonDocument(response).toJson(QJsonDocument::Compact) + '\n');
        return;
    }

    m_pendingUserInputRequests.insert(itemId, id);
    emit userInputRequested(
        params.value(QStringLiteral("threadId")).toString(),
        params.value(QStringLiteral("turnId")).toString(), itemId,
        params.value(QStringLiteral("questions")).toArray().toVariantList());
}

void CodexClient::writeResponse(const QJsonValue &id, const QJsonObject &result)
{
    QJsonObject response{{QStringLiteral("id"), id},
                         {QStringLiteral("result"), result}};
    m_process.write(QJsonDocument(response).toJson(QJsonDocument::Compact) + '\n');
}

void CodexClient::handleNotification(const QString &method, const QJsonObject &params)
{
    const auto turnId = params.value(QStringLiteral("turnId")).toString();
    if (method == QStringLiteral("account/updated")) {
        refreshAccountState();
    } else if (method == QStringLiteral("turn/started")) {
        const auto turn = params.value(QStringLiteral("turn")).toObject();
        emit activeTurnStarted(params.value(QStringLiteral("threadId")).toString(),
                               turn.value(QStringLiteral("id")).toString());
    } else if (method == QStringLiteral("thread/tokenUsage/updated")) {
        const auto usage = params.value(QStringLiteral("tokenUsage")).toObject();
        emit tokenUsageUpdated(
            params.value(QStringLiteral("threadId")).toString(),
            usage.value(QStringLiteral("last")).toObject()
                .value(QStringLiteral("totalTokens")).toInteger(),
            usage.value(QStringLiteral("total")).toObject()
                .value(QStringLiteral("totalTokens")).toInteger(),
            usage.value(QStringLiteral("modelContextWindow")).toInteger());
    } else if (method == QStringLiteral("item/started")
               || method == QStringLiteral("item/completed")) {
        normalizeItem(method.endsWith(QStringLiteral("started")) ? QStringLiteral("started")
                                                                 : QStringLiteral("completed"),
                      params);
    } else if (method == QStringLiteral("item/agentMessage/delta")) {
        emit domainEvent(params.value(QStringLiteral("threadId")).toString(),
                         QStringLiteral("assistant"), QStringLiteral("Artemis"),
                         params.value(QStringLiteral("delta")).toString(),
                         {{QStringLiteral("delta"), true},
                          {QStringLiteral("itemId"),
                           params.value(QStringLiteral("itemId")).toString()},
                          {QStringLiteral("turnId"), turnId}});
    } else if (method == QStringLiteral("turn/diff/updated")) {
        emit domainEvent(params.value(QStringLiteral("threadId")).toString(),
                         QStringLiteral("diff"), QStringLiteral("Changes"),
                         params.value(QStringLiteral("diff")).toString(),
                         {{QStringLiteral("turnId"), turnId}});
    } else if (method == QStringLiteral("turn/plan/updated")) {
        const auto explanation = params.value(QStringLiteral("explanation")).toString();
        emit domainEvent(params.value(QStringLiteral("threadId")).toString(),
                         QStringLiteral("plan"), QStringLiteral("Plan"),
                         explanation,
                         {{QStringLiteral("explanation"), explanation},
                          {QStringLiteral("turnId"), turnId},
                          {QStringLiteral("plan"),
                           params.value(QStringLiteral("plan")).toArray().toVariantList()}});
    } else if (method == QStringLiteral("turn/completed")) {
        const auto turn = params.value(QStringLiteral("turn")).toObject();
        emit domainEvent(params.value(QStringLiteral("threadId")).toString(),
                         QStringLiteral("status"), QStringLiteral("Turn completed"),
                         turn.value(QStringLiteral("status")).toString(),
                         {{QStringLiteral("turnId"),
                           turn.value(QStringLiteral("id")).toString(turnId)}});
    } else if (method.contains(QStringLiteral("approval"), Qt::CaseInsensitive)) {
        emit domainEvent(params.value(QStringLiteral("threadId")).toString(),
                         QStringLiteral("approval"), QStringLiteral("Approval required"),
                         QString::fromUtf8(QJsonDocument(params).toJson(QJsonDocument::Indented)), {});
    } else if (method == QStringLiteral("error")) {
        emit domainEvent(params.value(QStringLiteral("threadId")).toString(),
                         QStringLiteral("error"), QStringLiteral("Provider error"),
                         params.value(QStringLiteral("message")).toString(), {});
    }
}

void CodexClient::normalizeItem(const QString &lifecycle, const QJsonObject &params)
{
    const auto item = params.value(QStringLiteral("item")).toObject();
    const auto type = item.value(QStringLiteral("type")).toString();
    const auto itemId = item.value(QStringLiteral("id")).toString();
    QString domainType = QStringLiteral("activity");
    QString title = type;
    if (type == QStringLiteral("commandExecution")) {
        domainType = QStringLiteral("command");
        title = lifecycle == QStringLiteral("started") ? QStringLiteral("Running command")
                                                        : QStringLiteral("Ran command");
    } else if (type == QStringLiteral("fileChange")) {
        domainType = QStringLiteral("file");
        title = QStringLiteral("File changes");
    } else if (type == QStringLiteral("agentMessage")) {
        if (lifecycle == QStringLiteral("started"))
            return;
        domainType = QStringLiteral("assistant");
        title = QStringLiteral("Artemis");
    } else if (type == QStringLiteral("userMessage")) {
        return;
    } else if (type == QStringLiteral("reasoning")) {
        if (lifecycle == QStringLiteral("started"))
            return;
        domainType = QStringLiteral("reasoning");
        title = QStringLiteral("Reasoning");
    } else if (type == QStringLiteral("plan")) {
        if (lifecycle == QStringLiteral("started"))
            return;
        domainType = QStringLiteral("task");
        title = QStringLiteral("Tasks");
    }
    const auto content = itemContent(item);
    if (content.isEmpty())
        return;
    emit domainEvent(params.value(QStringLiteral("threadId")).toString(), domainType,
                     title, content, {{QStringLiteral("lifecycle"), lifecycle},
                                      {QStringLiteral("itemId"), itemId},
                                      {QStringLiteral("turnId"),
                                       params.value(QStringLiteral("turnId")).toString()}});
}

QString CodexClient::itemContent(const QJsonObject &item) const
{
    for (const auto &key : {QStringLiteral("text"), QStringLiteral("command"),
                            QStringLiteral("aggregatedOutput"), QStringLiteral("diff")}) {
        const auto text = item.value(key).toString();
        if (!text.isEmpty())
            return text;
    }
    const auto content = item.value(QStringLiteral("content"));
    if (content.isString())
        return content.toString();
    QStringList parts;
    appendText(item.value(QStringLiteral("summary")), parts);
    appendText(content, parts);
    parts.removeDuplicates();
    return parts.join(QLatin1Char('\n'));
}

void CodexClient::listModels(ResultHandler handler)
{
    request(QStringLiteral("model/list"), {{QStringLiteral("limit"), 100}}, std::move(handler));
}

void CodexClient::listThreads(const QString &workspacePath, ResultHandler handler)
{
    request(QStringLiteral("thread/list"),
            {{QStringLiteral("cwd"), QJsonArray{workspacePath}},
             {QStringLiteral("limit"), 100},
             {QStringLiteral("sortKey"), QStringLiteral("updated_at")},
             {QStringLiteral("sortDirection"), QStringLiteral("desc")}},
            std::move(handler));
}

void CodexClient::startThread(const ThreadConfiguration &configuration, ResultHandler handler)
{
    QJsonObject params{{QStringLiteral("cwd"), configuration.workspacePath},
                       {QStringLiteral("ephemeral"), configuration.ephemeral},
                       {QStringLiteral("approvalPolicy"), approvalPolicy(configuration.permissionProfile)},
                       {QStringLiteral("sandbox"), sandboxMode(configuration.permissionProfile)}};
    if (!configuration.modelId.isEmpty())
        params.insert(QStringLiteral("model"), configuration.modelId);
    if (!configuration.reasoningEffort.isEmpty())
        params.insert(QStringLiteral("reasoningEffort"), configuration.reasoningEffort);
    request(QStringLiteral("thread/start"), params, std::move(handler));
}

void CodexClient::resumeThread(const QString &threadId, ResultHandler handler)
{
    request(QStringLiteral("thread/resume"), {{QStringLiteral("threadId"), threadId}},
            std::move(handler));
}

void CodexClient::sendTurn(const QString &threadId, const QString &text,
                           const QStringList &images, const QString &modelId,
                           const QString &reasoningEffort, const QString &collaborationMode,
                           PermissionProfile permissionProfile,
                           ResultHandler handler)
{
    QJsonArray input;
    if (!text.isEmpty())
        input.append(QJsonObject{{QStringLiteral("type"), QStringLiteral("text")},
                                 {QStringLiteral("text"), text}});
    for (const auto &image : images)
        input.append(QJsonObject{{QStringLiteral("type"), QStringLiteral("localImage")},
                                 {QStringLiteral("path"), image}});
    QJsonObject params{{QStringLiteral("threadId"), threadId},
                       {QStringLiteral("input"), input},
                       {QStringLiteral("approvalPolicy"), approvalPolicy(permissionProfile)},
                       {QStringLiteral("sandboxPolicy"), sandboxPolicy(permissionProfile)}};
    if (!modelId.isEmpty()) {
        QJsonObject settings{{QStringLiteral("model"), modelId},
                             {QStringLiteral("reasoning_effort"),
                              reasoningEffort.isEmpty() ? QJsonValue(QJsonValue::Null)
                                                        : QJsonValue(reasoningEffort)},
                             {QStringLiteral("developer_instructions"), QJsonValue::Null}};
        params.insert(QStringLiteral("collaborationMode"),
                      QJsonObject{
                          {QStringLiteral("mode"), collaborationMode == QStringLiteral("plan")
                               ? QStringLiteral("plan") : QStringLiteral("default")},
                          {QStringLiteral("settings"), settings}});
    }
    request(QStringLiteral("turn/start"), params, std::move(handler));
}

void CodexClient::steerTurn(const QString &threadId, const QString &turnId,
                            const QString &text,
                            const QStringList &images, ResultHandler handler)
{
    QJsonArray input;
    if (!text.isEmpty())
        input.append(QJsonObject{{QStringLiteral("type"), QStringLiteral("text")},
                                 {QStringLiteral("text"), text}});
    for (const auto &image : images)
        input.append(QJsonObject{{QStringLiteral("type"), QStringLiteral("localImage")},
                                 {QStringLiteral("path"), image}});
    request(QStringLiteral("turn/steer"),
            {{QStringLiteral("threadId"), threadId},
             {QStringLiteral("expectedTurnId"), turnId},
             {QStringLiteral("input"), input}},
            std::move(handler));
}

void CodexClient::interruptTurn(const QString &threadId, const QString &turnId,
                                ResultHandler handler)
{
    request(QStringLiteral("turn/interrupt"),
            {{QStringLiteral("threadId"), threadId},
             {QStringLiteral("turnId"), turnId}},
            std::move(handler));
}

void CodexClient::respondToUserInput(const QString &itemId, const QVariantMap &answers,
                                     ResultHandler handler)
{
    const auto id = m_pendingUserInputRequests.take(itemId);
    if (id.isUndefined()) {
        if (handler)
            handler({}, QStringLiteral("The user-input request is no longer active"));
        return;
    }
    writeResponse(id, {{QStringLiteral("answers"), QJsonObject::fromVariantMap(answers)}});
    if (handler)
        handler({}, {});
}

void CodexClient::setThreadName(const QString &threadId, const QString &name,
                                ResultHandler handler)
{
    request(QStringLiteral("thread/name/set"),
            {{QStringLiteral("threadId"), threadId},
             {QStringLiteral("name"), name}},
            std::move(handler));
}

bool CodexClient::ready() const { return m_ready; }
bool CodexClient::setupRequired() const { return m_setupRequired; }
QString CodexClient::setupInstructions() const { return m_setupInstructions; }
QString CodexClient::version() const { return m_version; }

void CodexClient::setReady(bool ready)
{
    if (m_ready == ready)
        return;
    m_ready = ready;
    emit readyChanged(ready);
}

void CodexClient::setSetupRequired(bool required, const QString &instructions)
{
    if (m_setupRequired == required && m_setupInstructions == instructions)
        return;
    m_setupRequired = required;
    m_setupInstructions = instructions;
    emit setupChanged();
}

} // namespace Artemis
