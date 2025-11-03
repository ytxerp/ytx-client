#include "leafmodelo.h"

#include "global/entryshadowpool.h"
#include "websocket/jsongen.h"

LeafModelO::LeafModelO(CLeafModelArg& arg, const Node* node, TreeModel* tree_model_inventory, EntryHub* entry_hub_partner, QObject* parent)
    : LeafModel { arg, parent }
    , tree_model_i_ { static_cast<TreeModelI*>(tree_model_inventory) }
    , entry_hub_partner_ { static_cast<EntryHubP*>(entry_hub_partner) }
    , entry_hub_order_ { static_cast<EntryHubO*>(arg.entry_hub) }
    , d_node_ { static_cast<const NodeO*>(node) }
{
}

void LeafModelO::RSaveOrder()
{
    if (shadow_list_.isEmpty())
        return;

    // Normalize buffer states before saving ---
    // This ensures that insert/delete/update buffers remain consistent
    // and no redundant or conflicting entries exist.

    // Case ①: Entries that were inserted and then deleted.
    // → These should be removed from both inserted_entries_ and deleted_entries_.
    QSet<QUuid> to_remove_from_deleted;
    for (const QUuid& id : std::as_const(deleted_entries_)) {
        if (inserted_entries_.contains(id)) {
            inserted_entries_.remove(id);
            to_remove_from_deleted.insert(id);
        }
    }
    // Efficiently remove all matching ids from the deleted set.
    deleted_entries_.subtract(to_remove_from_deleted);

    // Case ② & ③:
    // - Deleted entries: remove any pending update caches (they're meaningless now).
    // - Newly inserted entries: remove redundant caches (fresh insertions don't need updates).
    for (const QUuid& id : std::as_const(deleted_entries_)) {
        caches_.remove(id);
    }

    for (auto it = inserted_entries_.cbegin(); it != inserted_entries_.cend(); ++it) {
        caches_.remove(it.key());
    }

    // At this point:
    // - shadow_list_ contains only valid entries ready for saving.
    // - buffer containers (inserted/deleted/caches) are consistent and conflict-free.
}

QVariant LeafModelO::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_shadow = DerivedPtr<EntryShadowO>(shadow_list_.at(index.row()));
    const EntryEnumO column { index.column() };

    switch (column) {
    case EntryEnumO::kId:
        return *d_shadow->id;
    case EntryEnumO::kLhsNode:
        return *d_shadow->lhs_node;
    case EntryEnumO::kRhsNode:
        return *d_shadow->rhs_node;
    case EntryEnumO::kUnitPrice:
        return *d_shadow->unit_price;
    case EntryEnumO::kMeasure:
        return *d_shadow->measure;
    case EntryEnumO::kDescription:
        return *d_shadow->description;
    case EntryEnumO::kCount:
        return *d_shadow->count;
    case EntryEnumO::kFinal:
        return *d_shadow->final;
    case EntryEnumO::kDiscount:
        return *d_shadow->discount;
    case EntryEnumO::kInitial:
        return *d_shadow->initial;
    case EntryEnumO::kUnitDiscount:
        return *d_shadow->unit_discount;
    case EntryEnumO::kExternalSku:
        return *d_shadow->external_sku;
    default:
        return QVariant();
    }
}

