#include "leafmodelo.h"

#include "global/entryshadowpool.h"
#include "websocket/jsongen.h"

LeafModelO::LeafModelO(CLeafModelArg& arg, const Node* node, TreeModel* tree_model_inventory, EntryHub* entry_hub_partner, QObject* parent)
    : LeafModel { arg, parent }
    , tree_model_i_ { static_cast<TreeModelI*>(tree_model_inventory) }
    , entry_hub_partner_ { static_cast<EntryHubP*>(entry_hub_partner) }
    , entry_hub_order_ { static_cast<EntryHubO*>(arg.entry_hub) }
    , partner_id_ { static_cast<const NodeO*>(node)->partner }
    , node_status_ { static_cast<const NodeO*>(node)->status }
{
}

void LeafModelO::RSaveOrder()
{
    if (shadow_list_.isEmpty())
        return;

    PurifyEntryShadow();

    if (!shadow_list_.isEmpty())
        entry_hub_order_->WriteTransRange(shadow_list_);
}

void LeafModelO::RSyncStatus(const QUuid& node_id, bool value)
{
    assert(lhs_id_ == node_id);
    node_status_ = value;

    if (!value)
        return;

    PurifyEntryShadow();
}

void LeafModelO::RSyncPartner(const QUuid& node_id, const QUuid& value)
{
    assert(lhs_id_ == node_id);
    partner_id_ = value;
}

QVariant LeafModelO::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_shadow = DerivedPtr<EntryShadowO>(shadow_list_.at(index.row()));
    const EntryEnumO kColumn { index.column() };

    switch (kColumn) {
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
        return *d_shadow->unit_price;
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

    const EntryEnumO kColumn { index.column() };

    auto* shadow = shadow_list_.at(index.row());
    auto* d_shadow = DerivedPtr<EntryShadowO>(shadow);

    const auto old_rhs_node { *d_shadow->rhs_node };
    const double old_count { *d_shadow->count };
    const double old_measure { *d_shadow->measure };
    const double old_discount { *d_shadow->discount };
    const double old_gross_amount { *d_shadow->initial };
    const double old_net_amount { *d_shadow->final };

    bool count_changed { false };
    bool measure_changed { false };
    bool unit_price_changed { false };
    bool unit_discount_changed { false };

    const QUuid id { *d_shadow->id };
    auto& entry { caches_[id] };

    if (!old_rhs_node.isNull()) {
        RestartTimer(id);
    }

    const int kCoefficient { direction_rule_ ? -1 : 1 };

    switch (kColumn) {
    case EntryEnumO::kDescription:
        EntryUtils::UpdateShadowField(entry, shadow, kDescription, value.toString(), &EntryShadow::description);
        break;
    case EntryEnumO::kRhsNode:
        UpdateLinkedNode(d_shadow, value.toUuid(), 0);
        break;
    case EntryEnumO::kUnitPrice:
        unit_price_changed = UpdateRate(d_shadow, value.toDouble());
        break;
    case EntryEnumO::kMeasure:
        measure_changed = UpdateMeasure(d_shadow, value.toDouble(), kCoefficient);
        break;
    case EntryEnumO::kCount:
        count_changed = UpdateCount(d_shadow, value.toDouble(), kCoefficient);
        break;
    case EntryEnumO::kUnitDiscount:
        unit_discount_changed = UpdateUnitDiscount(d_shadow, value.toDouble());
        break;
    case EntryEnumO::kExternalSku:
        UpdateExternaSku(d_shadow, value.toUuid());
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());

    if (count_changed)
        emit SSyncDelta(*d_shadow->lhs_node, 0.0, 0.0, *d_shadow->count - old_count);

    if (measure_changed) {
        const double second_delta { *d_shadow->measure - old_measure };
        const double gross_amount_delta { *d_shadow->initial - old_gross_amount };
        const double discount_delta { *d_shadow->discount - old_discount };
        const double net_amount_delta { *d_shadow->final - old_net_amount };
        emit SSyncDelta(*d_shadow->lhs_node, gross_amount_delta, net_amount_delta, 0.0, second_delta, discount_delta);
    }

    if (unit_price_changed) {
        const double gross_amount_delta { *d_shadow->initial - old_gross_amount };
        const double net_amount_delta { *d_shadow->final - old_net_amount };
        emit SSyncDelta(*d_shadow->lhs_node, gross_amount_delta, net_amount_delta);
    }

    if (unit_discount_changed) {
        const double discount_delta { *d_shadow->discount - old_discount };
        const double net_amount_delta { *d_shadow->final - old_net_amount };
        emit SSyncDelta(*d_shadow->lhs_node, 0.0, net_amount_delta, 0.0, 0.0, discount_delta);
    }

    return true;
}

