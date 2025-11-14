#include "tablemodelt.h"

#include "component/constant.h"
#include "global/entryshadowpool.h"
#include "utils/entryutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableModelT::TableModelT(CTableModelArg& arg, const Node* node, QObject* parent)
    : TableModel { arg, parent }
    , d_node_ { static_cast<const NodeT*>(node) }
{
}

QVariant TableModelT::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_shadow = DerivedPtr<EntryShadowT>(shadow_list_.at(index.row()));
    const EntryEnumT column { index.column() };

    switch (column) {
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
        return *d_shadow->unit_cost;
    case EntryEnumT::kDescription:
        return *d_shadow->description;
    case EntryEnumT::kRhsNode:
        return *d_shadow->rhs_node;
    case EntryEnumT::kStatus:
        return *d_shadow->status;
    case EntryEnumT::kDocument:
        return *d_shadow->document;
    case EntryEnumT::kDebit:
        return *d_shadow->lhs_debit;
    case EntryEnumT::kCredit:
        return *d_shadow->lhs_credit;
    case EntryEnumT::kBalance:
        return d_shadow->balance;
    default:
        return QVariant();
    }
}

bool TableModelT::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (d_node_->status == std::to_underlying(NodeStatus::kReleased)) {
        qInfo() << "Edit ignored: node is released, entry cannot be edited either.";
        return false;
    }

    const EntryEnumT column { index.column() };
    const int row { index.row() };

    auto* shadow { shadow_list_.at(index.row()) };
    auto* d_shadow = DerivedPtr<EntryShadowT>(shadow);

    const QUuid id { *shadow->id };

    switch (column) {
    case EntryEnumT::kIssuedTime:
        EntryUtils::UpdateShadowIssuedTime(
            pending_updates_[id], shadow, kIssuedTime, value.toDateTime(), &EntryShadow::issued_time, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumT::kCode:
        EntryUtils::UpdateShadowField(pending_updates_[id], shadow, kCode, value.toString(), &EntryShadow::code, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumT::kDocument:
        EntryUtils::UpdateShadowDocument(
            pending_updates_[id], shadow, kDocument, value.toStringList(), &EntryShadow::document, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumT::kStatus:
        EntryUtils::UpdateShadowField(pending_updates_[id], shadow, kStatus, value.toInt(), &EntryShadow::status, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumT::kDescription:
        EntryUtils::UpdateShadowField(
            pending_updates_[id], shadow, kDescription, value.toString(), &EntryShadow::description, [id, this]() { RestartTimer(id); });
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

void TableModelT::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column <= info_.entry_header.size() - 1);

    const EntryEnumT e_column { column };

    switch (e_column) {
    case EntryEnumT::kId:
    case EntryEnumT::kBalance:
    case EntryEnumT::kUserId:
    case EntryEnumT::kCreateTime:
    case EntryEnumT::kCreateBy:
    case EntryEnumT::kUpdateTime:
    case EntryEnumT::kUpdateBy:
        return;
    default:
        break;
    }

    auto Compare = [order, e_column](EntryShadow* lhs, EntryShadow* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryShadowT>(lhs) };
        auto* d_rhs { DerivedPtr<EntryShadowT>(rhs) };

        switch (e_column) {
        case EntryEnumT::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (*lhs->issued_time < *rhs->issued_time) : (*lhs->issued_time > *rhs->issued_time);
        case EntryEnumT::kCode:
            return (order == Qt::AscendingOrder) ? (*lhs->code < *rhs->code) : (*lhs->code > *rhs->code);
        case EntryEnumT::kUnitCost:
            return (order == Qt::AscendingOrder) ? (*d_lhs->unit_cost < *d_rhs->unit_cost) : (*d_lhs->unit_cost > *d_rhs->unit_cost);
        case EntryEnumT::kDescription:
            return (order == Qt::AscendingOrder) ? (*lhs->description < *rhs->description) : (*lhs->description > *rhs->description);
        case EntryEnumT::kRhsNode:
            return (order == Qt::AscendingOrder) ? (*lhs->rhs_node < *rhs->rhs_node) : (*lhs->rhs_node > *rhs->rhs_node);
        case EntryEnumT::kStatus:
            return (order == Qt::AscendingOrder) ? (*lhs->status < *rhs->status) : (*lhs->status > *rhs->status);
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

Qt::ItemFlags TableModelT::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const EntryEnumT column { index.column() };

    switch (column) {
    case EntryEnumT::kId:
    case EntryEnumT::kBalance:
    case EntryEnumT::kDocument:
    case EntryEnumT::kStatus:
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

bool TableModelT::UpdateNumeric(EntryShadow* entry_shadow, double value, int row, bool is_debit)
{
    auto* d_shadow { DerivedPtr<EntryShadowT>(entry_shadow) };

    const double lhs_old_debit { *d_shadow->lhs_debit };
    const double lhs_old_credit { *d_shadow->lhs_credit };
    const double unit_cost { *d_shadow->unit_cost };

    const double old_value { is_debit ? lhs_old_debit : lhs_old_credit };
    if (FloatEqual(old_value, value))
        return false;

    // Base represents the opposite side (used to compute the new diff)
    const double base { is_debit ? lhs_old_credit : lhs_old_debit };
    const double diff { std::abs(value - base) };

    // Determine which side (debit/credit) should hold the new value
    const bool to_debit { (is_debit && value > base) || (!is_debit && value <= base) };

    *d_shadow->lhs_debit = to_debit ? diff : 0.0;
    *d_shadow->lhs_credit = to_debit ? 0.0 : diff;

    // Mirror to RHS (Debit ↔ Credit)
    *d_shadow->rhs_debit = *d_shadow->lhs_credit;
    *d_shadow->rhs_credit = *d_shadow->lhs_debit;

    if (d_shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *d_shadow->id };
    const QUuid rhs_id { *d_shadow->rhs_node };

    QJsonObject cache {};
    const bool is_parallel { entry_shadow->is_parallel };

    cache.insert(is_parallel ? kLhsDebit : kRhsDebit, QString::number(*d_shadow->lhs_debit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kLhsCredit : kRhsCredit, QString::number(*d_shadow->lhs_credit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kRhsDebit : kLhsDebit, QString::number(*d_shadow->rhs_debit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kRhsCredit : kLhsCredit, QString::number(*d_shadow->rhs_credit, 'f', kMaxNumericScale_4));

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section_));
    message.insert(kSessionId, QString());
    message.insert(kCache, cache);
    message.insert(kIsParallel, is_parallel);
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));
    message.insert(kMeta, QJsonObject());

    // Delta calculation follows the DICD rule (Debit - Credit).
    // After the delta is computed, both the node and the server
    // will adjust the delta value according to the node's direction rule
    // (DICD → unchanged, DDCI → inverted).
    const double lhs_initial_delta { *d_shadow->lhs_debit - *d_shadow->lhs_credit - (lhs_old_debit - lhs_old_credit) };
    const double lhs_final_delta { unit_cost * lhs_initial_delta };

    // The right-hand side (RHS) node must always mirror the left-hand side (LHS),
    // therefore its delta is the opposite of LHS delta.
    // This ensures overall balance (LHS + RHS = 0).
    const double rhs_initial_delta { -lhs_initial_delta };
    const double rhs_final_delta { -lhs_final_delta };

    const bool has_leaf_delta { std::abs(lhs_initial_delta) > kTolerance };

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

        emit SNodeDelta(lhs_id_, lhs_initial_delta, lhs_final_delta);
        emit SNodeDelta(rhs_id, rhs_initial_delta, rhs_final_delta);
    }

    return true;
}

bool TableModelT::UpdateRate(EntryShadow* entry_shadow, double value)
{
    auto* d_shadow { DerivedPtr<EntryShadowT>(entry_shadow) };

    const double old_unit_cost { *d_shadow->unit_cost };
    if (FloatEqual(old_unit_cost, value) || value < 0)
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

    const bool has_leaf_delta { std::abs(lhs_final_delta) > kTolerance };

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section_));
    message.insert(kSessionId, QString());
    message.insert(kCache, cache);
    message.insert(kIsParallel, true);
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));
    message.insert(kMeta, QJsonObject());

    if (has_leaf_delta) {
        QJsonObject lhs_delta { JsonGen::NodeDelta(lhs_id_, 0.0, lhs_final_delta) };
        QJsonObject rhs_delta { JsonGen::NodeDelta(*d_shadow->rhs_node, 0.0, rhs_final_delta) };

        message.insert(kLhsDelta, lhs_delta);
        message.insert(kRhsDelta, rhs_delta);
    }

    WebSocket::Instance()->SendMessage(kEntryRate, message);

    if (has_leaf_delta) {
        emit SNodeDelta(lhs_id_, 0.0, lhs_final_delta);
        emit SNodeDelta(*d_shadow->rhs_node, 0.0, rhs_final_delta);
    }

    return true;
}