bool LeafModelO::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (d_node_->status == std::to_underlying(NodeStatus::kReleased))
        return false;

    const EntryEnumO column { index.column() };

    auto* shadow = shadow_list_.at(index.row());
    auto* d_shadow = DerivedPtr<EntryShadowO>(shadow);

    const double old_count { *d_shadow->count };
    const double old_measure { *d_shadow->measure };
    const double old_discount { *d_shadow->discount };
    const double old_initial { *d_shadow->initial };
    const double old_final { *d_shadow->final };

    bool count_changed { false };
    bool measure_changed { false };
    bool unit_price_changed { false };
    bool unit_discount_changed { false };

    auto& cache { caches_[*d_shadow->id] };

    switch (column) {
    case EntryEnumO::kDescription:
        EntryUtils::UpdateShadowField(cache, shadow, kDescription, value.toString(), &EntryShadow::description);
        break;
    case EntryEnumO::kRhsNode:
        unit_price_changed = UpdateLinkedNode(d_shadow, value.toUuid(), 0);
        break;
    case EntryEnumO::kUnitPrice:
        unit_price_changed = UpdateRate(d_shadow, value.toDouble());
        break;
    case EntryEnumO::kMeasure:
        measure_changed = UpdateMeasure(cache, d_shadow, value.toDouble());
        break;
    case EntryEnumO::kCount:
        count_changed = UpdateCount(cache, d_shadow, value.toDouble());
        break;
    case EntryEnumO::kUnitDiscount:
        unit_discount_changed = UpdateUnitDiscount(cache, d_shadow, value.toDouble());
        break;
    case EntryEnumO::kExternalSku:
        unit_price_changed = UpdateExternalSku(cache, d_shadow, value.toUuid());
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());

    if (count_changed)
        emit SSyncDelta(*d_shadow->lhs_node, 0.0, 0.0, *d_shadow->count - old_count);

    if (measure_changed) {
        const double measure_delta { *d_shadow->measure - old_measure };
        const double initial_delta { *d_shadow->initial - old_initial };
        const double discount_delta { *d_shadow->discount - old_discount };
        const double final_delta { *d_shadow->final - old_final };

        if (FloatChanged(measure_delta, 0.0) || FloatChanged(initial_delta, 0.0) || FloatChanged(discount_delta, 0.0) || FloatChanged(final_delta, 0.0)) {
            emit SSyncDelta(*d_shadow->lhs_node, initial_delta, final_delta, 0.0, measure_delta, discount_delta);
        }
    }

    if (unit_price_changed) {
        const double initial_delta { *d_shadow->initial - old_initial };
        const double final_delta { *d_shadow->final - old_final };

        if (FloatChanged(initial_delta, 0.0) || FloatChanged(final_delta, 0.0))
            emit SSyncDelta(*d_shadow->lhs_node, initial_delta, final_delta);
    }

    if (unit_discount_changed) {
        const double discount_delta { *d_shadow->discount - old_discount };
        const double final_delta { *d_shadow->final - old_final };

        if (FloatChanged(discount_delta, 0.0) || FloatChanged(final_delta, 0.0))
            emit SSyncDelta(*d_shadow->lhs_node, 0.0, final_delta, 0.0, 0.0, discount_delta);
    }

    return true;
}

void LeafModelO::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column <= info_.entry_header.size() - 1);

    const EntryEnumO e_column { column };

    switch (e_column) {
    case EntryEnumO::kId:
        return;
    default:
        break;
    }

    auto Compare = [order, e_column](EntryShadow* lhs, EntryShadow* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryShadowO>(lhs) };
        auto* d_rhs { DerivedPtr<EntryShadowO>(rhs) };

        switch (e_column) {
        case EntryEnumO::kRhsNode:
            return (order == Qt::AscendingOrder) ? (*d_lhs->rhs_node < *d_rhs->rhs_node) : (*d_lhs->rhs_node > *d_rhs->rhs_node);
        case EntryEnumO::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (*d_lhs->unit_price < *d_rhs->unit_price) : (*d_lhs->unit_price > *d_rhs->unit_price);
        case EntryEnumO::kCount:
            return (order == Qt::AscendingOrder) ? (*d_lhs->count < *d_rhs->count) : (*d_lhs->count > *d_rhs->count);
        case EntryEnumO::kMeasure:
            return (order == Qt::AscendingOrder) ? (*d_lhs->measure < *d_rhs->measure) : (*d_lhs->measure > *d_rhs->measure);
        case EntryEnumO::kFinal:
            return (order == Qt::AscendingOrder) ? (*d_lhs->final < *d_rhs->final) : (*d_lhs->final > *d_rhs->final);
        case EntryEnumO::kInitial:
            return (order == Qt::AscendingOrder) ? (*d_lhs->initial < *d_rhs->initial) : (*d_lhs->initial > *d_rhs->initial);
        case EntryEnumO::kUnitDiscount:
            return (order == Qt::AscendingOrder) ? (*d_lhs->unit_discount < *d_rhs->unit_discount) : (*d_lhs->unit_discount > *d_rhs->unit_discount);
        case EntryEnumO::kExternalSku:
            return (order == Qt::AscendingOrder) ? (*d_lhs->external_sku < *d_rhs->external_sku) : (*d_lhs->external_sku > *d_rhs->external_sku);
        case EntryEnumO::kDiscount:
            return (order == Qt::AscendingOrder) ? (*d_lhs->discount < *d_rhs->discount) : (*d_lhs->discount > *d_rhs->discount);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(shadow_list_.begin(), shadow_list_.end(), Compare);
    emit layoutChanged();
}

