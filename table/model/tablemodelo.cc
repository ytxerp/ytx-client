#include "tablemodelo.h"

#include <QJsonArray>

#include "enum/entryenum.h"
#include "global/entrypool.h"
#include "utils/compareutils.h"
#include "websocket/jsongen.h"

TableModelO::TableModelO(CTableModelArg& arg, TreeModel* tree_model_inventory, EntryHub* entry_hub_partner, QObject* parent)
    : TableModel { arg, parent }
    , tree_model_i_ { static_cast<TreeModelI*>(tree_model_inventory) }
    , entry_hub_p_ { static_cast<EntryHubP*>(entry_hub_partner) }
{
}

TableModelO::~TableModelO() { EntryPool::Instance().Recycle(entry_list_, section_); }

void TableModelO::RAppendMultiEntry(const EntryList& entry_list)
{
    if (entry_list.isEmpty())
        return;

    const auto row { entry_list_.size() };

    beginInsertRows(QModelIndex(), row, row + entry_list.size() - 1);
    entry_list_.append(entry_list);
    endInsertRows();

    sort(std::to_underlying(EntryEnumO::kRhsNode), Qt::AscendingOrder);
}

void TableModelO::Finalize(QJsonObject& message)
{
    {
        // - Remove entries from entry_list_ that have no linked rhs_node (i.e., internal SKU not selected).
        // - Mark them as deleted in deleted_entries_ and recycle the entry.
        PurifyEntry();

        // Normalize diff buffers (inserted / deleted)
        // to ensure no conflict states remain before packaging.
        // e.g. inserted+deleted ⇒ remove both
        // NOTE: update caches are implicitly consistent: newly inserted never have update update.
        NormalizeEntryBuffer();
    }

    // deleted
    {
        if (!pending_delete_.isEmpty()) {
            QJsonArray deleted_entry_array {};
            for (const auto& id : std::as_const(pending_delete_)) {
                deleted_entry_array.append(id.toString(QUuid::WithoutBraces));
            }
            message.insert(kDeletedEntryArray, deleted_entry_array);
        }
    }

    // insert
    {
        if (!pending_insert_.isEmpty()) {
            QJsonArray inserted_entry_array {};
            for (auto it = pending_insert_.cbegin(); it != pending_insert_.cend(); ++it) {
                const QJsonObject obj { it.value()->WriteJson() };
                inserted_entry_array.append(obj);
            }
            message.insert(kInsertedEntryArray, inserted_entry_array);
        }
    }

    // update
    {
        if (!pending_update_.isEmpty()) {
            QJsonArray updated_entry_array {};
            for (auto it = pending_update_.begin(); it != pending_update_.end(); ++it) {
                updated_entry_array.append(it.value()->WriteJson());
            }
            message.insert(kUpdatedEntryArray, updated_entry_array);
        }
    }

    // clear
    {
        pending_insert_.clear();
        pending_delete_.clear();
        pending_update_.clear();
    }
}

QVariant TableModelO::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return {};

    auto* d_entry = DerivedPtr<EntryO>(entry_list_.at(index.row()));
    const EntryEnumO column { index.column() };

    switch (column) {
    case EntryEnumO::kId:
        return d_entry->id;
    case EntryEnumO::kUserId:
        return d_entry->user_id;
    case EntryEnumO::kCreateTime:
        return d_entry->created_time;
    case EntryEnumO::kCreateBy:
        return d_entry->created_by;
    case EntryEnumO::kUpdateTime:
        return d_entry->updated_time;
    case EntryEnumO::kUpdateBy:
        return d_entry->updated_by;
    case EntryEnumO::kVersion:
        return d_entry->version;
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
    case EntryEnumO::kFinal:
        return d_entry->final;
    case EntryEnumO::kDiscount:
        return d_entry->discount;
    case EntryEnumO::kInitial:
        return d_entry->initial;
    case EntryEnumO::kUnitDiscount:
        return d_entry->unit_discount;
    case EntryEnumO::kExternalSku:
        return entry_hub_p_->ExternalSku(d_node_->partner_id, d_entry->rhs_node);
    default:
        return {};
    }
}

