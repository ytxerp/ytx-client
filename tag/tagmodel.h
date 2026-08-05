#ifndef TAGMODEL_H
#define TAGMODEL_H

#include <QAbstractItemModel>
#include <QTimer>

#include "enum/section.h"
#include "tagrow.h"

namespace tag {

class Model final : public QAbstractItemModel {
    Q_OBJECT

signals:
    void SInsertLocalTag(Section section, Row* tag);

public:
    explicit Model(Section section, const QHash<QUuid, Row*>& tag_hash, const QStringList& header, QObject* parent = nullptr);
    ~Model() override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override
    {
        Q_UNUSED(index)
        return QModelIndex();
    }
    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return list_.size();
    }
    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return header_.size();
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

private:
    bool UpdateName(Row* tag, const QString& new_name);
    bool UpdateColor(Row* tag, const QString& new_color);

    void RestartTimer(const QUuid& id, Row* tag);
    void FlushCaches();

    void TryInsert(Row* tag);

private:
    const Section section_ {};
    const QStringList& header_;

    // non-owning pointers, owned by tag_hash
    QList<Row*> list_ {};

    QHash<QUuid, QJsonObject> pending_updates_ {};
    QHash<QUuid, QTimer*> pending_timers_ {};
};
}

#endif // TAGMODEL_H
