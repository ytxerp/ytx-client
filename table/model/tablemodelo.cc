#include "tablemodelo.h"

#include <QJsonArray>

#include "component/constantstring.h"
#include "enum/entryenum.h"
#include "global/entrypool.h"
#include "global/masterdataregistry.h"
#include "global/partner_inventory_registry.h"

TableModelO::TableModelO(CTableModelArg& arg, QObject* parent)
    : TableModelSingleDeferred { arg, parent }
    , d_node_ { static_cast<NodeO*>(arg.node) }
{
}

TableModelO::~TableModelO() { EntryPool::Instance().Recycle(entry_list_, section_); }

bool TableModelO::HasZeroUnitPrice() const
{
    for (const Entry* entry : entry_list_) {
        const auto* d_entry { static_cast<const EntryO*>(entry) };

        if (qFuzzyIsNull(d_entry->unit_price))
            return true;
    }

    return false;
}

QVariant TableModelO::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const EntryEnumO column { index.column() };
    auto* d_entry { static_cast<EntryO*>(index.internalPointer()) };

    if (role == Qt::ToolTipRole && column == EntryEnumO::kIssuedTime)
        return d_entry->issued_time.toString(datetime_format::kDateTime);

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    switch (column) {
    case EntryEnumO::kLhsNode:
        return d_entry->lhs_node;
    case EntryEnumO::kRhsNode:
        return d_entry->rhs_node;
    case EntryEnumO::kUnitPrice:
        return d_entry->unit_price;
    case EntryEnumO::kMeasure:
        return d_entry->measure;
    case EntryEnumO::kDescription:
        return d_entry->description;
    case EntryEnumO::kCount:
        return d_entry->count;
    case EntryEnumO::kInitial:
        return d_entry->initial;
    case EntryEnumO::kExternalSku:
        return PartnerInventoryRegistry::Instance().ExternalSku(d_node_->partner_id, d_entry->rhs_node);
    case EntryEnumO::kTag:
        return d_entry->tag;
    case EntryEnumO::kStatus:
        return d_entry->status;
    case EntryEnumO::kDocument:
        return d_entry->document;
    case EntryEnumO::kIssuedTime:
        return d_entry->issued_time;
    }
}

bool TableModelO::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    const EntryEnumO column { index.column() };

    auto* entry { static_cast<Entry*>(index.internalPointer()) };

    if (column == EntryEnumO::kStatus) {
        entry->status = value.toInt();

        emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
        return true;
    }

    if (d_node_->order_status == OrderStatus::kReleased || d_node_->sync_state == SyncState::kUpdating)
        return false;

    auto* d_entry { static_cast<EntryO*>(entry) };
    const double old_count { d_entry->count };
    const double old_measure { d_entry->measure };
    const double old_initial { d_entry->initial };
    const int row { index.row() };

    switch (column) {
    case EntryEnumO::kDescription:
        UpdateDescription(d_entry, value.toString());
        break;
    case EntryEnumO::kRhsNode:
        UpdateInternalSku(entry, value.toUuid(), row);
        break;
    case EntryEnumO::kUnitPrice:
        UpdateUnitPrice(d_entry, value.toDouble());
        break;
    case EntryEnumO::kMeasure:
        UpdateMeasure(d_entry, value.toDouble());
        break;
    case EntryEnumO::kCount:
        UpdateCount(d_entry, value.toDouble());
        break;
    case EntryEnumO::kTag:
        UpdateTag(d_entry, value.toStringList());
        break;
    case EntryEnumO::kDocument:
        UpdateDocument(d_entry, value.toStringList());
        break;
    case EntryEnumO::kIssuedTime:
    case EntryEnumO::kStatus:
    case EntryEnumO::kLhsNode:
    case EntryEnumO::kExternalSku:
    case EntryEnumO::kInitial:
        return false;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    emit SDataChanged();

    const double initial_delta { d_entry->initial - old_initial };
    const entry::Delta delta { .initial = initial_delta, .count = d_entry->count - old_count, .measure = d_entry->measure - old_measure };

    if (!delta.IsNull())
        emit SSyncDelta(d_entry->lhs_node, delta);

    if (!qFuzzyIsNull(initial_delta))
        EmitDataChanged(row, row, std::to_underlying(EntryEnumO::kInitial), std::to_underlying(EntryEnumO::kInitial));

    return true;
}