bool TableModelT::UpdateLinkedNode(EntryShadow* entry_shadow, const QUuid& value, int row)
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

    const QString old_node_id { old_node.toString(QUuid::WithoutBraces) };
    const QString new_node_id { value.toString(QUuid::WithoutBraces) };

    QJsonObject cache {};
    cache = d_shadow->WriteJson();

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section_));
    message.insert(kSessionId, QString());
    message.insert(kEntry, cache);
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));

    if (old_node.isNull()) {
        double lhs_debit { *d_shadow->lhs_debit };
        double lhs_credit { *d_shadow->lhs_credit };

        const double lhs_initial_delta { lhs_debit - lhs_credit };
        const double lhs_final_delta { unit_cost * lhs_initial_delta };

        const double rhs_initial_delta { -lhs_initial_delta };
        const double rhs_final_delta { -lhs_final_delta };

        const bool has_leaf_delta { std::abs(lhs_initial_delta) > kTolerance };

        if (has_leaf_delta) {
            QJsonObject lhs_delta { JsonGen::NodeDelta(lhs_id_, lhs_initial_delta, lhs_final_delta) };
            QJsonObject rhs_delta { JsonGen::NodeDelta(value, rhs_initial_delta, rhs_final_delta) };

            message.insert(kLhsDelta, lhs_delta);
            message.insert(kRhsDelta, rhs_delta);
        }

        if (has_leaf_delta) {
            AccumulateBalance(row);

            emit SResizeColumnToContents(std::to_underlying(EntryEnumT::kBalance));
            emit SNodeDelta(lhs_id_, lhs_initial_delta, lhs_final_delta);
            emit SNodeDelta(value, rhs_initial_delta, rhs_final_delta);
        }

        emit SAppendOneEntry(value, d_shadow->entry);

        WebSocket::Instance()->SendMessage(kEntryInsert, message);
    }

    if (!old_node.isNull()) {
        const bool is_parallel { d_shadow->is_parallel };
        const auto field { is_parallel ? kRhsNode : kLhsNode };

        const double rhs_debit { *d_shadow->rhs_debit };
        const double rhs_credit { *d_shadow->rhs_credit };

        const double rhs_initial_delta { rhs_debit - rhs_credit };
        const double rhs_final_delta { unit_cost * rhs_initial_delta };

        const bool has_leaf_delta { std::abs(rhs_initial_delta) > kTolerance };

        if (has_leaf_delta) {
            QJsonObject new_node_delta { JsonGen::NodeDelta(value, rhs_initial_delta, rhs_final_delta) };
            QJsonObject old_node_delta { JsonGen::NodeDelta(old_node, -rhs_initial_delta, -rhs_final_delta) };

            message.insert(kNewNodeDelta, new_node_delta);
            message.insert(kOldNodeDelta, old_node_delta);
        }

        if (has_leaf_delta) {
            AccumulateBalance(row);

            emit SResizeColumnToContents(std::to_underlying(EntryEnumT::kBalance));
            emit SNodeDelta(value, rhs_initial_delta, rhs_final_delta);
            emit SNodeDelta(old_node, -rhs_initial_delta, -rhs_final_delta);
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

double TableModelT::CalculateBalance(EntryShadow* entry_shadow)
{
    auto* d_shadow { DerivedPtr<EntryShadowT>(entry_shadow) };
    return (direction_rule_ == Rule::kDICD ? 1 : -1) * (*d_shadow->lhs_debit - *d_shadow->lhs_credit);
}

bool TableModelT::insertRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent));
    if (d_node_->status != std::to_underlying(NodeStatus::kRecalled))
        return false;

    auto* entry_shadow { InsertRowsImpl(row, parent) };
    *entry_shadow->issued_time = QDateTime::currentDateTimeUtc();

    emit SInsertEntry(entry_shadow->entry);
    return true;
}

