#include "leafmodelo.h"

#include "global/entryshadowpool.h"
#include "utils/jsongen.h"

LeafModelO::LeafModelO(CLeafModelArg& arg, const Node* node, TreeModel* tree_model_item, EntryHub* entry_hub_stakeholder, QObject* parent)
    : LeafModel { arg, parent }
    , tree_model_item_ { static_cast<TreeModelI*>(tree_model_item) }
    , entry_hub_stakeholder_ { static_cast<EntryHubS*>(entry_hub_stakeholder) }
    , entry_hub_order_ { static_cast<EntryHubO*>(arg.entry_hub) }
    , party_id_ { static_cast<const NodeO*>(node)->party }
    , is_finished_ { static_cast<const NodeO*>(node)->is_finished }
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

void LeafModelO::UpdateParty(const QUuid& node_id, const QUuid& party_id)
{
    assert(lhs_id_ == node_id);
    if (party_id_ == party_id)
        return;

    party_id_ = party_id;
    // sql_stakeholder_->ReadTrans(party_id);
}

void LeafModelO::RSyncFinished(const QUuid& node_id, bool value)
{
    assert(lhs_id_ == node_id);
    is_finished_ = value;

    if (!value)
        return;

    PurifyEntryShadow();
}

void LeafModelO::RSyncParty(const QUuid& node_id, int column, const QUuid& value)
{
    const NodeEnumO kColumn { column };

    switch (kColumn) {
    case NodeEnumO::kParty:
        UpdateParty(node_id, value);
        break;
    default:
        break;
    }
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
    case EntryEnumO::kUserId:
        return *d_shadow->user_id;
    case EntryEnumO::kCreateTime:
        return *d_shadow->created_time;
    case EntryEnumO::kCreateBy:
        return *d_shadow->created_by;
    case EntryEnumO::kUpdateTime:
        return *d_shadow->updated_time;
    case EntryEnumO::kUpdateBy:
        return *d_shadow->updated_by;
    case EntryEnumO::kLhsNode:
        return *d_shadow->lhs_node;
    case EntryEnumO::kRhsNode:
        return d_shadow->rhs_node->isNull() ? QVariant() : *d_shadow->rhs_node;
    case EntryEnumO::kUnitPrice:
        return *d_shadow->unit_price == 0 ? QVariant() : *d_shadow->unit_price;
    case EntryEnumO::kMeasure:
        return *d_shadow->measure == 0 ? QVariant() : *d_shadow->measure;
    case EntryEnumO::kDescription:
        return *d_shadow->description;
    case EntryEnumO::kColor:
        return d_shadow->rhs_node->isNull() ? QVariant() : tree_model_item_->Color(*d_shadow->rhs_node);
    case EntryEnumO::kCount:
        return *d_shadow->count == 0 ? QVariant() : *d_shadow->count;
    case EntryEnumO::kFinal:
        return *d_shadow->final == 0 ? QVariant() : *d_shadow->final;
    case EntryEnumO::kDiscount:
        return *d_shadow->discount == 0 ? QVariant() : *d_shadow->discount;
    case EntryEnumO::kInitial:
        return *d_shadow->initial == 0 ? QVariant() : *d_shadow->initial;
    case EntryEnumO::kDiscountPrice:
        return *d_shadow->unit_price == 0 ? QVariant() : *d_shadow->unit_price;
    case EntryEnumO::kExternalItem:
        return d_shadow->external_item->isNull() ? QVariant() : *d_shadow->external_item;
    default:
        return QVariant();
    }
}

bool LeafModelO::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (is_finished_)
        return false;

    const EntryEnumO kColumn { index.column() };

    auto* shadow = shadow_list_.at(index.row());
    auto* d_shadow = DerivedPtr<EntryShadowO>(shadow);

    const auto old_rhs_node { *d_shadow->rhs_node };
    const double old_first { *d_shadow->count };
    const double old_second { *d_shadow->measure };
    const double old_discount { *d_shadow->discount };
    const double old_gross_amount { *d_shadow->initial };
    const double old_net_amount { *d_shadow->final };

    bool fir_changed { false };
    bool sec_changed { false };
    bool uni_changed { false };
    bool dis_changed { false };

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
        UpdateRhsNode(d_shadow, value.toUuid(), 0);
        break;
    case EntryEnumO::kUnitPrice:
        uni_changed = UpdateRate(d_shadow, value.toDouble());
        break;
    case EntryEnumO::kMeasure:
        sec_changed = UpdateSecond(d_shadow, value.toDouble(), kCoefficient);
        break;
    case EntryEnumO::kCount:
        fir_changed = UpdateFirst(d_shadow, value.toDouble(), kCoefficient);
        break;
    case EntryEnumO::kDiscountPrice:
        dis_changed = UpdateDiscountPrice(d_shadow, value.toDouble());
        break;
    case EntryEnumO::kExternalItem:
        UpdateExternalItem(d_shadow, value.toUuid());
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());

    if (fir_changed)
        emit SSyncDelta(*d_shadow->lhs_node, 0.0, 0.0, *d_shadow->count - old_first);

    if (sec_changed) {
        const double second_delta { *d_shadow->measure - old_second };
        const double gross_amount_delta { *d_shadow->initial - old_gross_amount };
        const double discount_delta { *d_shadow->discount - old_discount };
        const double net_amount_delta { *d_shadow->final - old_net_amount };
        emit SSyncDelta(*d_shadow->lhs_node, gross_amount_delta, net_amount_delta, 0.0, second_delta, discount_delta);
    }

    if (uni_changed) {
        const double gross_amount_delta { *d_shadow->initial - old_gross_amount };
        const double net_amount_delta { *d_shadow->final - old_net_amount };
        emit SSyncDelta(*d_shadow->lhs_node, gross_amount_delta, net_amount_delta);
    }

    if (dis_changed) {
        const double discount_delta { *d_shadow->discount - old_discount };
        const double net_amount_delta { *d_shadow->final - old_net_amount };
        emit SSyncDelta(*d_shadow->lhs_node, 0.0, net_amount_delta, 0.0, 0.0, discount_delta);
    }

    return true;
}

