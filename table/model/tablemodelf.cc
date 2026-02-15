#include "tablemodelf.h"

#include "component/constant.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableModelF::TableModelF(CTableModelArg& arg, QObject* parent)
    : TableModel { arg, parent }
{
}

bool TableModelF::insertRows(int row, int /*count*/, const QModelIndex& parent)
{
    Q_ASSERT(row >= 0 && row <= rowCount(parent));

    auto* entry_shadow { InsertRowsImpl(row, parent) };

    *entry_shadow->lhs_rate = 1.0;
    *entry_shadow->rhs_rate = 1.0;

    if (shadow_list_.size() == 1)
        EmitDataChanged(0, 0, std::to_underlying(EntryEnum::kIssuedTime), std::to_underlying(EntryEnum::kIssuedTime));

    return true;
}

bool TableModelF::UpdateLinkedNode(EntryShadow* shadow, const QUuid& value, int row)
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
            EmitDataChanged(row, row, std::to_underlying(EntryEnum::kBalance), std::to_underlying(EntryEnum::kBalance));
        }

        emit SAppendOneEntry(shadow->entry);
    }

    if (!old_node.isNull()) {
        // Indicates whether the EntryShadow's lhs_node corresponds to the Entry's lhs_node.
        // If true, the node is not collapsed; if false, it has been collapsed and flipped.
        const bool is_parallel { shadow->is_parallel };
        // If true, the EntryShadow corresponds to the left-side node in the Postgres table,
        // so we need to update the right-side node (kRhsNode).
        // If false, it means the entry is collapsed (lhs and rhs flipped), so update the left-side node (kLhsNode).
        const auto field { is_parallel ? kRhsNode : kLhsNode };

        QJsonObject update {};
        update.insert(field, value.toString(QUuid::WithoutBraces));

        message.insert(kUpdate, update);
        message.insert(kIsParallel, is_parallel);

        WebSocket::Instance()->SendMessage(kEntryLinkedNode, message);

        emit SDetachOneEntry(old_node, entry_id);
        emit SAttachOneEntry(value, shadow->entry);
    }

    return true;
}

bool TableModelF::UpdateNumeric(EntryShadow* shadow, double value, int row, bool is_debit)
{
    const double lhs_old_debit { *shadow->lhs_debit };
    const double lhs_old_credit { *shadow->lhs_credit };
    const double lhs_rate { *shadow->lhs_rate };
    const double rhs_rate { *shadow->rhs_rate };

    Q_ASSERT(lhs_rate != 0.0);
    Q_ASSERT(rhs_rate != 0.0);

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

    // Cauculate RHS
    *shadow->rhs_debit = (*shadow->lhs_credit) * lhs_rate / rhs_rate;
    *shadow->rhs_credit = (*shadow->lhs_debit) * lhs_rate / rhs_rate;

    if (shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *shadow->id };
    const QUuid rhs_id { *shadow->rhs_node };

    QJsonObject update {};
    const bool is_parallel { shadow->is_parallel };

    update.insert(is_parallel ? kLhsDebit : kRhsDebit, QString::number(*shadow->lhs_debit, 'f', kMaxNumericScale_4));
    update.insert(is_parallel ? kLhsCredit : kRhsCredit, QString::number(*shadow->lhs_credit, 'f', kMaxNumericScale_4));

    QJsonObject message { JsonGen::EntryValue(section_, entry_id, update, is_parallel) };

    // Delta calculation follows the DICD rule (Debit - Credit).
    // After the delta is computed, both the node and the server
    // will adjust the delta value according to the node's direction rule
    // (DICD → unchanged, DDCI → inverted).
    const double lhs_initial_delta { *shadow->lhs_debit - *shadow->lhs_credit - (lhs_old_debit - lhs_old_credit) };
    const bool has_leaf_delta { std::abs(lhs_initial_delta) > kTolerance };

    WebSocket::Instance()->SendMessage(kEntryNumeric, message);

    if (has_leaf_delta) {
        AccumulateBalance(row);
        EmitDataChanged(row, row, std::to_underlying(EntryEnum::kBalance), std::to_underlying(EntryEnum::kBalance));

        emit SUpdateBalance(rhs_id, *shadow->id);
    }

    return true;
}

