#include "tablemodeli.h"

#include "component/constant.h"
#include "global/entryshadowpool.h"
#include "utils/entryutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableModelI::TableModelI(CTableModelArg& arg, QObject* parent)
    : TableModel { arg, parent }
{
}

QVariant TableModelI::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_shadow = DerivedPtr<EntryShadowI>(shadow_list_.at(index.row()));

    const EntryEnumI column { index.column() };

    switch (column) {
    case EntryEnumI::kId:
        return *d_shadow->id;
    case EntryEnumI::kUserId:
        return *d_shadow->user_id;
    case EntryEnumI::kCreateTime:
        return *d_shadow->created_time;
    case EntryEnumI::kCreateBy:
        return *d_shadow->created_by;
    case EntryEnumI::kUpdateTime:
        return *d_shadow->updated_time;
    case EntryEnumI::kUpdateBy:
        return *d_shadow->updated_by;
    case EntryEnumI::kLhsNode:
        return *d_shadow->lhs_node;
    case EntryEnumI::kIssuedTime:
        return *d_shadow->issued_time;
    case EntryEnumI::kCode:
        return *d_shadow->code;
    case EntryEnumI::kUnitCost:
        return *d_shadow->unit_cost;
    case EntryEnumI::kDescription:
        return *d_shadow->description;
    case EntryEnumI::kRhsNode:
        return *d_shadow->rhs_node;
    case EntryEnumI::kStatus:
        return *d_shadow->status;
    case EntryEnumI::kDocument:
        return *d_shadow->document;
    case EntryEnumI::kDebit:
        return *d_shadow->lhs_debit;
    case EntryEnumI::kCredit:
        return *d_shadow->lhs_credit;
    case EntryEnumI::kBalance:
        return d_shadow->balance;
    default:
        return QVariant();
    }
}

