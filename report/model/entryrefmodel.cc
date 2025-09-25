#include "entryrefmodel.h"

#include "component/enumclass.h"
#include "global/resourcepool.h"

EntryRefModel::EntryRefModel(EntryHub* dbhub, CSectionInfo& info, int unit, QObject* parent)
    : QAbstractItemModel { parent }
    , dbhub_ { dbhub }
    , info_ { info }
    , unit_ { unit }
{
}

EntryRefModel::~EntryRefModel() { ResourcePool<EntryRef>::Instance().Recycle(entry_ref_list_); }

QModelIndex EntryRefModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex EntryRefModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int EntryRefModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return entry_ref_list_.size();
}

int EntryRefModel::columnCount(const QModelIndex& /*parent*/) const { return info_.entry_ref_header.size(); }

QVariant EntryRefModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* entry { entry_ref_list_.at(index.row()) };
    const EntryRefEnum kColumn { index.column() };

    switch (kColumn) {
    case EntryRefEnum::kIssuedTime:
        return entry->issued_time;
    case EntryRefEnum::kPIId:
        return entry->pi_id;
    case EntryRefEnum::kSection:
        return entry->section;
    case EntryRefEnum::kOrderId:
        return entry->order_id;
    case EntryRefEnum::kExternalItem:
        return entry->external_item.isNull() ? QVariant() : entry->external_item;
    case EntryRefEnum::kkCount:
        return entry->count == 0 ? QVariant() : entry->count;
    case EntryRefEnum::kkMeasure:
        return entry->measure == 0 ? QVariant() : entry->measure;
    case EntryRefEnum::kUnitPrice:
        return entry->unit_price == 0 ? QVariant() : entry->unit_price;
    case EntryRefEnum::kDiscountPrice:
        return entry->discount_price == 0 ? QVariant() : entry->discount_price;
    case EntryRefEnum::kDescription:
        return entry->description;
    case EntryRefEnum::kInitial:
        return entry->initial == 0 ? QVariant() : entry->initial;
    default:
        return QVariant();
    }
}

QVariant EntryRefModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.entry_ref_header.at(section);

    return QVariant();
}

void EntryRefModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.entry_ref_header.size() - 1)
        return;

    auto Compare = [column, order](const EntryRef* lhs, const EntryRef* rhs) -> bool {
        const EntryRefEnum kColumn { column };

        switch (kColumn) {
        case EntryRefEnum::kExternalItem:
            return (order == Qt::AscendingOrder) ? (lhs->external_item < rhs->external_item) : (lhs->external_item > rhs->external_item);
        case EntryRefEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case EntryRefEnum::kPIId:
            return (order == Qt::AscendingOrder) ? (lhs->pi_id < rhs->pi_id) : (lhs->pi_id > rhs->pi_id);
        case EntryRefEnum::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (lhs->unit_price < rhs->unit_price) : (lhs->unit_price > rhs->unit_price);
        case EntryRefEnum::kkCount:
            return (order == Qt::AscendingOrder) ? (lhs->count < rhs->count) : (lhs->count > rhs->count);
        case EntryRefEnum::kkMeasure:
            return (order == Qt::AscendingOrder) ? (lhs->measure < rhs->measure) : (lhs->measure > rhs->measure);
        case EntryRefEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case EntryRefEnum::kInitial:
            return (order == Qt::AscendingOrder) ? (lhs->initial < rhs->initial) : (lhs->initial > rhs->initial);
        case EntryRefEnum::kDiscountPrice:
            return (order == Qt::AscendingOrder) ? (lhs->discount_price < rhs->discount_price) : (lhs->discount_price > rhs->discount_price);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(entry_ref_list_.begin(), entry_ref_list_.end(), Compare);
    emit layoutChanged();
}

void EntryRefModel::RResetModel(const QUuid& node_id, const QDateTime& start, const QDateTime& end)
{
    if (node_id.isNull())
        return;

    beginResetModel();
    if (!entry_ref_list_.isEmpty())
        ResourcePool<EntryRef>::Instance().Recycle(entry_ref_list_);

    if (!node_id.isNull())
        dbhub_->ReadTransRef(entry_ref_list_, node_id, unit_, start.toUTC(), end.toUTC());

    endResetModel();
}