#if 0
bool LeafModelF::UpdateDebit(EntryShadow* shadow, double value, int row)
{
    auto* shadow { DerivedPtr<EntryShadowF>(shadow) };

    const double lhs_debit { *shadow->lhs_debit };
    if (FloatEqual(lhs_debit, value))
        return false;

    const double lhs_credit { *shadow->lhs_credit };
    const double lhs_rate { *shadow->lhs_rate };

    const double abs { qAbs(value - lhs_credit) };
    *shadow->lhs_debit = (value > lhs_credit) ? abs : 0;
    *shadow->lhs_credit = (value <= lhs_credit) ? abs : 0;

    const double rhs_debit { *shadow->rhs_debit };
    const double rhs_credit { *shadow->rhs_credit };
    const double rhs_rate { *shadow->rhs_rate };

    *shadow->rhs_debit = (*shadow->lhs_credit) * lhs_rate / rhs_rate;
    *shadow->rhs_credit = (*shadow->lhs_debit) * lhs_rate / rhs_rate;

    if (shadow->rhs_node->isNull())
        return false;

    const double lhs_debit_delta { *shadow->lhs_debit - lhs_debit };
    const double lhs_credit_delta { *shadow->lhs_credit - lhs_credit };
    emit SSyncDelta(lhs_id_, lhs_debit_delta, lhs_credit_delta, lhs_debit_delta * lhs_rate, lhs_credit_delta * lhs_rate);

    const double rhs_debit_delta { *shadow->rhs_debit - rhs_debit };
    const double rhs_credit_delta { *shadow->rhs_credit - rhs_credit };
    emit SSyncDelta(*shadow->rhs_node, rhs_debit_delta, rhs_credit_delta, rhs_debit_delta * rhs_rate, rhs_credit_delta * rhs_rate);

    return true;
}

bool LeafModelF::UpdateCredit(EntryShadow* shadow, double value, int row)
{
    auto* shadow { DerivedPtr<EntryShadowF>(shadow) };

    const double lhs_credit { *shadow->lhs_credit };
    if (FloatEqual(lhs_credit, value))
        return false;

    const double lhs_debit { *shadow->lhs_debit };
    const double lhs_rate { *shadow->lhs_rate };

    const double abs { qAbs(value - lhs_debit) };
    *shadow->lhs_debit = (value > lhs_debit) ? 0 : abs;
    *shadow->lhs_credit = (value <= lhs_debit) ? 0 : abs;

    const double rhs_debit { *shadow->rhs_debit };
    const double rhs_credit { *shadow->rhs_credit };
    const double rhs_rate { *shadow->rhs_rate };

    *shadow->rhs_debit = (*shadow->lhs_credit) * lhs_rate / rhs_rate;
    *shadow->rhs_credit = (*shadow->lhs_debit) * lhs_rate / rhs_rate;

    if (shadow->rhs_node->isNull())
        return false;

    const double lhs_debit_delta { *shadow->lhs_debit - lhs_debit };
    const double lhs_credit_delta { *shadow->lhs_credit - lhs_credit };
    emit SSyncDelta(lhs_id_, lhs_debit_delta, lhs_credit_delta, lhs_debit_delta * lhs_rate, lhs_credit_delta * lhs_rate);

    const double rhs_debit_delta { *shadow->rhs_debit - rhs_debit };
    const double rhs_credit_delta { *shadow->rhs_credit - rhs_credit };
    emit SSyncDelta(*shadow->rhs_node, rhs_debit_delta, rhs_credit_delta, rhs_debit_delta * rhs_rate, rhs_credit_delta * rhs_rate);

    return true;
}
#endif

bool TableModelF::UpdateRate(EntryShadow* shadow, double value)
{
    const double old_rate { *shadow->lhs_rate };
    if (FloatEqual(old_rate, value) || value <= 0)
        return false;

    const double proportion { value / old_rate };

    *shadow->lhs_rate = value;

    const double rhs_old_debit { *shadow->rhs_debit };
    const double rhs_old_credit { *shadow->rhs_credit };

    *shadow->rhs_debit *= proportion;
    *shadow->rhs_credit *= proportion;

    if (shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *shadow->id };

    QJsonObject update {};

    const bool is_parallel { shadow->is_parallel };
    update.insert(is_parallel ? kLhsRate : kRhsRate, QString::number(*shadow->lhs_rate, 'f', kMaxNumericScale_8));

    QJsonObject message { JsonGen::EntryValue(section_, entry_id, update, is_parallel) };
    WebSocket::Instance()->SendMessage(kEntryRate, message);

    const double rhs_initial_delta { *shadow->rhs_debit - *shadow->rhs_credit - (rhs_old_debit - rhs_old_credit) };
    const bool has_leaf_delta { std::abs(rhs_initial_delta) > kTolerance };

    if (has_leaf_delta) {
        const QUuid rhs_id { *shadow->rhs_node };
        emit SUpdateBalance(rhs_id, *shadow->id);
    }

    return true;
}
