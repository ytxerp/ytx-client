#include "salereferencemodel.h"

#include <QJsonArray>

#include "global/resourcepool.h"

SaleReferenceModel::SaleReferenceModel(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
{
}

SaleReferenceModel::~SaleReferenceModel() { ResourcePool<SaleReference>::Instance().Recycle(list_); }

QModelIndex SaleReferenceModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex SaleReferenceModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SaleReferenceModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int SaleReferenceModel::columnCount(const QModelIndex& /*parent*/) const { return info_.node_referenced_header.size(); }

QVariant SaleReferenceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.node_referenced_header.at(section);

    return QVariant();
}

void SaleReferenceModel::ResetModel(const QJsonArray& array)
{
    beginResetModel();

    ResourcePool<SaleReference>::Instance().Recycle(list_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* reference { ResourcePool<SaleReference>::Instance().Allocate() };
        reference->ReadJson(obj);

        list_.emplaceBack(reference);
    }

    endResetModel();
}
