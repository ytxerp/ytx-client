#include "orderreferencemodel.h"

#include <QJsonArray>

#include "global/resourcepool.h"

OrderReferenceModel::OrderReferenceModel(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
{
}

OrderReferenceModel::~OrderReferenceModel() { ResourcePool<OrderReference>::Instance().Recycle(list_); }

QModelIndex OrderReferenceModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex OrderReferenceModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int OrderReferenceModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int OrderReferenceModel::columnCount(const QModelIndex& /*parent*/) const { return info_.node_referenced_header.size(); }

QVariant OrderReferenceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.node_referenced_header.at(section);

    return QVariant();
}

void OrderReferenceModel::Rebuild(const QJsonArray& array)
{
    if (array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty array";
    }

    QList<OrderReference*> new_list {};
    new_list.reserve(array.size());

    for (const auto& value : array) {
        if (!value.isObject()) {
            qWarning() << Q_FUNC_INFO << "Invalid data, expected object:" << value;
            continue;
        }

        auto* reference { ResourcePool<OrderReference>::Instance().Allocate() };
        reference->ReadJson(value.toObject());

        new_list.emplaceBack(reference);
    }

    beginResetModel();

    ResourcePool<OrderReference>::Instance().Recycle(list_);
    list_ = std::move(new_list);

    endResetModel();
}
