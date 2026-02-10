#ifndef TAGMODEL_H
#define TAGMODEL_H

#include <QAbstractItemModel>
#include <QTimer>

#include "enum/section.h"
#include "tag.h"

class TagModel : public QAbstractItemModel {
    Q_OBJECT

signals:
    void SInsertingTag(Tag* tag);

public:
    explicit TagModel(Section section, const QHash<QUuid, Tag*>& tag_hash, QObject* parent = nullptr);
    ~TagModel() override = default;

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
        return tag_list_.size();
    }
    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return 4;
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
    bool UpdateName(Tag* tag, const QString& new_name);
    bool UpdateColor(Tag* tag, const QString& new_color);

    void RestartTimer(const QUuid& id);
    void TryInsert(Tag* tag);

private:
    const Section section_ {};

    QSet<QString> names_ {};
    QList<Tag*> tag_list_ {}; // non-owning
    QHash<QUuid, Tag*> tag_hash_ {}; // non-owning

    QSet<QUuid> pending_updates_ {};
    QHash<QUuid, QTimer*> pending_timers_ {};
};

#endif // TAGMODEL_H
