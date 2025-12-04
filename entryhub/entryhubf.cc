#include "entryhubf.h"

#include "component/constant.h"
#include "enum/entryenum.h"

EntryHubF::EntryHubF(CSectionInfo& info, QObject* parent)
    : EntryHub(info, parent)
{
}

void EntryHubF::UpdateEntryRate(const QUuid& entry_id, const QJsonObject& update, bool is_parallel)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* d_entry = static_cast<EntryF*>(it.value());
        QUuid rhs_id {};
        QUuid lhs_id {};

        if (is_parallel) {
            d_entry->lhs_rate = update[kLhsRate].toString().toDouble();
            d_entry->rhs_debit = update[kRhsDebit].toString().toDouble();
            d_entry->rhs_credit = update[kRhsCredit].toString().toDouble();
            rhs_id = d_entry->rhs_node;
            lhs_id = d_entry->lhs_node;
        } else {
            d_entry->rhs_rate = update[kRhsRate].toString().toDouble();
            d_entry->lhs_debit = update[kLhsDebit].toString().toDouble();
            d_entry->lhs_credit = update[kLhsCredit].toString().toDouble();
            rhs_id = d_entry->lhs_node;
            lhs_id = d_entry->rhs_node;
        }

        const int lhs_rate { std::to_underlying(EntryEnumF::kLhsRate) };

        emit SUpdateBalance(rhs_id, entry_id);
        emit SRefreshField(lhs_id, entry_id, lhs_rate, lhs_rate);
    }
}

void EntryHubF::UpdateEntryNumeric(const QUuid& entry_id, const QJsonObject& update)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* d_entry = static_cast<EntryF*>(it.value());

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