bool TableModelI::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const EntryEnumI column { index.column() };
    const int row { index.row() };

    auto* shadow { shadow_list_.at(index.row()) };
    auto* d_shadow = DerivedPtr<EntryShadowI>(shadow);

    const QUuid id { *shadow->id };

    switch (column) {
    case EntryEnumI::kIssuedTime:
        EntryUtils::UpdateShadowIssuedTime(
            pending_updates_[id], shadow, kIssuedTime, value.toDateTime(), &EntryShadow::issued_time, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumI::kCode:
        EntryUtils::UpdateShadowField(pending_updates_[id], shadow, kCode, value.toString(), &EntryShadow::code, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumI::kDocument:
        EntryUtils::UpdateShadowDocument(
            pending_updates_[id], shadow, kDocument, value.toStringList(), &EntryShadow::document, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumI::kStatus:
        EntryUtils::UpdateShadowField(pending_updates_[id], shadow, kStatus, value.toInt(), &EntryShadow::status, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumI::kDescription:
        EntryUtils::UpdateShadowField(
            pending_updates_[id], shadow, kDescription, value.toString(), &EntryShadow::description, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumI::kUnitCost:
        UpdateRate(d_shadow, value.toDouble());
        break;
    case EntryEnumI::kRhsNode:
        UpdateLinkedNode(d_shadow, value.toUuid(), row);
        break;
    case EntryEnumI::kDebit:
        UpdateNumeric(d_shadow, value.toDouble(), row, true);
        break;
    case EntryEnumI::kCredit:
        UpdateNumeric(d_shadow, value.toDouble(), row, false);
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());
    return true;
}

void TableModelI::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column <= info_.entry_header.size() - 1);

    const EntryEnumI e_column { column };

    switch (e_column) {
    case EntryEnumI::kId:
    case EntryEnumI::kBalance:
    case EntryEnumI::kUserId:
    case EntryEnumI::kCreateTime:
    case EntryEnumI::kCreateBy:
    case EntryEnumI::kUpdateTime:
    case EntryEnumI::kUpdateBy:
        return;
    default:
        break;
    }

    auto Compare = [order, e_column](EntryShadow* lhs, EntryShadow* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryShadowI>(lhs) };
        auto* d_rhs { DerivedPtr<EntryShadowI>(rhs) };

        switch (e_column) {
        case EntryEnumI::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (*lhs->issued_time < *rhs->issued_time) : (*lhs->issued_time > *rhs->issued_time);
        case EntryEnumI::kCode:
            return (order == Qt::AscendingOrder) ? (*lhs->code < *rhs->code) : (*lhs->code > *rhs->code);
        case EntryEnumI::kUnitCost:
            return (order == Qt::AscendingOrder) ? (*d_lhs->unit_cost < *d_rhs->unit_cost) : (*d_lhs->unit_cost > *d_rhs->unit_cost);
        case EntryEnumI::kDescription:
            return (order == Qt::AscendingOrder) ? (*lhs->description < *rhs->description) : (*lhs->description > *rhs->description);
        case EntryEnumI::kRhsNode:
            return (order == Qt::AscendingOrder) ? (*lhs->rhs_node < *rhs->rhs_node) : (*lhs->rhs_node > *rhs->rhs_node);
        case EntryEnumI::kStatus:
            return (order == Qt::AscendingOrder) ? (*lhs->status < *rhs->status) : (*lhs->status > *rhs->status);
        case EntryEnumI::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document->size() < rhs->document->size()) : (lhs->document->size() > rhs->document->size());
        case EntryEnumI::kDebit:
            return (order == Qt::AscendingOrder) ? (*d_lhs->lhs_debit < *d_rhs->lhs_debit) : (*d_lhs->lhs_debit > *d_rhs->lhs_debit);
        case EntryEnumI::kCredit:
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

Qt::ItemFlags TableModelI::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const EntryEnumI column { index.column() };

    switch (column) {
    case EntryEnumI::kId:
    case EntryEnumI::kBalance:
    case EntryEnumI::kDocument:
    case EntryEnumI::kStatus:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}

bool TableModelI::UpdateRate(EntryShadow* entry_shadow, double value)
{
    auto* d_shadow = DerivedPtr<EntryShadowI>(entry_shadow);

    const double unit_cost { *d_shadow->unit_cost };
    if (FloatEqual(unit_cost, value) || value < 0)
        return false;

    *d_shadow->unit_cost = value;

    if (d_shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *d_shadow->id };

    QJsonObject update {};
    update.insert(kUnitCost, QString::number(value, 'f', kMaxNumericScale_4));

    QJsonObject message { JsonGen::EntryValue(section_, entry_id, update, true) };
    WebSocket::Instance()->SendMessage(kEntryRate, message);

    return true;
}

bool TableModelI::UpdateNumeric(EntryShadow* entry_shadow, double value, int row, bool is_debit)
{
    auto* d_shadow { DerivedPtr<EntryShadowI>(entry_shadow) };

    const double lhs_old_debit { *d_shadow->lhs_debit };
    const double lhs_old_credit { *d_shadow->lhs_credit };

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

    QJsonObject update {};
    const bool is_parallel { entry_shadow->is_parallel };

    update.insert(is_parallel ? kLhsDebit : kRhsDebit, QString::number(*d_shadow->lhs_debit, 'f', kMaxNumericScale_4));
    update.insert(is_parallel ? kLhsCredit : kRhsCredit, QString::number(*d_shadow->lhs_credit, 'f', kMaxNumericScale_4));
    update.insert(is_parallel ? kRhsDebit : kLhsDebit, QString::number(*d_shadow->rhs_debit, 'f', kMaxNumericScale_4));
    update.insert(is_parallel ? kRhsCredit : kLhsCredit, QString::number(*d_shadow->rhs_credit, 'f', kMaxNumericScale_4));

    QJsonObject message { JsonGen::EntryValue(section_, entry_id, update, is_parallel) };
    WebSocket::Instance()->SendMessage(kEntryNumeric, message);

    // Delta calculation follows the DICD rule (Debit - Credit).
    // After the delta is computed, both the node and the server
    // will adjust the delta value according to the node's direction rule
    // (DICD → unchanged, DDCI → inverted).
    // The right-hand side (RHS) node must always mirror the left-hand side (LHS),
    // therefore its delta is the opposite of LHS delta.
    // This ensures overall balance (LHS + RHS = 0).
    const double lhs_initial_delta { *d_shadow->lhs_debit - *d_shadow->lhs_credit - (lhs_old_debit - lhs_old_credit) };
    const bool has_leaf_delta { std::abs(lhs_initial_delta) > kTolerance };

    if (has_leaf_delta) {
        AccumulateBalance(row);

        emit SResizeColumnToContents(std::to_underlying(EntryEnumI::kBalance));
        emit SUpdateBalance(rhs_id, *d_shadow->id);
    }

    return true;
}

#if 0
bool LeafModelI::UpdateDebit(EntryShadow* entry_shadow, double value, int row)
{
    auto* d_shadow = DerivedPtr<EntryShadowI>(entry_shadow);

    double lhs_debit { *d_shadow->lhs_debit };
    if (FloatEqual(lhs_debit, value))
        return false;

    const double lhs_credit { *d_shadow->lhs_credit };

    const double abs { qAbs(value - lhs_credit) };
    *d_shadow->lhs_debit = (value > lhs_credit) ? abs : 0;
    *d_shadow->lhs_credit = (value <= lhs_credit) ? abs : 0;

    *d_shadow->rhs_debit = *d_shadow->lhs_credit;
    *d_shadow->rhs_credit = *d_shadow->lhs_debit;

    if (d_shadow->rhs_node->isNull())
        return false;

    const double unit_cost { *d_shadow->unit_cost };
    const double quantity_debit_delta { *d_shadow->lhs_debit - lhs_debit };
    const double quantity_credit_delta { *d_shadow->lhs_credit - lhs_credit };
    const double amount_debit_delta { quantity_debit_delta * unit_cost };
    const double amount_credit_delta { quantity_credit_delta * unit_cost };

    emit SSyncDelta(lhs_id_, quantity_debit_delta, quantity_credit_delta, amount_debit_delta, amount_credit_delta);
    emit SSyncDelta(*d_shadow->rhs_node, quantity_credit_delta, quantity_debit_delta, amount_credit_delta, amount_debit_delta);

    return true;
}

bool LeafModelI::UpdateCredit(EntryShadow* entry_shadow, double value, int row)
{
    auto* d_shadow = DerivedPtr<EntryShadowI>(entry_shadow);

    double lhs_credit { *d_shadow->lhs_credit };
    if (FloatEqual(lhs_credit, value))
        return false;

    const double lhs_debit { *d_shadow->lhs_debit };

    const double abs { qAbs(value - lhs_debit) };
    *d_shadow->lhs_debit = (value > lhs_debit) ? 0 : abs;
    *d_shadow->lhs_credit = (value <= lhs_debit) ? 0 : abs;

    *d_shadow->rhs_debit = *d_shadow->lhs_credit;
    *d_shadow->rhs_credit = *d_shadow->lhs_debit;

    if (d_shadow->rhs_node->isNull())
        return false;

    const double unit_cost { *d_shadow->unit_cost };
    const double quantity_debit_delta { *d_shadow->lhs_debit - lhs_debit };
    const double quantity_credit_delta { *d_shadow->lhs_credit - lhs_credit };
    const double amount_debit_delta { quantity_debit_delta * unit_cost };
    const double amount_credit_delta { quantity_credit_delta * unit_cost };

    emit SSyncDelta(lhs_id_, quantity_debit_delta, quantity_credit_delta, amount_debit_delta, amount_credit_delta);
    emit SSyncDelta(*d_shadow->rhs_node, quantity_credit_delta, quantity_debit_delta, amount_credit_delta, amount_debit_delta);

    return true;
}
#endif

bool TableModelI::UpdateLinkedNode(EntryShadow* entry_shadow, const QUuid& value, int row)
{
    if (value.isNull())
        return false;

    auto* d_shadow = DerivedPtr<EntryShadowI>(entry_shadow);
    auto old_node { *d_shadow->rhs_node };
    if (old_node == value)
        return false;

    *d_shadow->rhs_node = value;

    const QUuid entry_id { *d_shadow->id };
    const double unit_cost { *d_shadow->unit_cost };

    const QString old_node_id { old_node.toString(QUuid::WithoutBraces) };
    const QString new_node_id { value.toString(QUuid::WithoutBraces) };

    QJsonObject message { JsonGen::EntryLinkedNode(section_, entry_id) };

    if (old_node.isNull()) {
        message.insert(kEntry, d_shadow->WriteJson());

        const double lhs_debit { *d_shadow->lhs_debit };
        const double lhs_credit { *d_shadow->lhs_credit };

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

        WebSocket::Instance()->SendMessage(kEntryInsert, message);

        if (has_leaf_delta) {
            AccumulateBalance(row);

            emit SResizeColumnToContents(std::to_underlying(EntryEnumI::kBalance));
            emit SNodeDelta(lhs_id_, lhs_initial_delta, lhs_final_delta);
            emit SNodeDelta(value, rhs_initial_delta, rhs_final_delta);
        }

        emit SAppendOneEntry(value, d_shadow->entry);
    }

    if (!old_node.isNull()) {
        const bool is_parallel { d_shadow->is_parallel };
        const auto field { is_parallel ? kRhsNode : kLhsNode };

        QJsonObject update {};
        update.insert(field, value.toString(QUuid::WithoutBraces));

        message.insert(kUpdate, update);
        message.insert(kIsParallel, is_parallel);

        WebSocket::Instance()->SendMessage(kEntryLinkedNode, message);

        emit SRemoveOneEntry(old_node, entry_id);
        emit SAppendOneEntry(value, d_shadow->entry);
    }

    return true;
}

double TableModelI::CalculateBalance(EntryShadow* entry_shadow)
{
    auto* d_shadow { DerivedPtr<EntryShadowI>(entry_shadow) };
    return (direction_rule_ == Rule::kDICD ? 1 : -1) * (*d_shadow->lhs_debit - *d_shadow->lhs_credit);
}

bool TableModelI::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent) - 1);

    auto* d_shadow = DerivedPtr<EntryShadowI>(shadow_list_.at(row));
    const auto rhs_node_id { *d_shadow->rhs_node };

    beginRemoveRows(parent, row, row);
    shadow_list_.removeAt(row);
    endRemoveRows();

    const auto entry_id { *d_shadow->id };

    if (!rhs_node_id.isNull()) {
        QJsonObject message { JsonGen::EntryRemove(section_, entry_id) };
        WebSocket::Instance()->SendMessage(kEntryRemove, message);

        const double lhs_initial_delta { *d_shadow->lhs_credit - *d_shadow->lhs_debit };
        const bool has_delta { std::abs(lhs_initial_delta) > kTolerance };

        if (has_delta) {
            AccumulateBalance(row);
        }

        emit SRemoveOneEntry(rhs_node_id, entry_id);
    }

    EntryShadowPool::Instance().Recycle(d_shadow, section_);
    emit SRemoveEntry(entry_id);
    return true;
}