void LeafModelO::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column <= info_.entry_header.size() - 1);

    const EntryEnumO kColumn { column };

    switch (kColumn) {
    case EntryEnumO::kId:
        return;
    default:
        break;
    }

    auto Compare = [order, kColumn](EntryShadow* lhs, EntryShadow* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryShadowO>(lhs) };
        auto* d_rhs { DerivedPtr<EntryShadowO>(rhs) };

        switch (kColumn) {
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
    const EntryEnumO kColumn { index.column() };

    switch (kColumn) {
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

    if (node_status_ == std::to_underlying(NodeStatus::kReleased))
        flags &= ~Qt::ItemIsEditable;

    return flags;
}

bool LeafModelO::insertRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent));
    if (node_status_ == std::to_underlying(NodeStatus::kReleased))
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
    if (node_status_ == std::to_underlying(NodeStatus::kReleased))
        return false;

    auto* d_shadow = DerivedPtr<EntryShadowO>(shadow_list_.at(row));
    const auto lhs_node { *d_shadow->lhs_node };
    const auto rhs_node { *d_shadow->rhs_node };

    beginRemoveRows(parent, row, row);
    shadow_list_.removeAt(row);
    endRemoveRows();

    emit SSyncDelta(lhs_node, -*d_shadow->initial, -*d_shadow->final, -*d_shadow->count, -*d_shadow->measure, -*d_shadow->discount);

    if (!lhs_node.isNull() && !rhs_node.isNull()) {
        // dbhub_->RemoveTrans(*d_shadow->id);
    }

    EntryShadowPool::Instance().Recycle(d_shadow, section_);
    return true;
}

bool LeafModelO::UpdateExternaSku(EntryShadowO* entry_shadow, const QUuid& value)
{
    if (*entry_shadow->external_sku == value)
        return false;

    const auto old_rhs_node { *entry_shadow->rhs_node };

    *entry_shadow->external_sku = value;
    ResolveFromExternal(entry_shadow, value);

    if (!old_rhs_node.isNull()) {
        // dbhub_->WriteField(info_->entry, kExternalItem, value, *entry_shadow->id);
    }

    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kUnitPrice));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kRhsNode));

    bool ins_changed { *entry_shadow->rhs_node != old_rhs_node };
    return ins_changed;
}

bool LeafModelO::UpdateLinkedNode(EntryShadow* entry_shadow, const QUuid& value, int /*row*/)
{
    if (value.isNull())
        return false;

    auto* d_shadow = DerivedPtr<EntryShadowO>(entry_shadow);
    auto old_rhs_node { *d_shadow->rhs_node };
    if (old_rhs_node == value)
        return false;

    *d_shadow->rhs_node = value;
    State state { caches_.take(*d_shadow->id) };

    ResolveFromInternal(d_shadow, value);
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kUnitPrice));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kExternalSku));

    if (old_rhs_node.isNull()) {
        // const auto message { JsonGen::InsertEntry(info_->section_str, d_shadow, node_id_) };
        // WebSocket::Instance().SendMessage(kInsertEntry, message);

        emit SSyncDelta(*d_shadow->lhs_node, *d_shadow->initial, *d_shadow->final, *d_shadow->count, *d_shadow->measure, *d_shadow->discount);
    }

    if (!old_rhs_node.isNull()) {
        // cache.insert(kRhsNode, value.toString(QUuid::WithoutBraces));
    }

    return true;
}

