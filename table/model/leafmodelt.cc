#include "leafmodelt.h"

#include <QTimer>

#include "component/constant.h"
#include "global/entryshadowpool.h"
#include "global/websocket.h"
#include "utils/jsongen.h"

LeafModelT::LeafModelT(CLeafModelArg& arg, QObject* parent)
    : LeafModel { arg, parent }
{
}

QVariant LeafModelT::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_shadow = DerivedPtr<EntryShadowT>(shadow_list_.at(index.row()));
    const EntryEnumT kColumn { index.column() };

    switch (kColumn) {
    case EntryEnumT::kId:
        return *d_shadow->id;
    case EntryEnumT::kUserId:
        return *d_shadow->user_id;
    case EntryEnumT::kCreateTime:
        return *d_shadow->created_time;
    case EntryEnumT::kCreateBy:
        return *d_shadow->created_by;
    case EntryEnumT::kUpdateTime:
        return *d_shadow->updated_time;
    case EntryEnumT::kUpdateBy:
        return *d_shadow->updated_by;
    case EntryEnumT::kLhsNode:
        return *d_shadow->lhs_node;
    case EntryEnumT::kIssuedTime:
        return *d_shadow->issued_time;
    case EntryEnumT::kCode:
        return *d_shadow->code;
    case EntryEnumT::kUnitCost:
        return *d_shadow->unit_cost == 0 ? QVariant() : *d_shadow->unit_cost;
    case EntryEnumT::kDescription:
        return *d_shadow->description;
    case EntryEnumT::kRhsNode:
        return d_shadow->rhs_node->isNull() ? QVariant() : *d_shadow->rhs_node;
    case EntryEnumT::kMarkStatus:
        return *d_shadow->mark_status ? *d_shadow->mark_status : QVariant();
    case EntryEnumT::kDocument:
        return d_shadow->document->isEmpty() ? QVariant() : *d_shadow->document;
    case EntryEnumT::kDebit:
        return *d_shadow->lhs_debit == 0 ? QVariant() : *d_shadow->lhs_debit;
    case EntryEnumT::kCredit:
        return *d_shadow->lhs_credit == 0 ? QVariant() : *d_shadow->lhs_credit;
    case EntryEnumT::kBalance:
        return d_shadow->balance;
    default:
        return QVariant();
    }
}