bool TableModelO::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (d_node_->status == NodeStatus::kFinished)
        return false;

    const EntryEnumO column { index.column() };

    auto* entry = entry_list_.at(index.row());
    auto* d_entry = DerivedPtr<EntryO>(entry);

    const double old_count { d_entry->count };
    const double old_measure { d_entry->measure };
    const double old_discount { d_entry->discount };
    const double old_initial { d_entry->initial };
    const double old_final { d_entry->final };

    const auto entry_id { d_entry->id };
    const bool is_persisted { !pending_insert_.contains(entry_id) };

    bool count_changed { false };
    bool measure_changed { false };
    bool unit_price_changed { false };
    bool unit_discount_changed { false };

    switch (column) {
    case EntryEnumO::kDescription:
        UpdateDescription(d_entry, value.toString(), is_persisted);
        break;
    case EntryEnumO::kRhsNode:
        unit_price_changed = UpdateInternalSku(d_entry, value.toUuid(), is_persisted);
        break;
    case EntryEnumO::kUnitPrice:
        unit_price_changed = UpdateUnitPrice(d_entry, value.toDouble(), is_persisted);
        break;
    case EntryEnumO::kMeasure:
        measure_changed = UpdateMeasure(d_entry, value.toDouble(), is_persisted);
        break;
    case EntryEnumO::kCount:
        count_changed = UpdateCount(d_entry, value.toDouble(), is_persisted);
        break;
    case EntryEnumO::kUnitDiscount:
        unit_discount_changed = UpdateUnitDiscount(d_entry, value.toDouble(), is_persisted);
        break;
    case EntryEnumO::kId:
    case EntryEnumO::kUpdateBy:
    case EntryEnumO::kUpdateTime:
    case EntryEnumO::kCreateTime:
    case EntryEnumO::kCreateBy:
    case EntryEnumO::kVersion:
    case EntryEnumO::kUserId:
    case EntryEnumO::kLhsNode:
    case EntryEnumO::kExternalSku:
    case EntryEnumO::kInitial:
    case EntryEnumO::kDiscount:
    case EntryEnumO::kFinal:
        return false;
    }

    emit SResizeColumnToContents(index.column());

    if (count_changed)
        emit SSyncDeltaO(d_entry->lhs_node, 0.0, 0.0, d_entry->count - old_count, 0.0, 0.0);

    if (measure_changed) {
        const double measure_delta { d_entry->measure - old_measure };
        const double initial_delta { d_entry->initial - old_initial };
        const double discount_delta { d_entry->discount - old_discount };
        const double final_delta { d_entry->final - old_final };

        if (FloatChanged(measure_delta, 0.0) || FloatChanged(initial_delta, 0.0) || FloatChanged(discount_delta, 0.0) || FloatChanged(final_delta, 0.0)) {
            emit SSyncDeltaO(d_entry->lhs_node, initial_delta, final_delta, 0.0, measure_delta, discount_delta);
        }
    }

    if (unit_price_changed) {
        const double initial_delta { d_entry->initial - old_initial };
        const double final_delta { d_entry->final - old_final };

        if (FloatChanged(initial_delta, 0.0) || FloatChanged(final_delta, 0.0))
            emit SSyncDeltaO(d_entry->lhs_node, initial_delta, final_delta, 0.0, 0.0, 0.0);
    }

    if (unit_discount_changed) {
        const double discount_delta { d_entry->discount - old_discount };
        const double final_delta { d_entry->final - old_final };

        if (FloatChanged(discount_delta, 0.0) || FloatChanged(final_delta, 0.0))
            emit SSyncDeltaO(d_entry->lhs_node, 0.0, final_delta, 0.0, 0.0, discount_delta);
    }

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
            return Utils::CompareMember(lhs, rhs, &Entry::description, order);
        case EntryEnumO::kRhsNode:
            return Utils::CompareMember(lhs, rhs, &EntryO::rhs_node, order);
        case EntryEnumO::kUnitPrice:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryO::unit_price, order);
        case EntryEnumO::kCount:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryO::count, order);
        case EntryEnumO::kMeasure:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryO::measure, order);
        case EntryEnumO::kFinal:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryO::final, order);
        case EntryEnumO::kInitial:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryO::initial, order);
        case EntryEnumO::kUnitDiscount:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryO::unit_discount, order);
        case EntryEnumO::kDiscount:
            return Utils::CompareMember(d_lhs, d_rhs, &EntryO::discount, order);
        case EntryEnumO::kId:
        case EntryEnumO::kUpdateBy:
        case EntryEnumO::kUpdateTime:
        case EntryEnumO::kCreateTime:
        case EntryEnumO::kCreateBy:
        case EntryEnumO::kVersion:
        case EntryEnumO::kUserId:
        case EntryEnumO::kLhsNode:
        case EntryEnumO::kExternalSku:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(entry_list_.begin(), entry_list_.end(), Compare);
    emit layoutChanged();
}