bool LeafModelO::UpdateRate(EntryShadow* entry_shadow, double value)
{
    auto* d_shadow = DerivedPtr<EntryShadowO>(entry_shadow);

    if (std::abs(*d_shadow->unit_price - value) < kTolerance)
        return false;

    const double delta { *d_shadow->measure * (value - *d_shadow->unit_price) };
    *d_shadow->final += delta;
    *d_shadow->initial += delta;
    *d_shadow->unit_price = value;

    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kInitial));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));

    if (d_shadow->lhs_node->isNull() || d_shadow->rhs_node->isNull())
        return true;

    // cache.insert(kUnitPrice, *d_shadow->unit_price);
    // cache.insert(kInitial, *d_shadow->initial);
    // cache.insert(kFinal, *d_shadow->final);

    return true;
}

bool LeafModelO::UpdateUnitDiscount(EntryShadow* entry_shadow, double value)
{
    auto* d_shadow = DerivedPtr<EntryShadowO>(entry_shadow);

    if (std::abs(*d_shadow->unit_price - value) < kTolerance)
        return false;

    const double delta { *d_shadow->measure * (value - *d_shadow->unit_price) };
    *d_shadow->final -= delta;
    *d_shadow->discount += delta;
    *d_shadow->unit_price = value;

    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kDiscount));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));

    if (d_shadow->lhs_node->isNull() || d_shadow->rhs_node->isNull())
        return true;

    // dbhub_->WriteField(info_->entry, kDiscountPrice, value, *d_shadow->id);
    return true;
}

bool LeafModelO::UpdateMeasure(EntryShadow* entry_shadow, double value, int kCoefficient)
{
    auto* d_shadow = DerivedPtr<EntryShadowO>(entry_shadow);

    if (std::abs(*d_shadow->measure - value) < kTolerance)
        return false;

    const double delta { value * kCoefficient - *d_shadow->measure };
    *d_shadow->initial += *d_shadow->unit_price * delta;
    *d_shadow->discount += *d_shadow->unit_price * delta;
    *d_shadow->final += (*d_shadow->unit_price - *d_shadow->unit_price) * delta;

    *d_shadow->measure = value * kCoefficient;

    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kInitial));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kDiscount));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kFinal));

    if (d_shadow->lhs_node->isNull() || d_shadow->rhs_node->isNull())
        // Return without writing data to SQLite
        return true;

    return true;
}

bool LeafModelO::UpdateCount(EntryShadow* entry_shadow, double value, int kCoefficient)
{
    auto* d_shadow = DerivedPtr<EntryShadowO>(entry_shadow);

    if (std::abs(*d_shadow->count - value) < kTolerance)
        return false;

    // EntryUtils::UpdateShadow(dbhub_, d_shadow, info_->trans, kFirst, value * kCoefficient, &OTransShadow::first);
    return true;
}

void LeafModelO::PurifyEntryShadow()
{
    for (auto i = shadow_list_.size() - 1; i >= 0; --i) {
        auto* entry_shadow { shadow_list_[i] };

        if (entry_shadow->rhs_node->isNull()) {
            beginRemoveRows(QModelIndex(), i, i);
            EntryShadowPool::Instance().Recycle(shadow_list_.takeAt(i), section_);
            endRemoveRows();
        }
    }
}

void LeafModelO::ResolveFromInternal(EntryShadowO* shadow, const QUuid& internal_sku) const
{
    if (!shadow || !entry_hub_partner_ || internal_sku.isNull())
        return;

    if (auto result = entry_hub_partner_->ResolveFromInternal(partner_id_, internal_sku)) {
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

    if (auto result = entry_hub_partner_->ResolveFromExternal(partner_id_, external_sku)) {
        const auto& [rhs_node, price] = *result;
        *shadow->unit_price = price;
        *shadow->rhs_node = rhs_node;
    } else {
        *shadow->unit_price = 0.0;
        *shadow->rhs_node = QUuid();
    }
}
