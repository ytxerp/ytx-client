#include "entryhubt.h"

#include "component/constant.h"

EntryHubT::EntryHubT(CSectionInfo& info, QObject* parent)
    : EntryHub(info, parent)
{
}

void EntryHubT::UpdateEntryRate(const QUuid& entry_id, const QJsonObject& data, bool /*is_parallel*/)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* d_entry = static_cast<EntryI*>(it.value());

        d_entry->unit_cost = data[kUnitCost].toString().toDouble();
        d_entry->updated_time = QDateTime::fromString(data[kUpdatedTime].toString(), Qt::ISODate);
        d_entry->updated_by = QUuid(data[kUpdatedBy].toString());

        const int unit_cost { std::to_underlying(EntryEnumT::kUnitCost) };

        emit SRefreshField(d_entry->lhs_node, entry_id, unit_cost, unit_cost);
        emit SRefreshField(d_entry->rhs_node, entry_id, unit_cost, unit_cost);
    }
}

void EntryHubT::UpdateEntryNumeric(const QUuid& entry_id, const QJsonObject& data, bool is_parallel)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* d_entry = static_cast<EntryT*>(it.value());
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

        const auto [debit, balance] = NumericColumnRange();

        emit SRefreshField(lhs_id, entry_id, debit, balance);
        emit SRefreshField(rhs_id, entry_id, debit, balance);
    }
}
