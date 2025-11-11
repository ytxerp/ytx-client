#include "tablemodelo.h"

#include <QJsonArray>

#include "global/entrypool.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableModelO::TableModelO(CTableModelArg& arg, TreeModel* tree_model_inventory, EntryHub* entry_hub_partner, QObject* parent)
    : TableModel { arg, parent }
    , tree_model_i_ { static_cast<TreeModelI*>(tree_model_inventory) }
    , entry_hub_p_ { static_cast<EntryHubP*>(entry_hub_partner) }
{
}

TableModelO::~TableModelO()
{
    emit SReleaseEntry(lhs_id_);
    FlushUpdates();
}

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

void TableModelO::FinalizeInserts(QJsonObject& order_cache)
{
    PurifyEntry();

    // inserted
    QJsonArray inserted_entry_array {};
    for (auto it = pending_inserts_.cbegin(); it != pending_inserts_.cend(); ++it) {
        const QJsonObject obj { it.value()->WriteJson() };
        inserted_entry_array.append(obj);
    }
    order_cache.insert(kInsertedEntryArray, inserted_entry_array);

    // clear
    pending_inserts_.clear();
}

QVariant TableModelO::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_entry = DerivedPtr<EntryO>(entry_list_.at(index.row()));
    const EntryEnumO column { index.column() };

    switch (column) {
    case EntryEnumO::kId:
        return d_entry->id;
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
        return d_entry->external_sku;
    default:
        return QVariant();
    }
}

bool TableModelO::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (d_node_->status == std::to_underlying(NodeStatus::kReleased))
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

    bool count_changed { false };
    bool measure_changed { false };
    bool unit_price_changed { false };
    bool unit_discount_changed { false };

    switch (column) {
    case EntryEnumO::kDescription:
        UpdateDescription(d_entry, value.toString());
        break;
    case EntryEnumO::kRhsNode:
        unit_price_changed = UpdateInternalSku(d_entry, value.toUuid());
        break;
    case EntryEnumO::kUnitPrice:
        unit_price_changed = UpdateUnitPrice(d_entry, value.toDouble());
        break;
    case EntryEnumO::kMeasure:
        measure_changed = UpdateMeasure(d_entry, value.toDouble());
        break;
    case EntryEnumO::kCount:
        count_changed = UpdateCount(d_entry, value.toDouble());
        break;
    case EntryEnumO::kUnitDiscount:
        unit_discount_changed = UpdateUnitDiscount(d_entry, value.toDouble());
        break;
    case EntryEnumO::kExternalSku:
        unit_price_changed = UpdateExternalSku(d_entry, value.toUuid());
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());

    if (count_changed)
        emit SSyncDeltaO(d_entry->lhs_node, 0.0, 0.0, d_entry->count - old_count, 0.0, 0.0, !pending_inserts_.contains(entry_id));

    if (measure_changed) {
        const double measure_delta { d_entry->measure - old_measure };
        const double initial_delta { d_entry->initial - old_initial };
        const double discount_delta { d_entry->discount - old_discount };
        const double final_delta { d_entry->final - old_final };

        if (FloatChanged(measure_delta, 0.0) || FloatChanged(initial_delta, 0.0) || FloatChanged(discount_delta, 0.0) || FloatChanged(final_delta, 0.0)) {
            emit SSyncDeltaO(d_entry->lhs_node, initial_delta, final_delta, 0.0, measure_delta, discount_delta, !pending_inserts_.contains(entry_id));
        }
    }

    if (unit_price_changed) {
        const double initial_delta { d_entry->initial - old_initial };
        const double final_delta { d_entry->final - old_final };

        if (FloatChanged(initial_delta, 0.0) || FloatChanged(final_delta, 0.0))
            emit SSyncDeltaO(d_entry->lhs_node, initial_delta, final_delta, 0.0, 0.0, 0.0, !pending_inserts_.contains(entry_id));
    }

    if (unit_discount_changed) {
        const double discount_delta { d_entry->discount - old_discount };
        const double final_delta { d_entry->final - old_final };

        if (FloatChanged(discount_delta, 0.0) || FloatChanged(final_delta, 0.0))
            emit SSyncDeltaO(d_entry->lhs_node, 0.0, final_delta, 0.0, 0.0, discount_delta, !pending_inserts_.contains(entry_id));
    }

    return true;
}

