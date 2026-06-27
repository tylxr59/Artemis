#include "models/ConversationModel.h"

namespace Artemis {

ConversationModel::ConversationModel(QObject *parent) : QAbstractListModel(parent) {}

int ConversationModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_events.size());
}

QVariant ConversationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_events.size())
        return {};
    const auto &event = m_events.at(index.row());
    switch (role) {
    case TypeRole: return event.type;
    case TitleRole: return event.title;
    case ContentRole: return event.content;
    case MetadataRole: return event.metadata;
    default: return {};
    }
}

QHash<int, QByteArray> ConversationModel::roleNames() const
{
    return {{TypeRole, "eventType"}, {TitleRole, "title"},
            {ContentRole, "content"}, {MetadataRole, "metadata"}};
}

void ConversationModel::setThread(const QString &threadId)
{
    if (threadId == m_threadId)
        return;
    resetThread(threadId);
}

void ConversationModel::resetThread(const QString &threadId)
{
    beginResetModel();
    m_threadId = threadId;
    m_events.clear();
    endResetModel();
}

void ConversationModel::append(const ConversationEvent &event)
{
    if (!m_threadId.isEmpty() && event.threadId != m_threadId)
        return;
    const auto itemId = event.metadata.value(QStringLiteral("itemId")).toString();
    const auto turnId = event.metadata.value(QStringLiteral("turnId")).toString();
    const bool replacesTurnDiff = event.type == QStringLiteral("diff")
        && !turnId.isEmpty();
    if (!itemId.isEmpty() || replacesTurnDiff) {
        for (int row = static_cast<int>(m_events.size()) - 1; row >= 0; --row) {
            auto &existing = m_events[row];
            const bool sameItem = !itemId.isEmpty()
                && existing.metadata.value(QStringLiteral("itemId")).toString() == itemId;
            const bool sameTurnDiff = replacesTurnDiff
                && existing.type == QStringLiteral("diff")
                && existing.metadata.value(QStringLiteral("turnId")).toString() == turnId;
            if (!sameItem && !sameTurnDiff)
                continue;
            existing = event;
            const auto idx = index(row);
            emit dataChanged(idx, idx, {TypeRole, TitleRole, ContentRole, MetadataRole});
            return;
        }
    }
    const int row = static_cast<int>(m_events.size());
    beginInsertRows({}, row, row);
    m_events.push_back(event);
    endInsertRows();
}

void ConversationModel::appendOrMergeDelta(const ConversationEvent &event)
{
    if (!m_threadId.isEmpty() && event.threadId != m_threadId)
        return;
    const bool completed = event.metadata.value(QStringLiteral("lifecycle")).toString()
                           == QStringLiteral("completed");
    if (completed) {
        for (int row = static_cast<int>(m_events.size()) - 1; row >= 0; --row) {
            auto &existing = m_events[row];
            if (existing.type != QStringLiteral("assistant"))
                continue;
            if (existing.content == event.content)
                return;
            if (event.content.startsWith(existing.content)) {
                existing.content = event.content;
                existing.metadata = event.metadata;
                const auto idx = index(row);
                emit dataChanged(idx, idx, {ContentRole, MetadataRole});
                return;
            }
            break;
        }
    }
    if (!m_events.isEmpty() && event.type == QStringLiteral("assistant")
        && m_events.last().type == event.type && m_events.last().title == event.title) {
        m_events.last().content += event.content;
        const auto idx = index(static_cast<int>(m_events.size()) - 1);
        emit dataChanged(idx, idx, {ContentRole});
        return;
    }
    append(event);
}

void ConversationModel::clear()
{
    beginResetModel();
    m_events.clear();
    endResetModel();
}

} // namespace Artemis