Qt::ItemFlags TableModelO::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const EntryEnumO column { index.column() };

    switch (column) {
    case EntryEnumO::kId:
    case EntryEnumO::kInitial:
    case EntryEnumO::kDiscount:
    case EntryEnumO::kFinal:
    case EntryEnumO::kExternalSku:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    if (d_node_->status == NodeStatus::kFinished)
        flags &= ~Qt::ItemIsEditable;

    return flags;
}

QModelIndex TableModelO::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, entry_list_.at(row));
}

bool TableModelO::insertRows(int row, int /*count*/, const QModelIndex& parent)
{
    Q_ASSERT(row >= 0 && row <= rowCount(parent));
    if (d_node_->status == NodeStatus::kFinished)
        return false;

    auto* entry { EntryPool::Instance().Allocate(section_) };
    entry->id = QUuid::createUuidV7();
    entry->lhs_node = lhs_id_;

    last_issued_ = last_issued_.isValid() ? last_issued_.addSecs(1) : QDateTime::currentDateTimeUtc();
    entry->issued_time = last_issued_;

    beginInsertRows(parent, row, row);
    entry_list_.emplaceBack(entry);
    endInsertRows();

    pending_insert_.insert(entry->id, entry);

    return true;
}

bool TableModelO::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    Q_ASSERT(row >= 0 && row <= rowCount(parent) - 1);
    if (d_node_->status == NodeStatus::kFinished)
        return false;

    auto* d_entry = DerivedPtr<EntryO>(entry_list_.at(row));

    const auto lhs_node { d_entry->lhs_node };
    const auto rhs_node { d_entry->rhs_node };
    const auto entry_id { d_entry->id };

    beginRemoveRows(parent, row, row);
    entry_list_.removeAt(row);
    endRemoveRows();

    if (!rhs_node.isNull()) {
        const double count_delta { -d_entry->count };
        const double measure_delta { -d_entry->measure };
        const double discount_delta { -d_entry->discount };
        const double initial_delta { -d_entry->initial };
        const double final_delta { -d_entry->final };

        if (FloatChanged(count_delta, 0.0) || FloatChanged(measure_delta, 0.0) || FloatChanged(discount_delta, 0.0) || FloatChanged(initial_delta, 0.0)
            || FloatChanged(final_delta, 0.0)) {
            emit SSyncDeltaO(lhs_node, initial_delta, final_delta, count_delta, measure_delta, discount_delta);
        }
    }

    pending_delete_.insert(entry_id);
    pending_update_.remove(entry_id);

    EntryPool::Instance().Recycle(d_entry, section_);
    return true;
}

#if 0
/// @brief Update entry by customer product code (external_sku)
/// @note Responsibility: Handle insertion and clearing, not deletion
/// @note If mapping not found, rhs_node is cleared but entry is kept for later correction
bool TableModelO::UpdateExternalSku(EntryO* entry, const QUuid& value)
{
    if (entry->external_sku == value)
        return false;

    const double old_unit_price { entry->unit_price };
    const QUuid old_rhs_node { entry->rhs_node };

    entry->external_sku = value;
    ResolveFromExternal(entry, value);

    const QUuid new_rhs_node { entry->rhs_node };
    const auto entry_id { entry->id };

    const bool price_changed { FloatChanged(old_unit_price, entry->unit_price) };
    const bool rhs_changed { old_rhs_node != new_rhs_node };

    if (price_changed) {
        RecalculateAmount(entry);
    }

    if (!pending_inserts_.contains(entry_id)) {
        auto& update { pending_updates_[entry_id] };
        update.insert(kExternalSku, value.toString(QUuid::WithoutBraces));

        if (price_changed) {
            update.insert(kUnitPrice, QString::number(entry->unit_price, 'f', kMaxNumericScale_4));
            update.insert(kInitial, QString::number(entry->initial, 'f', kMaxNumericScale_4));
            update.insert(kFinal, QString::number(entry->final, 'f', kMaxNumericScale_4));
        }

        if (rhs_changed) {
            update.insert(kRhsNode, new_rhs_node.toString(QUuid::WithoutBraces));
        }

        ScheduleUpdate(entry_id);
    }

    if (rhs_changed) {
        emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kRhsNode));
    }

    if (price_changed) {
        emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kUnitPrice));
        emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kInitial));
        emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));
    }

    return price_changed;
}
#endif