bool TableModelT::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent) - 1);

    if (d_node_->status == std::to_underlying(NodeStatus::kReleased))
        return false;

    auto* d_shadow = DerivedPtr<EntryShadowT>(shadow_list_.at(row));
    const auto rhs_node_id { *d_shadow->rhs_node };

    beginRemoveRows(parent, row, row);
    shadow_list_.removeAt(row);
    endRemoveRows();

    const auto entry_id { *d_shadow->id };

    if (!rhs_node_id.isNull()) {
        const double lhs_initial_delta { *d_shadow->lhs_credit - *d_shadow->lhs_debit };
        const double lhs_final_delta { *d_shadow->unit_cost * lhs_initial_delta };

        const double rhs_initial_delta { *d_shadow->rhs_credit - *d_shadow->rhs_debit };
        const double rhs_final_delta { *d_shadow->unit_cost * rhs_initial_delta };

        const bool has_delta { std::abs(lhs_initial_delta) > kTolerance };

        QJsonObject message {};
        message.insert(kSection, std::to_underlying(section_));
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
            emit SNodeDelta(lhs_id_, lhs_initial_delta, lhs_final_delta);
            emit SNodeDelta(*d_shadow->rhs_node, rhs_initial_delta, rhs_final_delta);
            AccumulateBalance(row);
        }

        emit SRemoveOneEntry(rhs_node_id, entry_id);
    }

    EntryShadowPool::Instance().Recycle(d_shadow, section_);
    emit SRemoveEntry(entry_id);
    return true;
}