bool LeafModelT::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const EntryEnumT column { index.column() };
    const int row { index.row() };

    auto* shadow { shadow_list_.at(index.row()) };
    auto* d_shadow = DerivedPtr<EntryShadowT>(shadow);

    const QUuid id { *shadow->id };

    switch (column) {
    case EntryEnumT::kIssuedTime:
        EntryUtils::UpdateShadowIssuedTime(caches_[id], shadow, kIssuedTime, value.toDateTime(), &EntryShadow::issued_time, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumT::kCode:
        EntryUtils::UpdateShadowField(caches_[id], shadow, kCode, value.toString(), &EntryShadow::code, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumT::kDocument:
        EntryUtils::UpdateShadowDocument(caches_[id], shadow, kDocument, value.toStringList(), &EntryShadow::document, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumT::kMarkStatus:
        EntryUtils::UpdateShadowField(caches_[id], shadow, kMarkStatus, value.toBool(), &EntryShadow::mark_status, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumT::kDescription:
        EntryUtils::UpdateShadowField(caches_[id], shadow, kDescription, value.toString(), &EntryShadow::description, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumT::kUnitCost:
        UpdateRate(d_shadow, value.toDouble());
        break;
    case EntryEnumT::kRhsNode:
        UpdateLinkedNode(d_shadow, value.toUuid(), row);
        break;
    case EntryEnumT::kDebit:
        UpdateNumeric(d_shadow, value.toDouble(), row, true);
        break;
    case EntryEnumT::kCredit:
        UpdateNumeric(d_shadow, value.toDouble(), row, false);
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());
    return true;
}

void LeafModelT::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0);
    if (column >= info_.entry_header.size() - 1)
        return;

    auto Compare = [column, order](EntryShadow* lhs, EntryShadow* rhs) -> bool {
        const EntryEnumT kColumn { column };
        auto* d_lhs { DerivedPtr<EntryShadowT>(lhs) };
        auto* d_rhs { DerivedPtr<EntryShadowT>(rhs) };

        switch (kColumn) {
        case EntryEnumT::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (*lhs->issued_time < *rhs->issued_time) : (*lhs->issued_time > *rhs->issued_time);
        case EntryEnumT::kUserId:
            return (order == Qt::AscendingOrder) ? (*lhs->user_id < *rhs->user_id) : (*lhs->user_id > *rhs->user_id);
        case EntryEnumT::kCreateTime:
            return (order == Qt::AscendingOrder) ? (*lhs->created_time < *rhs->created_time) : (*lhs->created_time > *rhs->created_time);
        case EntryEnumT::kCreateBy:
            return (order == Qt::AscendingOrder) ? (*lhs->created_by < *rhs->created_by) : (*lhs->created_by > *rhs->created_by);
        case EntryEnumT::kUpdateTime:
            return (order == Qt::AscendingOrder) ? (*lhs->updated_time < *rhs->updated_time) : (*lhs->updated_time > *rhs->updated_time);
        case EntryEnumT::kUpdateBy:
            return (order == Qt::AscendingOrder) ? (*lhs->updated_by < *rhs->updated_by) : (*lhs->updated_by > *rhs->updated_by);
        case EntryEnumT::kCode:
            return (order == Qt::AscendingOrder) ? (*lhs->code < *rhs->code) : (*lhs->code > *rhs->code);
        case EntryEnumT::kUnitCost:
            return (order == Qt::AscendingOrder) ? (*d_lhs->unit_cost < *d_rhs->unit_cost) : (*d_lhs->unit_cost > *d_rhs->unit_cost);
        case EntryEnumT::kDescription:
            return (order == Qt::AscendingOrder) ? (*lhs->description < *rhs->description) : (*lhs->description > *rhs->description);
        case EntryEnumT::kRhsNode:
            return (order == Qt::AscendingOrder) ? (*lhs->rhs_node < *rhs->rhs_node) : (*lhs->rhs_node > *rhs->rhs_node);
        case EntryEnumT::kMarkStatus:
            return (order == Qt::AscendingOrder) ? (*lhs->mark_status < *rhs->mark_status) : (*lhs->mark_status > *rhs->mark_status);
        case EntryEnumT::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document->size() < rhs->document->size()) : (lhs->document->size() > rhs->document->size());
        case EntryEnumT::kDebit:
            return (order == Qt::AscendingOrder) ? (*d_lhs->lhs_debit < *d_rhs->lhs_debit) : (*d_lhs->lhs_debit > *d_rhs->lhs_debit);
        case EntryEnumT::kCredit:
            return (order == Qt::AscendingOrder) ? (*d_lhs->lhs_credit < *d_rhs->lhs_credit) : (*d_lhs->lhs_credit > *d_rhs->lhs_credit);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(shadow_list_.begin(), shadow_list_.end(), Compare);
    emit layoutChanged();

    AccumulateBalance(0);
}

Qt::ItemFlags LeafModelT::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const EntryEnumT kColumn { index.column() };

    switch (kColumn) {
    case EntryEnumT::kId:
    case EntryEnumT::kBalance:
    case EntryEnumT::kDocument:
    case EntryEnumT::kMarkStatus:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}

bool LeafModelT::UpdateNumeric(EntryShadow* entry_shadow, double value, int row, bool is_debit)
{
    auto* d_shadow { DerivedPtr<EntryShadowT>(entry_shadow) };

    const double lhs_old_debit = *d_shadow->lhs_debit;
    const double lhs_old_credit = *d_shadow->lhs_credit;
    const double unit_cost = *d_shadow->unit_cost;

    double lhs_original = is_debit ? lhs_old_debit : lhs_old_credit;
    if (std::abs(lhs_original - value) < kTolerance)
        return false;

    const double base = is_debit ? lhs_old_credit : lhs_old_debit;
    const double diff = qAbs(value - base);

    const bool assign_debit = (is_debit && value > base) || (!is_debit && value <= base);
    const bool assign_credit = !assign_debit;

    *d_shadow->lhs_debit = assign_debit ? diff : 0.0;
    *d_shadow->lhs_credit = assign_credit ? diff : 0.0;

    *d_shadow->rhs_debit = *d_shadow->lhs_credit;
    *d_shadow->rhs_credit = *d_shadow->lhs_debit;

    if (d_shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id = *d_shadow->id;
    const QUuid rhs_id { *d_shadow->rhs_node };

    QJsonObject cache {};
    const bool is_parallel = entry_shadow->is_parallel;

    cache.insert(is_parallel ? kLhsDebit : kRhsDebit, QString::number(*d_shadow->lhs_debit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kLhsCredit : kRhsCredit, QString::number(*d_shadow->lhs_credit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kRhsDebit : kLhsDebit, QString::number(*d_shadow->rhs_debit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kRhsCredit : kLhsCredit, QString::number(*d_shadow->rhs_credit, 'f', kMaxNumericScale_4));

    QJsonObject message {};
    message.insert(kSection, info_.section_str);
    message.insert(kSessionId, QString());
    message.insert(kCache, cache);
    message.insert(kIsParallel, is_parallel);
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));

    const double lhs_initial_delta = *d_shadow->lhs_debit - *d_shadow->lhs_credit - (lhs_old_debit - lhs_old_credit);
    const double lhs_final_delta = unit_cost * lhs_initial_delta;

    const double rhs_initial_delta = -lhs_initial_delta;
    const double rhs_final_delta = unit_cost * rhs_initial_delta;

    const bool has_leaf_delta = std::abs(lhs_initial_delta) > kTolerance;

    if (has_leaf_delta) {
        QJsonObject lhs_delta { JsonGen::NodeDelta(lhs_id_, lhs_initial_delta, lhs_final_delta) };
        QJsonObject rhs_delta { JsonGen::NodeDelta(rhs_id, rhs_initial_delta, rhs_final_delta) };

        message.insert(kLhsDelta, lhs_delta);
        message.insert(kRhsDelta, rhs_delta);
    }

    WebSocket::Instance()->SendMessage(kEntryNumeric, message);

    if (has_leaf_delta) {
        AccumulateBalance(row);

        emit SResizeColumnToContents(std::to_underlying(EntryEnumT::kBalance));
        emit SUpdateBalance(rhs_id, *d_shadow->id);

        emit SSyncDelta(lhs_id_, lhs_initial_delta, lhs_final_delta);
        emit SSyncDelta(rhs_id, rhs_initial_delta, rhs_final_delta);
    }

    return true;
}

bool LeafModelT::UpdateRate(EntryShadow* entry_shadow, double value)
{
    auto* d_shadow { DerivedPtr<EntryShadowT>(entry_shadow) };

    const double old_unit_cost { *d_shadow->unit_cost };
    if (std::abs(old_unit_cost - value) < kTolerance || value < 0)
        return false;

    const double delta { value - old_unit_cost };
    *d_shadow->unit_cost = value;

    if (d_shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *d_shadow->id };
    QJsonObject cache {};

    cache.insert(kUnitCost, QString::number(value, 'f', kMaxNumericScale_4));

    const double lhs_final_delta { delta * (*d_shadow->lhs_debit - *d_shadow->lhs_credit) };
    const double rhs_final_delta { -lhs_final_delta };

    const bool has_leaf_delta = std::abs(lhs_final_delta) > kTolerance;

    QJsonObject message {};
    message.insert(kSection, info_.section_str);
    message.insert(kSessionId, QString());
    message.insert(kCache, cache);
    message.insert(kIsParallel, true);
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));

    if (has_leaf_delta) {
        QJsonObject lhs_delta { JsonGen::NodeDelta(lhs_id_, 0.0, lhs_final_delta) };
        QJsonObject rhs_delta { JsonGen::NodeDelta(*d_shadow->rhs_node, 0.0, rhs_final_delta) };

        message.insert(kLhsDelta, lhs_delta);
        message.insert(kRhsDelta, rhs_delta);
    }

    WebSocket::Instance()->SendMessage(kEntryRate, message);

    if (has_leaf_delta) {
        emit SSyncDelta(lhs_id_, 0.0, lhs_final_delta);
        emit SSyncDelta(*d_shadow->rhs_node, 0.0, rhs_final_delta);
    }

    return true;
}

bool LeafModelT::UpdateLinkedNode(EntryShadow* entry_shadow, const QUuid& value, int row)
{
    if (value.isNull())
        return false;

    auto* d_shadow = DerivedPtr<EntryShadowT>(entry_shadow);
    auto old_node { *d_shadow->rhs_node };
    if (old_node == value)
        return false;

    *d_shadow->rhs_node = value;

    const QUuid entry_id { *d_shadow->id };
    const double unit_cost { *d_shadow->unit_cost };

    const QString old_node_id = old_node.toString(QUuid::WithoutBraces);
    const QString new_node_id = value.toString(QUuid::WithoutBraces);

    QJsonObject cache {};
    cache = d_shadow->WriteJson();

    QJsonObject message {};
    message.insert(kSection, info_.section_str);
    message.insert(kSessionId, QString());
    message.insert(kEntry, cache);
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));

    if (old_node.isNull()) {
        double lhs_debit { *d_shadow->lhs_debit };
        double lhs_credit { *d_shadow->lhs_credit };

        const double lhs_initial_delta = lhs_debit - lhs_credit;
        const double lhs_final_delta = unit_cost * lhs_initial_delta;

        const double rhs_initial_delta = -lhs_initial_delta;
        const double rhs_final_delta = -lhs_final_delta;

        const bool has_leaf_delta = std::abs(lhs_initial_delta) > kTolerance;

        if (has_leaf_delta) {
            QJsonObject lhs_delta { JsonGen::NodeDelta(lhs_id_, lhs_initial_delta, lhs_final_delta) };
            QJsonObject rhs_delta { JsonGen::NodeDelta(value, rhs_initial_delta, rhs_final_delta) };

            message.insert(kLhsDelta, lhs_delta);
            message.insert(kRhsDelta, rhs_delta);
        }

        if (has_leaf_delta) {
            AccumulateBalance(row);

            emit SResizeColumnToContents(std::to_underlying(EntryEnumT::kBalance));
            emit SSyncDelta(lhs_id_, lhs_initial_delta, lhs_final_delta);
            emit SSyncDelta(value, rhs_initial_delta, rhs_final_delta);
        }

        emit SAppendOneEntry(value, d_shadow->entry);

        WebSocket::Instance()->SendMessage(kEntryInsert, message);
    }

    if (!old_node.isNull()) {
        const bool is_parallel { d_shadow->is_parallel };
        const auto field = is_parallel ? kRhsNode : kLhsNode;

        const double rhs_debit { *d_shadow->rhs_debit };
        const double rhs_credit { *d_shadow->rhs_credit };

        const double rhs_initial_delta = rhs_debit - rhs_credit;
        const double rhs_final_delta = unit_cost * rhs_initial_delta;

        const bool has_leaf_delta = std::abs(rhs_initial_delta) > kTolerance;

        if (has_leaf_delta) {
            QJsonObject new_node_delta { JsonGen::NodeDelta(value, rhs_initial_delta, rhs_final_delta) };
            QJsonObject old_node_delta { JsonGen::NodeDelta(old_node, -rhs_initial_delta, -rhs_final_delta) };

            message.insert(kNewNodeDelta, new_node_delta);
            message.insert(kOldNodeDelta, old_node_delta);
        }

        if (has_leaf_delta) {
            AccumulateBalance(row);

            emit SResizeColumnToContents(std::to_underlying(EntryEnumT::kBalance));
            emit SSyncDelta(value, rhs_initial_delta, rhs_final_delta);
            emit SSyncDelta(old_node, -rhs_initial_delta, -rhs_final_delta);
        }

        emit SRemoveOneEntry(old_node, entry_id);
        emit SAppendOneEntry(value, d_shadow->entry);

        message.insert(kOldNodeId, old_node_id);
        message.insert(kNewNodeId, new_node_id);
        message.insert(kField, field);

        WebSocket::Instance()->SendMessage(kEntryLinkedNode, message);
    }

    return true;
}

double LeafModelT::CalculateBalance(EntryShadow* entry_shadow)
{
    auto* d_shadow { DerivedPtr<EntryShadowT>(entry_shadow) };
    return (direction_rule_ ? 1 : -1) * (*d_shadow->lhs_debit - *d_shadow->lhs_credit);
}

bool LeafModelT::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent) - 1);

    auto* d_shadow = DerivedPtr<EntryShadowT>(shadow_list_.at(row));
    const auto rhs_node_id { *d_shadow->rhs_node };

    beginRemoveRows(parent, row, row);
    shadow_list_.removeAt(row);
    endRemoveRows();

    if (!rhs_node_id.isNull()) {
        const double lhs_initial_delta { *d_shadow->lhs_credit - *d_shadow->lhs_debit };
        const double lhs_final_delta { *d_shadow->unit_cost * lhs_initial_delta };

        const double rhs_initial_delta { *d_shadow->rhs_credit - *d_shadow->rhs_debit };
        const double rhs_final_delta { *d_shadow->unit_cost * rhs_initial_delta };

        const auto entry_id { *d_shadow->id };
        const bool has_delta = std::abs(lhs_initial_delta) > kTolerance;

        QJsonObject message {};
        message.insert(kSection, info_.section_str);
        message.insert(kSessionId, QString());
        message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));

        if (has_delta) {
            QJsonObject lhs_delta { JsonGen::NodeDelta(lhs_id_, lhs_initial_delta, lhs_final_delta) };
            message.insert(kLhsDelta, lhs_delta);

            QJsonObject rhs_delta { JsonGen::NodeDelta(*d_shadow->rhs_node, rhs_initial_delta, rhs_final_delta) };
            message.insert(kRhsDelta, rhs_delta);
        }

        WebSocket::Instance()->SendMessage(kEntryRemove, message);

        if (has_delta) {
            emit SSyncDelta(lhs_id_, lhs_initial_delta, lhs_final_delta);
            emit SSyncDelta(*d_shadow->rhs_node, rhs_initial_delta, rhs_final_delta);
            AccumulateBalance(row);
        }

        emit SRemoveOneEntry(rhs_node_id, entry_id);
    }

    EntryShadowPool::Instance().Recycle(d_shadow, section_);
    return true;
}
