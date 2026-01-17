#include "tablemodeli.h"

#include "component/constant.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableModelI::TableModelI(CTableModelArg& arg, QObject* parent)
    : TableModel { arg, parent }
{
}

bool TableModelI::insertRows(int row, int /*count*/, const QModelIndex& parent)
{
    Q_ASSERT(row >= 0 && row <= rowCount(parent));

    InsertRowsImpl(row, parent);

    if (shadow_list_.size() == 1)
        emit SResizeColumnToContents(std::to_underlying(EntryEnum::kIssuedTime));

    return true;
}

bool TableModelI::UpdateRate(EntryShadow* shadow, double value)
{
    const double unit_cost { *shadow->lhs_rate };
    if (FloatEqual(unit_cost, value) || value < 0)
        return false;

    *shadow->lhs_rate = value;
    *shadow->rhs_rate = value;

    if (shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *shadow->id };

    QJsonObject update {};
    update.insert(kLhsRate, QString::number(value, 'f', kMaxNumericScale_8));
    update.insert(kRhsRate, QString::number(value, 'f', kMaxNumericScale_8));

    QJsonObject message { JsonGen::EntryValue(section_, entry_id, update, shadow->is_parallel) };
    WebSocket::Instance()->SendMessage(kEntryRate, message);

    return true;
}

bool TableModelI::UpdateNumeric(EntryShadow* shadow, double value, int row, bool is_debit)
{
    const double lhs_old_debit { *shadow->lhs_debit };
    const double lhs_old_credit { *shadow->lhs_credit };

    const double old_value { is_debit ? lhs_old_debit : lhs_old_credit };
    if (FloatEqual(old_value, value))
        return false;

    // Base represents the opposite side (used to compute the new diff)
    const double base { is_debit ? lhs_old_credit : lhs_old_debit };
    const double diff { std::abs(value - base) };

    // Determine which side (debit/credit) should hold the new value
    const bool to_debit { (is_debit && value > base) || (!is_debit && value <= base) };

    *shadow->lhs_debit = to_debit ? diff : 0.0;
    *shadow->lhs_credit = to_debit ? 0.0 : diff;

    // Mirror to RHS (Debit ↔ Credit)
    *shadow->rhs_debit = *shadow->lhs_credit;
    *shadow->rhs_credit = *shadow->lhs_debit;

    if (shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *shadow->id };
    const QUuid rhs_id { *shadow->rhs_node };

    QJsonObject update {};
    const bool is_parallel { shadow->is_parallel };

    update.insert(is_parallel ? kLhsDebit : kRhsDebit, QString::number(*shadow->lhs_debit, 'f', kMaxNumericScale_8));
    update.insert(is_parallel ? kLhsCredit : kRhsCredit, QString::number(*shadow->lhs_credit, 'f', kMaxNumericScale_8));
    update.insert(is_parallel ? kRhsDebit : kLhsDebit, QString::number(*shadow->rhs_debit, 'f', kMaxNumericScale_8));
    update.insert(is_parallel ? kRhsCredit : kLhsCredit, QString::number(*shadow->rhs_credit, 'f', kMaxNumericScale_8));

    QJsonObject message { JsonGen::EntryValue(section_, entry_id, update, is_parallel) };
    WebSocket::Instance()->SendMessage(kEntryNumeric, message);

    // Delta calculation follows the DICD rule (Debit - Credit).
    // After the delta is computed, both the node and the server
    // will adjust the delta value according to the node's direction rule
    // (DICD → unchanged, DDCI → inverted).
    // The right-hand side (RHS) node must always mirror the left-hand side (LHS),
    // therefore its delta is the opposite of LHS delta.
    // This ensures overall balance (LHS + RHS = 0).
    const double lhs_initial_delta { *shadow->lhs_debit - *shadow->lhs_credit - (lhs_old_debit - lhs_old_credit) };
    const bool has_leaf_delta { std::abs(lhs_initial_delta) > kTolerance };

    if (has_leaf_delta) {
        AccumulateBalance(row);

        emit SResizeColumnToContents(std::to_underlying(EntryEnum::kBalance));
        emit SUpdateBalance(rhs_id, *shadow->id);
    }

    return true;
}

bool TableModelI::UpdateLinkedNode(EntryShadow* shadow, const QUuid& value, int row)
{
    if (value.isNull())
        return false;

    auto old_node { *shadow->rhs_node };
    if (old_node == value)
        return false;

    *shadow->rhs_node = value;

    const QUuid entry_id { *shadow->id };

    QJsonObject message { JsonGen::EntryLinkedNode(section_, entry_id) };

    if (old_node.isNull()) {
        message.insert(kEntry, shadow->WriteJson());
        WebSocket::Instance()->SendMessage(kEntryInsert, message);

        const double lhs_debit { *shadow->lhs_debit };
        const double lhs_credit { *shadow->lhs_credit };

        const bool has_leaf_delta { std::abs(lhs_debit - lhs_credit) > kTolerance };
        if (has_leaf_delta) {
            AccumulateBalance(row);
            emit SResizeColumnToContents(std::to_underlying(EntryEnum::kBalance));
        }

        emit SAppendOneEntry(shadow->entry);
    }

    if (!old_node.isNull()) {
        const bool is_parallel { shadow->is_parallel };
        const auto field { is_parallel ? kRhsNode : kLhsNode };

        QJsonObject update {};
        update.insert(field, value.toString(QUuid::WithoutBraces));

        message.insert(kUpdate, update);
        message.insert(kIsParallel, is_parallel);

        WebSocket::Instance()->SendMessage(kEntryLinkedNode, message);

        emit SAttachOneEntry(value, shadow->entry);
        emit SDetachOneEntry(old_node, entry_id);
    }

    return true;
}
