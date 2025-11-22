#include "entryhubt.h"

#include "component/constant.h"

EntryHubT::EntryHubT(CSectionInfo& info, QObject* parent)
    : EntryHub(info, parent)
{
}

void EntryHubT::UpdateEntryRate(const QUuid& entry_id, const QJsonObject& update, bool /*is_parallel*/)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* d_entry = static_cast<EntryI*>(it.value());

        d_entry->unit_cost = update[kUnitCost].toString().toDouble();

        const int unit_cost { std::to_underlying(EntryEnumT::kUnitCost) };

        emit SRefreshField(d_entry->lhs_node, entry_id, unit_cost, unit_cost);
        emit SRefreshField(d_entry->rhs_node, entry_id, unit_cost, unit_cost);
    }
}

void EntryHubT::UpdateEntryNumeric(const QUuid& entry_id, const QJsonObject& update)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* d_entry = static_cast<EntryT*>(it.value());

        d_entry->lhs_debit = update[kLhsDebit].toString().toDouble();
        d_entry->lhs_credit = update[kLhsCredit].toString().toDouble();
        d_entry->rhs_debit = update[kRhsDebit].toString().toDouble();
        d_entry->rhs_credit = update[kRhsCredit].toString().toDouble();

        QUuid rhs_id { d_entry->rhs_node };
        QUuid lhs_id { d_entry->lhs_node };

        emit SUpdateBalance(rhs_id, entry_id);
        emit SUpdateBalance(lhs_id, entry_id);
    }
}