#if 0
bool LeafModelT::UpdateDebit(EntryShadow* entry_shadow, double value, int row)
{
    auto* d_shadow { DerivedPtr<EntryShadowT>(entry_shadow) };

    const double lhs_old_debit { *d_shadow->lhs_debit };
    if (FloatEqual(lhs_old_debit, value))
        return false;

    const double lhs_old_credit { *d_shadow->lhs_credit };

    const double abs { qAbs(value - lhs_old_credit) };
    *d_shadow->lhs_debit = (value > lhs_old_credit) ? abs : 0;
    *d_shadow->lhs_credit = (value <= lhs_old_credit) ? abs : 0;

    *d_shadow->rhs_debit = *d_shadow->lhs_credit;
    *d_shadow->rhs_credit = *d_shadow->lhs_debit;

    if (d_shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *d_shadow->id };
    const QUuid rhs_id { *d_shadow->rhs_node };

    QJsonObject cache {};
    const bool is_parallel { entry_shadow->is_parallel };

    cache.insert(is_parallel ? kLhsDebit : kRhsDebit, QString::number(*d_shadow->lhs_debit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kLhsCredit : kRhsCredit, QString::number(*d_shadow->lhs_credit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kRhsDebit : kLhsDebit, QString::number(*d_shadow->rhs_debit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kRhsCredit : kLhsCredit, QString::number(*d_shadow->rhs_credit, 'f', kMaxNumericScale_4));

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section_));
    message.insert(kSessionId, QString());
    message.insert(kCache, cache);
    message.insert(kIsParallel, is_parallel);
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));

    const double unit_cost { *d_shadow->unit_cost };

    const double lhs_initial_delta { *d_shadow->lhs_debit - *d_shadow->lhs_credit - (lhs_old_debit - lhs_old_credit) };
    const double lhs_final_delta { unit_cost * lhs_initial_delta };

    const double rhs_initial_delta { -lhs_initial_delta };
    const double rhs_final_delta { unit_cost * rhs_initial_delta };

    const bool has_leaf_delta { std::abs(lhs_initial_delta) > kTolerance };

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

bool LeafModelT::UpdateCredit(EntryShadow* entry_shadow, double value, int row)
{
    auto* d_shadow { DerivedPtr<EntryShadowT>(entry_shadow) };

    double lhs_old_credit { *d_shadow->lhs_credit };
    if (FloatEqual(lhs_old_credit, value))
        return false;

    const double lhs_old_debit { *d_shadow->lhs_debit };

    const double abs { qAbs(value - lhs_old_debit) };
    *d_shadow->lhs_debit = (value > lhs_old_debit) ? 0 : abs;
    *d_shadow->lhs_credit = (value <= lhs_old_debit) ? 0 : abs;

    *d_shadow->rhs_debit = *d_shadow->lhs_credit;
    *d_shadow->rhs_credit = *d_shadow->lhs_debit;

    if (d_shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *d_shadow->id };
    const QUuid rhs_id { *d_shadow->rhs_node };

    QJsonObject cache {};
    const bool is_parallel { entry_shadow->is_parallel };

    cache.insert(is_parallel ? kLhsDebit : kRhsDebit, QString::number(*d_shadow->lhs_debit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kLhsCredit : kRhsCredit, QString::number(*d_shadow->lhs_credit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kRhsDebit : kLhsDebit, QString::number(*d_shadow->rhs_debit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kRhsCredit : kLhsCredit, QString::number(*d_shadow->rhs_credit, 'f', kMaxNumericScale_4));

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section_));
    message.insert(kSessionId, QString());
    message.insert(kCache, cache);
    message.insert(kIsParallel, is_parallel);
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));

    const double unit_cost { *d_shadow->unit_cost };

    const double lhs_initial_delta { *d_shadow->lhs_debit - *d_shadow->lhs_credit - (lhs_old_debit - lhs_old_credit) };
    const double lhs_final_delta { unit_cost * lhs_initial_delta };

    const double rhs_initial_delta { -lhs_initial_delta };
    const double rhs_final_delta { unit_cost * rhs_initial_delta };

    const bool has_leaf_delta { std::abs(lhs_initial_delta) > kTolerance };

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
#endif