Qt::ItemFlags LeafModelO::flags(const QModelIndex& index) const
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
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    if (d_node_->status == std::to_underlying(NodeStatus::kReleased))
        flags &= ~Qt::ItemIsEditable;

    return flags;
}

bool LeafModelO::insertRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent));
    if (d_node_->status == std::to_underlying(NodeStatus::kReleased))
        return false;

    auto* entry_shadow { entry_hub_->AllocateEntryShadow() };

    *entry_shadow->lhs_node = lhs_id_;

    beginInsertRows(parent, row, row);
    shadow_list_.emplaceBack(entry_shadow);
    endInsertRows();

    return true;
}

bool LeafModelO::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent) - 1);
    if (d_node_->status == std::to_underlying(NodeStatus::kReleased))
        return false;

    auto* d_shadow = DerivedPtr<EntryShadowO>(shadow_list_.at(row));
    const auto lhs_node { *d_shadow->lhs_node };
    const auto rhs_node { *d_shadow->rhs_node };

    beginRemoveRows(parent, row, row);
    shadow_list_.removeAt(row);
    endRemoveRows();

    if (!rhs_node.isNull()) {
        emit SSyncDelta(lhs_node, -*d_shadow->initial, -*d_shadow->final, -*d_shadow->count, -*d_shadow->measure, -*d_shadow->discount);
        deleted_entries_.insert(*d_shadow->id);
    }

    EntryShadowPool::Instance().Recycle(d_shadow, section_);
    return true;
}

/// @brief Update entry by customer product code (external_sku)
/// @note Responsibility: Handle insertion and clearing, not deletion
/// @note If mapping not found, rhs_node is cleared but entry is kept for later correction
bool LeafModelO::UpdateExternalSku(QJsonObject& cache, EntryShadowO* entry_shadow, const QUuid& value)
{
    if (*entry_shadow->external_sku == value)
        return false;

    const double old_unit_price { *entry_shadow->unit_price };
    const QUuid old_rhs_node { *entry_shadow->rhs_node };

    *entry_shadow->external_sku = value;
    ResolveFromExternal(entry_shadow, value);

    const bool price_changed { FloatChanged(old_unit_price, *entry_shadow->unit_price) };
    const bool rhs_changed { old_rhs_node != *entry_shadow->rhs_node };

    cache.insert(kExternalSku, value.toString(QUuid::WithoutBraces));

    if (price_changed) {
        RecalculateAmount(entry_shadow);

        cache.insert(kUnitPrice, *entry_shadow->unit_price);
        cache.insert(kInitial, *entry_shadow->initial);
        cache.insert(kFinal, *entry_shadow->final);
    }

    if (rhs_changed) {
        if (old_rhs_node.isNull()) {
            inserted_entries_.insert(*entry_shadow->id, entry_shadow);
        }

        cache.insert(kRhsNode, entry_shadow->rhs_node->toString(QUuid::WithoutBraces));
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

/// @brief Update entry by internal product ID (rhs_node)
/// @note Responsibility: Handle insertion and update, not deletion
/// @note rhs_node must be valid, external_sku is auto-filled
bool LeafModelO::UpdateLinkedNode(EntryShadow* entry_shadow, const QUuid& value, int /*row*/)
{
    if (value.isNull())
        return false;

    auto* d_shadow = DerivedPtr<EntryShadowO>(entry_shadow);
    auto old_rhs_node { *d_shadow->rhs_node };
    if (old_rhs_node == value)
        return false;

    const double old_unit_price { *d_shadow->unit_price };
    const QUuid old_external_sku { *d_shadow->external_sku };

    *d_shadow->rhs_node = value;
    auto& cache { caches_[*d_shadow->id] };

    ResolveFromInternal(d_shadow, value);

    const bool price_changed { FloatChanged(old_unit_price, *d_shadow->unit_price) };
    const bool external_changed { old_external_sku != *d_shadow->external_sku };

    if (price_changed)
        RecalculateAmount(d_shadow);

    if (old_rhs_node.isNull()) {
        inserted_entries_.insert(*entry_shadow->id, entry_shadow);
    } else {
        cache.insert(kRhsNode, value.toString(QUuid::WithoutBraces));

        if (external_changed) {
            cache.insert(kExternalSku, d_shadow->external_sku->toString(QUuid::WithoutBraces));
        }

        if (price_changed) {
            cache.insert(kUnitPrice, *d_shadow->unit_price);
            cache.insert(kInitial, *d_shadow->initial);
            cache.insert(kFinal, *d_shadow->final);
        }
    }

    if (external_changed) {
        emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kExternalSku));
    }

    if (price_changed) {
        emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kUnitPrice));
        emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kInitial));
        emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));
    }

    return price_changed;
}