/// @brief Update entry by internal product ID (rhs_node)
/// @note Responsibility: Handle insertion and update, not deletion
/// @note rhs_node must be valid, external_sku is auto-filled
bool TableModelO::UpdateInternalSku(EntryO* entry, const QUuid& value, bool is_persisted)
{
    if (value.isNull())
        return false;

    auto old_rhs_node { entry->rhs_node };
    if (old_rhs_node == value)
        return false;

    const double old_unit_price { entry->unit_price };

    entry->rhs_node = value;

    ResolveFromInternal(entry, value);

    const bool price_changed { FloatChanged(old_unit_price, entry->unit_price) };

    if (price_changed)
        RecalculateAmount(entry);

    if (is_persisted) {
        pending_update_.insert(entry->id, entry);
    }

    if (entry_hub_p_->ExternalSku(d_node_->partner_id, old_rhs_node) != entry_hub_p_->ExternalSku(d_node_->partner_id, value)) {
        emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kExternalSku));
    }

    if (price_changed) {
        emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kUnitPrice));
        emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kInitial));
        emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));
    }

    return price_changed;
}

bool TableModelO::UpdateUnitPrice(EntryO* entry, double value, bool is_persisted)
{
    if (FloatEqual(entry->unit_price, value))
        return false;

    entry->initial = entry->measure * value;
    entry->final = entry->initial - entry->discount;
    entry->unit_price = value;

    if (is_persisted) {
        pending_update_.insert(entry->id, entry);
    }

    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kInitial));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));

    return true;
}

bool TableModelO::UpdateUnitDiscount(EntryO* entry, double value, bool is_persisted)
{
    if (FloatEqual(entry->unit_discount, value))
        return false;

    entry->discount = entry->measure * value;
    entry->final = entry->initial - entry->discount;
    entry->unit_discount = value;

    if (is_persisted) {
        pending_update_.insert(entry->id, entry);
    }

    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kDiscount));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));
    return true;
}

bool TableModelO::UpdateMeasure(EntryO* entry, double value, bool is_persisted)
{
    if (FloatEqual(entry->measure, value))
        return false;

    entry->initial = entry->unit_price * value;
    entry->discount = entry->unit_discount * value;
    entry->final = (entry->unit_price - entry->unit_discount) * value;

    entry->measure = value;

    if (is_persisted) {
        pending_update_.insert(entry->id, entry);
    }

    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kInitial));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kDiscount));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));
    return true;
}

bool TableModelO::UpdateCount(EntryO* entry, double value, bool is_persisted)
{
    if (FloatEqual(entry->count, value))
        return false;

    entry->count = value;

    if (is_persisted) {
        pending_update_.insert(entry->id, entry);
    }

    return true;
}

bool TableModelO::UpdateDescription(EntryO* entry, const QString& value, bool is_persisted)
{
    if (entry->description == value)
        return false;

    entry->description = value;

    if (is_persisted) {
        pending_update_.insert(entry->id, entry);
    }

    return true;
}

void TableModelO::ResolveFromInternal(EntryO* entry, const QUuid& internal_sku) const
{
    if (!entry || !entry_hub_p_ || internal_sku.isNull())
        return;

    entry->unit_price = entry_hub_p_->UnitPrice(d_node_->partner_id, internal_sku).value_or(tree_model_i_->UnitPrice(internal_sku));
}

void TableModelO::RecalculateAmount(EntryO* entry) const
{
    if (!entry)
        return;

    const double measure { entry->measure };
    const double unit_price { entry->unit_price };
    const double unit_discount { entry->unit_discount };

    const double discount { measure * unit_discount };
    const double gross_amount { measure * unit_price };
    const double net_amount { gross_amount - discount };

    entry->initial = gross_amount;
    entry->final = net_amount;
    entry->discount = discount;
}

// Purify newly inserted entries:
// - Entries with null `rhs_node` (internal SKU not selected) are removed.
// - The entry objects are recycled via EntryPool.
void TableModelO::PurifyEntry()
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

// Entries that were inserted and then deleted.
// → These should be removed from both inserted_entries_ and deleted_entries_.
void TableModelO::NormalizeEntryBuffer()
{
    QSet<QUuid> to_remove_from_deleted {};

    for (const QUuid& id : std::as_const(pending_delete_)) {
        auto it = pending_insert_.find(id);
        if (it != pending_insert_.end()) {
            pending_insert_.erase(it);
            to_remove_from_deleted.insert(id);
        }
    }

    // Efficiently remove all matching ids from the deleted set.
    pending_delete_.subtract(to_remove_from_deleted);
}
