#pragma once

#include <QAbstractListModel>
#include <QVector>

namespace Artemis {

struct ProjectRow {
    qint64 id = -1;
    QString name;
    QString path;
    bool git = false;
    int threadCount = 0;
};

class ProjectTreeModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles { IdRole = Qt::UserRole + 1, NameRole, PathRole, GitRole, ThreadCountRole };

    explicit ProjectTreeModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void setRows(QVector<ProjectRow> rows);
    ProjectRow row(int index) const;
    int indexOfId(qint64 id) const;

private:
    QVector<ProjectRow> m_rows;
};

} // namespace Artemis