void LeafModelO::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column < info_.node_header.size());

    auto Compare = [column, order](EntryShadow* lhs, EntryShadow* rhs) -> bool {
        const EntryEnumO kColumn { column };
        auto* d_lhs { DerivedPtr<EntryShadowO>(lhs) };
        auto* d_rhs { DerivedPtr<EntryShadowO>(rhs) };

        switch (kColumn) {
        case EntryEnumO::kUserId:
            return (order == Qt::AscendingOrder) ? (*lhs->user_id < *rhs->user_id) : (*lhs->user_id > *rhs->user_id);
        case EntryEnumO::kCreateTime:
            return (order == Qt::AscendingOrder) ? (*lhs->created_time < *rhs->created_time) : (*lhs->created_time > *rhs->created_time);
        case EntryEnumO::kCreateBy:
            return (order == Qt::AscendingOrder) ? (*lhs->created_by < *rhs->created_by) : (*lhs->created_by > *rhs->created_by);
        case EntryEnumO::kUpdateTime:
            return (order == Qt::AscendingOrder) ? (*lhs->updated_time < *rhs->updated_time) : (*lhs->updated_time > *rhs->updated_time);
        case EntryEnumO::kUpdateBy:
            return (order == Qt::AscendingOrder) ? (*lhs->updated_by < *rhs->updated_by) : (*lhs->updated_by > *rhs->updated_by);
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
        case EntryEnumO::kDiscountPrice:
            return (order == Qt::AscendingOrder) ? (*d_lhs->unit_price < *d_rhs->unit_price) : (*d_lhs->unit_price > *d_rhs->unit_price);
        case EntryEnumO::kExternalItem:
            return (order == Qt::AscendingOrder) ? (*d_lhs->external_item < *d_rhs->external_item) : (*d_lhs->external_item > *d_rhs->external_item);
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
    case EntryEnumO::kColor:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}

bool LeafModelO::insertRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent));
    if (is_finished_)
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
    if (is_finished_)
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

bool LeafModelO::UpdateExternalItem(EntryShadowO* entry_shadow, const QUuid& value)
{
    if (*entry_shadow->external_item == value)
        return false;

    const auto old_rhs_node { *entry_shadow->rhs_node };

    *entry_shadow->external_item = value;
    CrossSearch(entry_shadow, value, false);

    if (!old_rhs_node.isNull()) {
        // dbhub_->WriteField(info_->entry, kExternalItem, value, *entry_shadow->id);
    }

    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kUnitPrice));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kRhsNode));

    bool ins_changed { *entry_shadow->rhs_node != old_rhs_node };
    return ins_changed;
}

bool LeafModelO::UpdateRhsNode(EntryShadow* entry_shadow, const QUuid& value, int /*row*/)
{
    if (value.isNull())
        return false;

    auto* d_shadow = DerivedPtr<EntryShadowO>(entry_shadow);
    auto old_rhs_node { *d_shadow->rhs_node };
    if (old_rhs_node == value)
        return false;

    *d_shadow->rhs_node = value;
    State state { caches_.take(*d_shadow->id) };

    CrossSearch(entry_shadow, value, true);
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kUnitPrice));
    emit SResizeColumnToContents(std::to_underlying(EntryEnumO::kExternalItem));

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

bool LeafModelO::UpdateDiscountPrice(EntryShadow* entry_shadow, double value)
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

bool LeafModelO::UpdateSecond(EntryShadow* entry_shadow, double value, int kCoefficient)
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

bool LeafModelO::UpdateFirst(EntryShadow* entry_shadow, double value, int kCoefficient)
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

void LeafModelO::CrossSearch(EntryShadow* entry_shadow, const QUuid& item_id, bool is_internal) const
{
    if (!entry_shadow || !entry_hub_stakeholder_ || item_id.isNull())
        return;

    auto* d_shadow = DerivedPtr<EntryShadowO>(entry_shadow);

    if (entry_hub_stakeholder_->CrossSearch(d_shadow, party_id_, item_id, is_internal))
        return;

    *d_shadow->unit_price = is_internal ? tree_model_item_->UnitPrice(item_id) : 0.0;
    is_internal ? * d_shadow->external_item = QUuid() : * d_shadow->rhs_node = QUuid();
}
