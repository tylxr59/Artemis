#include "models/ProjectTreeModel.h"

namespace Artemis {

ProjectTreeModel::ProjectTreeModel(QObject *parent) : QAbstractListModel(parent) {}

int ProjectTreeModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_rows.size();
}

QVariant ProjectTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
        return {};
    const auto &row = m_rows.at(index.row());
    switch (role) {
    case IdRole: return row.id;
    case NameRole: return row.name;
    case PathRole: return row.path;
    case GitRole: return row.git;
    case ThreadCountRole: return row.threadCount;
    default: return {};
    }
}

QHash<int, QByteArray> ProjectTreeModel::roleNames() const
{
    return {{IdRole, "projectId"}, {NameRole, "name"}, {PathRole, "path"},
            {GitRole, "isGit"}, {ThreadCountRole, "threadCount"}};
}

void ProjectTreeModel::setRows(QVector<ProjectRow> rows)
{
    beginResetModel();
    m_rows = std::move(rows);
    endResetModel();
}

ProjectRow ProjectTreeModel::row(int index) const
{
    return index >= 0 && index < m_rows.size() ? m_rows.at(index) : ProjectRow{};
}

int ProjectTreeModel::indexOfId(qint64 id) const
{
    for (int i = 0; i < m_rows.size(); ++i)
        if (m_rows.at(i).id == id)
            return i;
    return -1;
}

} // namespace Artemis