void TableModelO::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column <= info_.entry_header.size() - 1);

    const EntryEnumO e_column { column };

    switch (e_column) {
    case EntryEnumO::kId:
        return;
    default:
        break;
    }

    auto Compare = [order, e_column](Entry* lhs, Entry* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryO>(lhs) };
        auto* d_rhs { DerivedPtr<EntryO>(rhs) };

        switch (e_column) {
        case EntryEnumO::kRhsNode:
            return (order == Qt::AscendingOrder) ? (d_lhs->rhs_node < d_rhs->rhs_node) : (d_lhs->rhs_node > d_rhs->rhs_node);
        case EntryEnumO::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (d_lhs->unit_price < d_rhs->unit_price) : (d_lhs->unit_price > d_rhs->unit_price);
        case EntryEnumO::kCount:
            return (order == Qt::AscendingOrder) ? (d_lhs->count < d_rhs->count) : (d_lhs->count > d_rhs->count);
        case EntryEnumO::kMeasure:
            return (order == Qt::AscendingOrder) ? (d_lhs->measure < d_rhs->measure) : (d_lhs->measure > d_rhs->measure);
        case EntryEnumO::kFinal:
            return (order == Qt::AscendingOrder) ? (d_lhs->final < d_rhs->final) : (d_lhs->final > d_rhs->final);
        case EntryEnumO::kInitial:
            return (order == Qt::AscendingOrder) ? (d_lhs->initial < d_rhs->initial) : (d_lhs->initial > d_rhs->initial);
        case EntryEnumO::kUnitDiscount:
            return (order == Qt::AscendingOrder) ? (d_lhs->unit_discount < d_rhs->unit_discount) : (d_lhs->unit_discount > d_rhs->unit_discount);
        case EntryEnumO::kExternalSku:
            return (order == Qt::AscendingOrder) ? (d_lhs->external_sku < d_rhs->external_sku) : (d_lhs->external_sku > d_rhs->external_sku);
        case EntryEnumO::kDiscount:
            return (order == Qt::AscendingOrder) ? (d_lhs->discount < d_rhs->discount) : (d_lhs->discount > d_rhs->discount);
        default:
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

bool TableModelO::insertRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent));
    if (d_node_->status == std::to_underlying(NodeStatus::kReleased))
        return false;

    auto* entry { EntryPool::Instance().Allocate(section_) };
    entry->id = QUuid::createUuidV7();
    entry->lhs_node = lhs_id_;

    beginInsertRows(parent, row, row);
    entry_list_.emplaceBack(entry);
    endInsertRows();

    pending_inserts_.insert(entry->id, entry);

    return true;
}