void TableModelO::sort(int column, Qt::SortOrder order)
{
    const EntryEnumO e_column { column };

    auto Compare = [order, e_column](Entry* lhs, Entry* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryO>(lhs) };
        auto* d_rhs { DerivedPtr<EntryO>(rhs) };

        switch (e_column) {
        case EntryEnumO::kDescription:
            return utils::CompareMember(lhs, rhs, &EntryO::description, order);
        case EntryEnumO::kRhsNode:
            return utils::CompareMember(lhs, rhs, &EntryO::rhs_node, order);
        case EntryEnumO::kUnitPrice:
            return utils::CompareMember(d_lhs, d_rhs, &EntryO::unit_price, order);
        case EntryEnumO::kCount:
            return utils::CompareMember(d_lhs, d_rhs, &EntryO::count, order);
        case EntryEnumO::kMeasure:
            return utils::CompareMember(d_lhs, d_rhs, &EntryO::measure, order);
        case EntryEnumO::kInitial:
            return utils::CompareMember(d_lhs, d_rhs, &EntryO::initial, order);
        case EntryEnumO::kTag:
            return utils::CompareMember(lhs, rhs, &EntryO::tag, order);
        case EntryEnumO::kDocument:
            return utils::CompareMember(lhs, rhs, &EntryO::document, order);
        case EntryEnumO::kStatus:
            return utils::CompareMember(lhs, rhs, &EntryO::status, order);
        case EntryEnumO::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &EntryO::issued_time, order);
        case EntryEnumO::kLhsNode:
        case EntryEnumO::kExternalSku:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(entry_list_, Compare);
    emit layoutChanged();
}

Qt::ItemFlags TableModelO::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const EntryEnumO column { index.column() };

    switch (column) {
    case EntryEnumO::kLhsNode:
    case EntryEnumO::kInitial:
    case EntryEnumO::kTag:
    case EntryEnumO::kDocument:
    case EntryEnumO::kExternalSku:
    case EntryEnumO::kStatus:
    case EntryEnumO::kIssuedTime:
        flags &= ~Qt::ItemIsEditable;
        break;
    case EntryEnumO::kRhsNode:
    case EntryEnumO::kDescription:
    case EntryEnumO::kCount:
    case EntryEnumO::kMeasure:
    case EntryEnumO::kUnitPrice:
        flags |= Qt::ItemIsEditable;
        break;
    }

    if (d_node_->order_status == OrderStatus::kReleased)
        flags &= ~Qt::ItemIsEditable;

    return flags;
}

bool TableModelO::insertRows(int row, int /*count*/, const QModelIndex& parent)
{
    Q_ASSERT(row >= 0 && row <= rowCount(parent));
    if (d_node_->order_status == OrderStatus::kReleased || d_node_->sync_state == SyncState::kUpdating)
        return false;

    auto* entry { EntryPool::Instance().Allocate(section_) };
    entry->id = QUuid::createUuidV7();
    entry->lhs_node = node_id_;
    entry->issued_time = QDateTime::currentDateTime();

    beginInsertRows(parent, row, row);
    entry_list_.insert(row, entry);
    endInsertRows();

    emit SDataChanged();
    return true;
}

