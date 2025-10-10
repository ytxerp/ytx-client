#include "entryhubf.h"

#include "component/constant.h"

EntryHubF::EntryHubF(CSectionInfo& info, QObject* parent)
    : EntryHub(info, parent)
{
}

void EntryHubF::UpdateEntryRate(const QUuid& entry_id, const QJsonObject& data, bool is_parallel)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* d_entry = static_cast<EntryF*>(it.value());
        QUuid rhs_id {};
        QUuid lhs_id {};

        if (is_parallel) {
            d_entry->lhs_rate = data[kLhsRate].toString().toDouble();
            d_entry->rhs_debit = data[kRhsDebit].toString().toDouble();
            d_entry->rhs_credit = data[kRhsCredit].toString().toDouble();
            rhs_id = d_entry->rhs_node;
            lhs_id = d_entry->lhs_node;
        } else {
            d_entry->rhs_rate = data[kRhsRate].toString().toDouble();
            d_entry->lhs_debit = data[kLhsDebit].toString().toDouble();
            d_entry->lhs_credit = data[kLhsCredit].toString().toDouble();
            rhs_id = d_entry->lhs_node;
            lhs_id = d_entry->rhs_node;
        }

        d_entry->updated_time = QDateTime::fromString(data[kUpdatedTime].toString(), Qt::ISODate);
        d_entry->updated_by = QUuid(data[kUpdatedBy].toString());

        const int lhs_rate { std::to_underlying(EntryEnumF::kLhsRate) };

        emit SUpdateBalance(rhs_id, entry_id);
        emit SRefreshField(lhs_id, entry_id, lhs_rate, lhs_rate);
    }
}

void EntryHubF::UpdateEntryNumeric(const QUuid& entry_id, const QJsonObject& data, bool is_parallel)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* d_entry = static_cast<EntryF*>(it.value());
        QUuid rhs_id {};
        QUuid lhs_id {};

        if (is_parallel) {
            d_entry->lhs_debit = data[kLhsDebit].toString().toDouble();
            d_entry->lhs_credit = data[kLhsCredit].toString().toDouble();
            d_entry->rhs_debit = data[kRhsDebit].toString().toDouble();
            d_entry->rhs_credit = data[kRhsCredit].toString().toDouble();
            rhs_id = d_entry->rhs_node;
            lhs_id = d_entry->lhs_node;
        } else {
            d_entry->lhs_debit = data[kRhsDebit].toString().toDouble();
            d_entry->lhs_credit = data[kRhsCredit].toString().toDouble();
            d_entry->rhs_debit = data[kLhsDebit].toString().toDouble();
            d_entry->rhs_credit = data[kLhsCredit].toString().toDouble();
            rhs_id = d_entry->lhs_node;
            lhs_id = d_entry->rhs_node;
        }

        d_entry->updated_time = QDateTime::fromString(data[kUpdatedTime].toString(), Qt::ISODate);
        d_entry->updated_by = QUuid(data[kUpdatedBy].toString());

        emit SUpdateBalance(rhs_id, entry_id);
        emit SUpdateBalance(lhs_id, entry_id);
    }
}

// QString EntryHubF::QSLeafTotal(int /*unit*/) const
// {
//     return QStringLiteral(R"(
//     WITH node_balance AS (
//         SELECT
//             lhs_debit AS initial_debit,
//             lhs_credit AS initial_credit,
//             lhs_rate * lhs_debit AS final_debit,
//             lhs_rate * lhs_credit AS final_credit
//         FROM finance_transaction
//         WHERE lhs_node = :node_id AND is_valid = TRUE

//         UNION ALL

//         SELECT
//             rhs_debit,
//             rhs_credit,
//             rhs_rate * rhs_debit,
//             rhs_rate * rhs_credit
//         FROM finance_transaction
//         WHERE rhs_node = :node_id AND is_valid = TRUE
//     )
//     SELECT
//         SUM(initial_credit) - SUM(initial_debit) AS initial_balance,
//         SUM(final_credit) - SUM(final_debit) AS final_balance
//     FROM node_balance;
//     )");
// }