bool TableModelO::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent) - 1);
    if (d_node_->status == std::to_underlying(NodeStatus::kReleased))
        return false;

    auto* d_entry = DerivedPtr<EntryO>(entry_list_.at(row));

    const auto lhs_node { d_entry->lhs_node };
    const auto rhs_node { d_entry->rhs_node };
    const auto entry_id { d_entry->id };

    if (!pending_inserts_.contains(entry_id)) {
        if (auto it = pending_timers_.find(entry_id); it != pending_timers_.end()) {
            it.value()->stop();
            it.value()->deleteLater();
            pending_timers_.erase(it);
        }
    }

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
            emit SSyncDeltaO(lhs_node, initial_delta, final_delta, count_delta, measure_delta, discount_delta, !pending_inserts_.contains(entry_id));
        }
    }

    pending_updates_.remove(entry_id);

    auto it = pending_inserts_.find(entry_id);
    if (it == pending_inserts_.end()) {
        QJsonObject message {};
        WebSocket::Instance()->SendMessage(kEntryRemove, message);
    } else {
        pending_inserts_.erase(it);
    }

    EntryPool::Instance().Recycle(d_entry, section_);
    return true;
}

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
        auto& cache { pending_updates_[entry_id] };
        cache.insert(kExternalSku, value.toString(QUuid::WithoutBraces));

        if (price_changed) {
            cache.insert(kUnitPrice, QString::number(entry->unit_price, 'f', kMaxNumericScale_4));
            cache.insert(kInitial, QString::number(entry->initial, 'f', kMaxNumericScale_4));
            cache.insert(kFinal, QString::number(entry->final, 'f', kMaxNumericScale_4));
        }

        if (rhs_changed) {
            cache.insert(kRhsNode, new_rhs_node.toString(QUuid::WithoutBraces));
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

/// @brief Update entry by internal product ID (rhs_node)
/// @note Responsibility: Handle insertion and update, not deletion
/// @note rhs_node must be valid, external_sku is auto-filled
bool TableModelO::UpdateInternalSku(EntryO* entry, const QUuid& value)
{
    if (value.isNull())
        return false;

    auto old_rhs_node { entry->rhs_node };
    if (old_rhs_node == value)
        return false;

    const double old_unit_price { entry->unit_price };
    const QUuid old_external_sku { entry->external_sku };

    entry->rhs_node = value;

    ResolveFromInternal(entry, value);

    const bool price_changed { FloatChanged(old_unit_price, entry->unit_price) };
    const bool external_changed { old_external_sku != entry->external_sku };

    if (price_changed)
        RecalculateAmount(entry);

    if (!pending_inserts_.contains(entry->id)) {
        auto& cache { pending_updates_[entry->id] };
        cache.insert(kRhsNode, value.toString(QUuid::WithoutBraces));

        if (external_changed) {
            cache.insert(kExternalSku, entry->external_sku.toString(QUuid::WithoutBraces));
        }

        if (price_changed) {
            cache.insert(kUnitPrice, QString::number(entry->unit_price, 'f', kMaxNumericScale_4));
            cache.insert(kInitial, QString::number(entry->initial, 'f', kMaxNumericScale_4));
            cache.insert(kFinal, QString::number(entry->final, 'f', kMaxNumericScale_4));
        }

        ScheduleUpdate(entry->id);
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

bool TableModelO::UpdateUnitPrice(EntryO* entry, double value)
{
    if (FloatEqual(entry->unit_price, value))
        return false;

    entry->initial = entry->measure * value;
    entry->final = entry->initial - entry->discount;
    entry->unit_price = value;

    if (!pending_inserts_.contains(entry->id)) {
        auto& cache { pending_updates_[entry->id] };

        cache.insert(kUnitPrice, QString::number(entry->unit_price, 'f', kMaxNumericScale_4));
        cache.insert(kInitial, QString::number(entry->initial, 'f', kMaxNumericScale_4));
        cache.insert(kFinal, QString::number(entry->final, 'f', kMaxNumericScale_4));

        ScheduleUpdate(entry->id);
    }

    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kInitial));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));

    return true;
}

bool TableModelO::UpdateUnitDiscount(EntryO* entry, double value)
{
    if (FloatEqual(entry->unit_discount, value))
        return false;

    entry->discount = entry->measure * value;
    entry->final = entry->initial - entry->discount;
    entry->unit_discount = value;

    if (!pending_inserts_.contains(entry->id)) {
        auto& cache { pending_updates_[entry->id] };

        cache.insert(kUnitDiscount, QString::number(entry->unit_discount, 'f', kMaxNumericScale_4));
        cache.insert(kDiscount, QString::number(entry->discount, 'f', kMaxNumericScale_4));
        cache.insert(kFinal, QString::number(entry->final, 'f', kMaxNumericScale_4));

        ScheduleUpdate(entry->id);
    }

    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kDiscount));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));
    return true;
}

bool TableModelO::UpdateMeasure(EntryO* entry, double value)
{
    if (FloatEqual(entry->measure, value))
        return false;

    entry->initial = entry->unit_price * value;
    entry->discount = entry->unit_discount * value;
    entry->final = (entry->unit_price - entry->unit_discount) * value;

    entry->measure = value;

    if (!pending_inserts_.contains(entry->id)) {
        auto& cache { pending_updates_[entry->id] };

        cache.insert(kMeasure, QString::number(entry->measure, 'f', kMaxNumericScale_4));
        cache.insert(kInitial, QString::number(entry->initial, 'f', kMaxNumericScale_4));
        cache.insert(kDiscount, QString::number(entry->discount, 'f', kMaxNumericScale_4));
        cache.insert(kFinal, QString::number(entry->final, 'f', kMaxNumericScale_4));

        ScheduleUpdate(entry->id);
    }

    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kInitial));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kDiscount));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));
    return true;
}