bool TableModelO::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    Q_ASSERT(row >= 0 && row <= rowCount(parent) - 1);
    if (d_node_->order_status == OrderStatus::kReleased || d_node_->sync_state == SyncState::kUpdating)
        return false;

    auto* d_entry = DerivedPtr<EntryO>(entry_list_.at(row));

    const auto lhs_node { d_entry->lhs_node };
    const auto rhs_node { d_entry->rhs_node };
    const auto entry_id { d_entry->id };

    beginRemoveRows(parent, row, row);
    entry_list_.removeAt(row);
    endRemoveRows();

    if (!rhs_node.isNull()) {
        const entry::Delta delta { .initial = -d_entry->initial, .count = -d_entry->count, .measure = -d_entry->measure };

        if (!delta.IsNull())
            emit SSyncDelta(lhs_node, delta);
    }

    if (d_entry->sync_state != SyncState::kCreating)
        pending_delete_.insert(entry_id);

    EntryPool::Instance().Recycle(d_entry, section_);
    emit SDataChanged();
    return true;
}

/// @brief Update entry by internal product ID (rhs_node)
/// @note Responsibility: Handle insertion and update, not deletion
/// @note rhs_node must be valid, external_sku is auto-filled
bool TableModelO::UpdateInternalSku(Entry* entry, QUuid value, int row)
{
    if (value.isNull())
        return false;

    auto* d_entry { static_cast<EntryO*>(entry) };

    auto old_rhs_node { d_entry->rhs_node };
    if (old_rhs_node == value)
        return false;

    const double old_unit_price { d_entry->unit_price };

    d_entry->rhs_node = value;

    const auto unit_price { PartnerInventoryRegistry::Instance().UnitPrice(d_node_->partner_id, value) };
    d_entry->unit_price = unit_price.value_or(MasterDataRegistry::Instance().InventoryUnitPrice(value));

    const bool price_changed { !qFuzzyIsNull(old_unit_price - d_entry->unit_price) };

    if (price_changed) {
        d_entry->initial = d_entry->measure * d_entry->unit_price;
    }

    if (d_entry->sync_state == SyncState::kSynced)
        d_entry->sync_state = SyncState::kUpdating;

    const auto& registry { PartnerInventoryRegistry::Instance() };

    if (registry.ExternalSku(d_node_->partner_id, old_rhs_node) != registry.ExternalSku(d_node_->partner_id, value)) {
        EmitDataChanged(row, row, std::to_underlying(EntryEnumO::kExternalSku), std::to_underlying(EntryEnumO::kExternalSku));
    }

    if (price_changed) {
        EmitDataChanged(row, row, std::to_underlying(EntryEnumO::kUnitPrice), std::to_underlying(EntryEnumO::kInitial));
    }

    return price_changed;
}

bool TableModelO::UpdateUnitPrice(EntryO* entry, double value)
{
    if (qFuzzyCompare(entry->unit_price, value))
        return false;

    entry->initial = entry->measure * value;
    entry->unit_price = value;

    if (entry->sync_state == SyncState::kSynced)
        entry->sync_state = SyncState::kUpdating;

    return true;
}

bool TableModelO::UpdateMeasure(EntryO* entry, double value)
{
    if (qFuzzyCompare(entry->measure, value))
        return false;

    entry->initial = entry->unit_price * value;
    entry->measure = value;

    if (entry->sync_state == SyncState::kSynced)
        entry->sync_state = SyncState::kUpdating;

    return true;
}

bool TableModelO::UpdateCount(EntryO* entry, double value)
{
    if (qFuzzyCompare(entry->count, value))
        return false;

    entry->count = value;

    if (entry->sync_state == SyncState::kSynced)
        entry->sync_state = SyncState::kUpdating;

    return true;
}

// Purify newly inserted entries:
// - Entries with null `rhs_node` (internal SKU not selected) are removed.
// - The entry objects are recycled via EntryPool.
void TableModelO::Purify()
{
    for (auto i = entry_list_.size() - 1; i >= 0; --i) {
        auto* entry { entry_list_[i] };

        if (!entry->rhs_node.isNull())
            continue;

        beginRemoveRows(QModelIndex(), i, i);
        EntryPool::Instance().Recycle(entry_list_.takeAt(i), section_);
        endRemoveRows();
    }
}
