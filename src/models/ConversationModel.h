#pragma once

#include <QAbstractListModel>
#include <QVector>

namespace Artemis {

struct ConversationEvent {
    QString threadId;
    QString type;
    QString title;
    QString content;
    QVariantMap metadata;
};

class ConversationModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles { TypeRole = Qt::UserRole + 1, TitleRole, ContentRole, MetadataRole };
    explicit ConversationModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setThread(const QString &threadId);
    void append(const ConversationEvent &event);
    void appendOrMergeDelta(const ConversationEvent &event);
    void clear();

private:
    QString m_threadId;
    QVector<ConversationEvent> m_events;
};

} // namespace Artemis