bool TableModelO::UpdateCount(EntryO* entry, double value)
{
    if (FloatEqual(entry->count, value))
        return false;

    entry->count = value;

    if (!pending_inserts_.contains(entry->id)) {
        auto& cache { pending_updates_[entry->id] };

        cache.insert(kCount, QString::number(value, 'f', kMaxNumericScale_4));

        ScheduleUpdate(entry->id);
    }

    return true;
}

bool TableModelO::UpdateDescription(EntryO* entry, const QString& value)
{
    if (entry->description == value)
        return false;

    entry->description = value;

    if (!pending_inserts_.contains(entry->id)) {
        auto& cache { pending_updates_[entry->id] };

        cache.insert(kDescription, value);

        ScheduleUpdate(entry->id);
    }

    return true;
}

void TableModelO::ResolveFromInternal(EntryO* entry, const QUuid& internal_sku) const
{
    if (!entry || !entry_hub_p_ || internal_sku.isNull())
        return;

    if (auto result = entry_hub_p_->ResolveFromInternal(d_node_->partner, internal_sku)) {
        const auto& [external_id, price] = *result;
        entry->unit_price = price;
        entry->external_sku = external_id;
    } else {
        entry->unit_price = tree_model_i_->UnitPrice(internal_sku);
        entry->external_sku = QUuid();
    }
}

void TableModelO::ResolveFromExternal(EntryO* entry, const QUuid& external_sku) const
{
    if (!entry || !entry_hub_p_ || external_sku.isNull())
        return;

    if (auto result = entry_hub_p_->ResolveFromExternal(d_node_->partner, external_sku)) {
        const auto& [rhs_node, price] = *result;
        entry->unit_price = price;
        entry->rhs_node = rhs_node;
    } else {
        entry->unit_price = 0.0;
        entry->rhs_node = QUuid();
    }
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
// - Only entries in `pending_inserts_` are considered (newly created, not yet persisted).
// - Entries with null `rhs_node` (internal SKU not selected) are removed.
// - Corresponding pending updates are cleared.
// - The entry objects are recycled via EntryPool.
void TableModelO::PurifyEntry()
{
    for (auto i = entry_list_.size() - 1; i >= 0; --i) {
        auto* entry { entry_list_[i] };

        if (!pending_inserts_.contains(entry->id))
            continue;

        if (!entry->rhs_node.isNull())
            continue;

        beginRemoveRows(QModelIndex(), i, i);
        pending_updates_.remove(entry->id);
        EntryPool::Instance().Recycle(entry_list_.takeAt(i), section_);
        endRemoveRows();
    }
}

void TableModelO::ScheduleUpdate(const QUuid& id)
{
    if (pending_timers_.contains(id)) {
        pending_timers_[id]->stop();
    } else {
        auto* timer { new QTimer(this) };
        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout, this, [this, id]() {
            auto* expired_timer { pending_timers_.take(id) };

            if (auto it = pending_updates_.find(id); it != pending_updates_.end()) {
                const auto& update { it.value() };

                if (!update.isEmpty()) {
                    const QJsonObject message { JsonGen::EntryUpdate(section_, id, update) };
                    WebSocket::Instance()->SendMessage(kEntryUpdateOrder, message);
                }

                pending_updates_.erase(it);
            }

            expired_timer->deleteLater();
        });
        pending_timers_[id] = timer;
    }

    pending_timers_[id]->start(kThreeThousand);
}

void TableModelO::FlushUpdates()
{
    for (auto* timer : std::as_const(pending_timers_)) {
        timer->stop();
        timer->deleteLater();
    }

    pending_timers_.clear();

    for (auto it = pending_updates_.cbegin(); it != pending_updates_.cend(); ++it) {
        if (!it.value().isEmpty()) {
            const auto message { JsonGen::EntryUpdate(section_, it.key(), it.value()) };
            WebSocket::Instance()->SendMessage(kEntryUpdateOrder, message);
        }
    }

    pending_updates_.clear();
}