bool LeafModelO::UpdateRate(EntryShadow* entry_shadow, double value)
{
    auto* d_shadow = DerivedPtr<EntryShadowO>(entry_shadow);

    if (FloatEqual(*d_shadow->unit_price, value))
        return false;

    auto& cache { caches_[*d_shadow->id] };

    *d_shadow->initial = *d_shadow->measure * value;
    *d_shadow->final = *d_shadow->initial - *d_shadow->discount;
    *d_shadow->unit_price = value;

    if (!d_shadow->rhs_node->isNull()) {
        cache.insert(kUnitPrice, *d_shadow->unit_price);
        cache.insert(kInitial, *d_shadow->initial);
        cache.insert(kFinal, *d_shadow->final);
    }

    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kInitial));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));

    return true;
}

bool LeafModelO::UpdateUnitDiscount(QJsonObject& cache, EntryShadowO* entry_shadow, double value)
{
    if (FloatEqual(*entry_shadow->unit_discount, value))
        return false;

    *entry_shadow->discount = *entry_shadow->measure * value;
    *entry_shadow->final = *entry_shadow->initial - *entry_shadow->discount;
    *entry_shadow->unit_discount = value;

    if (!entry_shadow->rhs_node->isNull()) {
        cache.insert(kUnitDiscount, *entry_shadow->unit_discount);
        cache.insert(kDiscount, *entry_shadow->discount);
        cache.insert(kFinal, *entry_shadow->final);
    }

    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kDiscount));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));
    return true;
}

bool LeafModelO::UpdateMeasure(QJsonObject& cache, EntryShadowO* entry_shadow, double value)
{
    if (FloatEqual(*entry_shadow->measure, value))
        return false;

    *entry_shadow->initial = *entry_shadow->unit_price * value;
    *entry_shadow->discount = *entry_shadow->unit_discount * value;
    *entry_shadow->final = (*entry_shadow->unit_price - *entry_shadow->unit_discount) * value;

    *entry_shadow->measure = value;

    if (!entry_shadow->rhs_node->isNull()) {
        cache.insert(kMeasure, *entry_shadow->measure);
        cache.insert(kInitial, *entry_shadow->initial);
        cache.insert(kDiscount, *entry_shadow->discount);
        cache.insert(kFinal, *entry_shadow->final);
    }

    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kInitial));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kDiscount));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));
    return true;
}

bool LeafModelO::UpdateCount(QJsonObject& cache, EntryShadowO* entry_shadow, double value)
{
    if (FloatEqual(*entry_shadow->count, value))
        return false;

    *entry_shadow->count = value;

    if (!entry_shadow->rhs_node->isNull())
        cache.insert(kCount, *entry_shadow->count);

    return true;
}

void LeafModelO::ResolveFromInternal(EntryShadowO* shadow, const QUuid& internal_sku) const
{
    if (!shadow || !entry_hub_partner_ || internal_sku.isNull())
        return;

    if (auto result = entry_hub_partner_->ResolveFromInternal(d_node_->partner, internal_sku)) {
        const auto& [external_id, price] = *result;
        *shadow->unit_price = price;
        *shadow->external_sku = external_id;
    } else {
        *shadow->unit_price = tree_model_i_->UnitPrice(internal_sku);
        *shadow->external_sku = QUuid();
    }
}

void LeafModelO::ResolveFromExternal(EntryShadowO* shadow, const QUuid& external_sku) const
{
    if (!shadow || !entry_hub_partner_ || external_sku.isNull())
        return;

    if (auto result = entry_hub_partner_->ResolveFromExternal(d_node_->partner, external_sku)) {
        const auto& [rhs_node, price] = *result;
        *shadow->unit_price = price;
        *shadow->rhs_node = rhs_node;
    } else {
        *shadow->unit_price = 0.0;
        *shadow->rhs_node = QUuid();
    }
}

void LeafModelO::RecalculateAmount(EntryShadowO* shadow) const
{
    if (!shadow)
        return;

    const double measure { *shadow->measure };
    const double unit_price { *shadow->unit_price };
    const double unit_discount { *shadow->unit_discount };

    const double discount { measure * unit_discount };
    const double gross_amount { measure * unit_price };
    const double net_amount { gross_amount - discount };

    *shadow->initial = gross_amount;
    *shadow->final = net_amount;
    *shadow->discount = discount;
}
