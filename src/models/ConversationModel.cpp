#include "models/ConversationModel.h"

namespace Artemis {

ConversationModel::ConversationModel(QObject *parent) : QAbstractListModel(parent) {}

int ConversationModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_events.size();
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
    beginResetModel();
    m_threadId = threadId;
    m_events.clear();
    endResetModel();
}

void ConversationModel::append(const ConversationEvent &event)
{
    if (!m_threadId.isEmpty() && event.threadId != m_threadId)
        return;
    const int row = m_events.size();
    beginInsertRows({}, row, row);
    m_events.push_back(event);
    endInsertRows();
}

void ConversationModel::appendOrMergeDelta(const ConversationEvent &event)
{
    if (!m_events.isEmpty() && event.type == QStringLiteral("assistant")
        && m_events.last().type == event.type && m_events.last().title == event.title) {
        m_events.last().content += event.content;
        const auto idx = index(m_events.size() - 1);
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
