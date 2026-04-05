#ifndef WORKSPACEMEMBERMODEL_H
#define WORKSPACEMEMBERMODEL_H

#include <QAbstractItemModel>
#include <QTimer>

#include "workspacemember.h"

class WorkspaceMemberModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit WorkspaceMemberModel(QObject* parent = nullptr);
    ~WorkspaceMemberModel() override = default;

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
        return member_list_.size();
    }
    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return 7;
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    void ResetModel(const QJsonArray& array);

private:
    void RestartTimer(const QUuid& id);

private:
    QList<WorkspaceMember*> member_list_ {};

    QHash<QUuid, QJsonObject> pending_updates_ {};
    QHash<QUuid, QTimer*> pending_timers_ {};
};

#endif // WORKSPACEMEMBERMODEL_H
