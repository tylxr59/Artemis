#include "providers/codex/CodexClient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>

namespace Artemis {

CodexClient::CodexClient(QObject *parent)
    : AgentProvider(parent)
{
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
    connect(probe, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, probe](int code, QProcess::ExitStatus) {
        const auto output = QString::fromUtf8(probe->readAllStandardOutput()).trimmed();
        probe->deleteLater();
        if (code != 0) {
            emit providerError(QStringLiteral("Codex CLI was not found. Install it and run codex login."));
            return;
        }
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
    probe->start(QStringLiteral("codex"), {QStringLiteral("--version")});
}

void CodexClient::startProcess()
{
    if (m_process.state() != QProcess::NotRunning)
        return;
    m_stopping = false;
    m_process.setProgram(QStringLiteral("codex"));
    m_process.setArguments({QStringLiteral("app-server"), QStringLiteral("--listen"), QStringLiteral("stdio://")});
    m_process.start();
    if (!m_process.waitForStarted(3000)) {
        scheduleRestart(m_process.errorString());
        return;
    }
    request(QStringLiteral("initialize"),
            {{QStringLiteral("clientInfo"),
              QJsonObject{{QStringLiteral("name"), QStringLiteral("artemis")},
                          {QStringLiteral("title"), QStringLiteral("Artemis")},
                          {QStringLiteral("version"), QStringLiteral(ARTEMIS_VERSION)}}}},
            [this](const QJsonObject &, const QString &error) {
        if (!error.isEmpty()) {
            scheduleRestart(error);
            return;
        }
        QJsonObject initialized{{QStringLiteral("method"), QStringLiteral("initialized")},
                                {QStringLiteral("params"), QJsonObject{}}};
        m_process.write(QJsonDocument(initialized).toJson(QJsonDocument::Compact) + '\n');
        m_restartAttempt = 0;
        setReady(true);
    });
}

void CodexClient::scheduleRestart(const QString &reason)
{
    setReady(false);
    for (auto it = m_pending.begin(); it != m_pending.end(); ++it) {
        if (it->timeout)
            it->timeout->deleteLater();
        if (it->handler)
            it->handler({}, QStringLiteral("Codex disconnected: %1").arg(reason));
    }
    m_pending.clear();
    emit providerError(reason);
    if (m_stopping)
        return;
    const int delay = qMin(16000, 500 * (1 << qMin(m_restartAttempt++, 5)));
    QTimer::singleShot(delay, this, &CodexClient::startProcess);
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

void CodexClient::handleNotification(const QString &method, const QJsonObject &params)
{
    emit rawNotification(method, params);
    if (method == QStringLiteral("item/started") || method == QStringLiteral("item/completed")) {
        normalizeItem(method.endsWith(QStringLiteral("started")) ? QStringLiteral("started")
                                                                 : QStringLiteral("completed"),
                      params);
    } else if (method == QStringLiteral("item/agentMessage/delta")) {
        emit domainEvent(params.value(QStringLiteral("threadId")).toString(),
                         QStringLiteral("assistant"), QStringLiteral("Artemis"),
                         params.value(QStringLiteral("delta")).toString(), {});
    } else if (method == QStringLiteral("turn/diff/updated")) {
        emit domainEvent(params.value(QStringLiteral("threadId")).toString(),
                         QStringLiteral("diff"), QStringLiteral("Changes"),
                         params.value(QStringLiteral("diff")).toString(), {});
    } else if (method == QStringLiteral("turn/plan/updated")) {
        emit domainEvent(params.value(QStringLiteral("threadId")).toString(),
                         QStringLiteral("plan"), QStringLiteral("Plan"),
                         QString::fromUtf8(QJsonDocument(params).toJson(QJsonDocument::Indented)), {});
    } else if (method == QStringLiteral("turn/completed")) {
        const auto turn = params.value(QStringLiteral("turn")).toObject();
        emit domainEvent(params.value(QStringLiteral("threadId")).toString(),
                         QStringLiteral("status"), QStringLiteral("Turn completed"),
                         turn.value(QStringLiteral("status")).toString(), {});
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
    QString domainType = QStringLiteral("activity");
    QString title = type;
    if (type == QStringLiteral("commandExecution")) {
        domainType = QStringLiteral("command");
        title = lifecycle == QStringLiteral("started") ? QStringLiteral("Running command")
                                                        : QStringLiteral("Command completed");
    } else if (type == QStringLiteral("fileChange")) {
        domainType = QStringLiteral("file");
        title = QStringLiteral("File changes");
    } else if (type == QStringLiteral("agentMessage")) {
        domainType = QStringLiteral("assistant");
        title = QStringLiteral("Artemis");
    } else if (type == QStringLiteral("userMessage")) {
        domainType = QStringLiteral("user");
        title = QStringLiteral("You");
    }
    emit domainEvent(params.value(QStringLiteral("threadId")).toString(), domainType,
                     title, itemContent(item), {{QStringLiteral("lifecycle"), lifecycle}});
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
    return QString::fromUtf8(QJsonDocument(item).toJson(QJsonDocument::Indented));
}

void CodexClient::listModels(ResultHandler handler)
{
    request(QStringLiteral("model/list"), {{QStringLiteral("limit"), 100}}, std::move(handler));
}

void CodexClient::listThreads(const QString &workspacePath, ResultHandler handler)
{
    request(QStringLiteral("thread/list"),
            {{QStringLiteral("cwd"), workspacePath},
             {QStringLiteral("limit"), 100},
             {QStringLiteral("sortKey"), QStringLiteral("updated_at")},
             {QStringLiteral("sortDirection"), QStringLiteral("desc")}},
            std::move(handler));
}

void CodexClient::startThread(const ThreadConfiguration &configuration, ResultHandler handler)
{
    QJsonObject params{{QStringLiteral("cwd"), configuration.workspacePath},
                       {QStringLiteral("ephemeral"), configuration.ephemeral},
                       {QStringLiteral("approvalPolicy"), QStringLiteral("never")},
                       {QStringLiteral("sandbox"), QStringLiteral("danger-full-access")}};
    if (!configuration.modelId.isEmpty())
        params.insert(QStringLiteral("model"), configuration.modelId);
    request(QStringLiteral("thread/start"), params, std::move(handler));
}

void CodexClient::resumeThread(const QString &threadId, ResultHandler handler)
{
    request(QStringLiteral("thread/resume"), {{QStringLiteral("threadId"), threadId}},
            std::move(handler));
}

void CodexClient::sendTurn(const QString &threadId, const QString &text,
                           const QStringList &images, ResultHandler handler)
{
    QJsonArray input;
    input.append(QJsonObject{{QStringLiteral("type"), QStringLiteral("text")},
                             {QStringLiteral("text"), text}});
    for (const auto &image : images)
        input.append(QJsonObject{{QStringLiteral("type"), QStringLiteral("localImage")},
                                 {QStringLiteral("path"), image}});
    request(QStringLiteral("turn/start"),
            {{QStringLiteral("threadId"), threadId}, {QStringLiteral("input"), input}},
            std::move(handler));
}

void CodexClient::steerTurn(const QString &threadId, const QString &text, ResultHandler handler)
{
    const QJsonArray input{QJsonObject{{QStringLiteral("type"), QStringLiteral("text")},
                                       {QStringLiteral("text"), text}}};
    request(QStringLiteral("turn/steer"),
            {{QStringLiteral("threadId"), threadId}, {QStringLiteral("input"), input}},
            std::move(handler));
}

void CodexClient::interruptTurn(const QString &threadId, ResultHandler handler)
{
    request(QStringLiteral("turn/interrupt"), {{QStringLiteral("threadId"), threadId}},
            std::move(handler));
}

bool CodexClient::ready() const { return m_ready; }
QString CodexClient::version() const { return m_version; }

void CodexClient::setReady(bool ready)
{
    if (m_ready == ready)
        return;
    m_ready = ready;
    emit readyChanged(ready);
}

} // namespace Artemis
