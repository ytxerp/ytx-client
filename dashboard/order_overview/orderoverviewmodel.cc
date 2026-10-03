#include "orderoverviewmodel.h"

#include "global/resourcepool.h"
#include "utils/templateutils.h"

namespace order_overview {

Model::Model(const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
    , header_ { header }
{
}

Model::~Model() { ResourcePool<Row>::Instance().Recycle(list_); }

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex Model::parent(const QModelIndex& index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int Model::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return list_.size();
}

int Model::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return header_.size();
}

QVariant Model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const auto* row { static_cast<Row*>(index.internalPointer()) };
    if (!row)
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    switch (static_cast<RowField>(index.column())) {
    case RowField::kIssuedTime:
        return row->issued_time;
    case RowField::kPartner:
        return row->partner;
    case RowField::kCode:
        return row->code;
    case RowField::kInventory:
        return row->inventory;
    case RowField::kDirectionRule:
        return row->direction_rule;
    case RowField::kCount:
        return row->count;
    case RowField::kMeasure:
        return row->measure;
    case RowField::kUnitPrice:
        return row->unit_price;
    case RowField::kAmount:
        return row->amount;
    case RowField::kUnit:
        return row->unit;
    case RowField::kStatus:
        return row->status;
    case RowField::kPlaceholder:
        return QVariant();
    }

    return QVariant();
}

void Model::Rebuild(const QJsonArray& array)
{
    if (array.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Received empty member array";
    }

    // Parse outside the reset block
    QList<Row*> new_list {};
    new_list.reserve(array.size());

    for (const auto& value : array) {
        Q_ASSERT(value.isObject());

        auto* row { ResourcePool<Row>::Instance().Allocate() };
        row->ReadJson(value.toObject());
        new_list.emplaceBack(row);
    }

    std::ranges::sort(new_list, [](const auto* lhs, const auto* rhs) { return utils::CompareMember(lhs, rhs, &Row::issued_time, Qt::AscendingOrder); });

    // Keep reset block as short as possible
    beginResetModel();
    ResourcePool<Row>::Instance().Recycle(list_);
    list_ = std::move(new_list);
    endResetModel();
}
}
