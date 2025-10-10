#include "entryhubi.h"

#include "component/constant.h"

EntryHubI::EntryHubI(CSectionInfo& info, QObject* parent)
    : EntryHub(info, parent)
{
}

// void EntryHubI::RRemoveSupportNode(const QUuid& support_id)
// {
//     for (auto* trans : std::as_const(entry_cache_)) {
//         auto* d_trans = static_cast<EntryI*>(trans);

//         if (d_trans->support_node == support_id) {
//             d_trans->support_node = QUuid();
//         }
//     }
// }

// void EntryHubI::RemoveSupportEntry(const Entry* entry)
// {
//     auto* d_entry { static_cast<const EntryI*>(entry) };

//     if (const auto support_id { d_entry->support_node }; !support_id.isNull()) {
//         QHash<QUuid, QSet<QUuid>> support_entry {};
//         support_entry.insert(support_id, { entry->id });

//         emit SRemoveMultiEntryS( support_entry);
//     }
// }

// void EntryHubI::SupportInsert(Entry* entry)
// {
//     auto* d_entry { static_cast<const EntryI*>(entry) };

//     if (const auto support_id { d_entry->support_node }; !support_id.isNull()) {
//         emit SAppendOneEntryS( support_id, entry);
//     }
// }

// void EntryHubI::RReplaceSupportNode(const QUuid& old_support_id, const QUuid& new_support_id)
// {
//     QList<Entry*> list {};

//     for (auto* entry : std::as_const(entry_cache_)) {
//         auto* d_entry = static_cast<EntryF*>(entry);

//         if (d_entry->support_node == old_support_id) {
//             d_entry->support_node = new_support_id;
//             list.emplaceBack(entry);
//         }
//     }

//     emit SAppendMultiEntryS( new_support_id, list);
// }

// void EntryHubI::ApplyEntrySupportNode(const QUuid& entry_id, const QUuid& old_support_id, const QUuid& new_support_id, const QJsonObject& meta)
// {
//     auto it = entry_cache_.constFind(entry_id);
//     if (it != entry_cache_.constEnd()) {
//         auto* d_entry = static_cast<EntryI*>(it.value());

//         d_entry->support_node = new_support_id;
//         d_entry->updated_time = QDateTime::fromString(meta[kUpdatedTime].toString(), Qt::ISODate);
//         d_entry->updated_by = QUuid(meta[kUpdatedBy].toString());

//         emit SRemoveOneEntryS( old_support_id, entry_id);
//         emit SAppendOneEntryS( new_support_id, d_entry);

//         emit SRefreshField( d_entry->lhs_node, entry_id, EntryEnum::kSupportNode, EntryEnum::kSupportNode);
//         emit SRefreshField( d_entry->rhs_node, entry_id, EntryEnum::kSupportNode, EntryEnum::kSupportNode);
//     };
// }

void EntryHubI::UpdateEntryRate(const QUuid& entry_id, const QJsonObject& data, bool /*is_parallel*/)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* d_entry = static_cast<EntryI*>(it.value());

        d_entry->unit_cost = data[kUnitCost].toString().toDouble();
        d_entry->updated_time = QDateTime::fromString(data[kUpdatedTime].toString(), Qt::ISODate);
        d_entry->updated_by = QUuid(data[kUpdatedBy].toString());

        const int unit_cost { std::to_underlying(EntryEnumI::kUnitCost) };

        emit SRefreshField(d_entry->lhs_node, entry_id, unit_cost, unit_cost);
        emit SRefreshField(d_entry->rhs_node, entry_id, unit_cost, unit_cost);
    }
}

void EntryHubI::UpdateEntryNumeric(const QUuid& entry_id, const QJsonObject& data, bool is_parallel)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* d_entry = static_cast<EntryI*>(it.value());
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

// QString EntryHubI::QSLeafTotal(int /*unit*/) const
// {
//     return QStringLiteral(R"(
//     WITH node_balance AS (
//         SELECT
//             lhs_debit AS initial_debit,
//             lhs_credit AS initial_credit,
//             unit_cost * lhs_debit AS final_debit,
//             unit_cost * lhs_credit AS final_credit
//         FROM item_transaction
//         WHERE lhs_node = :node_id AND is_valid = TRUE

//         UNION ALL

//         SELECT
//             rhs_debit,
//             rhs_credit,
//             unit_cost * rhs_debit,
//             unit_cost * rhs_credit
//         FROM item_transaction
//         WHERE rhs_node = :node_id AND is_valid = TRUE
//     )
//     SELECT
//         SUM(initial_credit) - SUM(initial_debit) AS initial_balance,
//         SUM(final_credit) - SUM(final_debit) AS final_balance
//     FROM node_balance;
//     )");
// }

// void EntryHubI::ReadTransRefQuery(EntryRefList& list, QSqlQuery& query) const
// {
//     while (query.next()) {
//         auto* trans_ref { ResourcePool<EntryRef>::Instance().Allocate() };

//         trans_ref->section = QStringLiteral("section").toInt();
//         trans_ref->pi_id = query.value(QStringLiteral("partner")).toUuid();
//         trans_ref->unit_price = query.value(QStringLiteral("unit_price")).toDouble();
//         trans_ref->second = query.value(QStringLiteral("second")).toDouble();
//         trans_ref->description = query.value(QStringLiteral("description")).toString();
//         trans_ref->first = query.value(QStringLiteral("first")).toDouble();
//         trans_ref->initial = query.value(QStringLiteral("initial")).toDouble();
//         trans_ref->external_item = query.value(QStringLiteral("support_id")).toUuid();
//         trans_ref->issued_time = query.value(QStringLiteral("issued_time")).toDateTime();
//         trans_ref->order_id = query.value(QStringLiteral("lhs_node")).toUuid();

//         list.emplaceBack(trans_ref);
//     }
// }

QString EntryHubI::QSReadTransRef(int /*unit*/) const
{
    return QStringLiteral(R"(
    SELECT
        4 AS section,
        st.unit_price,
        st.second,
        st.lhs_node,
        st.description,
        st.first,
        st.initial,
        st.discount,
        st.final,
        st.support_id,
        st.discount_price,
        sn.partner,
        sn.issued_time
    FROM sale_transaction st
    INNER JOIN sale_node sn ON st.lhs_node = sn.id
    WHERE st.rhs_node = :node_id AND sn.is_finished = TRUE AND (sn.issued_time BETWEEN :start AND :end) AND st.is_valid = TRUE

    UNION ALL

    SELECT
        5 AS section,
        pt.unit_price,
        pt.second,
        pt.lhs_node,
        pt.description,
        pt.first,
        pt.initial,
        pt.discount,
        pt.final,
        pt.support_id,
        pt.discount_price,
        pn.partner,
        pn.issued_time
    FROM purchase_transaction pt
    INNER JOIN purchase_node pn ON pt.lhs_node = pn.id
    WHERE pt.rhs_node = :node_id AND pn.is_finished = TRUE AND (pn.issued_time BETWEEN :start AND :end) AND pt.is_valid = TRUE;
    )");
}

QString EntryHubI::QSReplaceLeafSI() const
{
    return QStringLiteral(R"(
    UPDATE stakeholder_transaction
    SET rhs_node = :new_node_id
    WHERE rhs_node = :old_node_id;
    )");
}

QString EntryHubI::QSReplaceLeafOSI() const
{
    return QStringLiteral(R"(
    UPDATE sale_transaction
    SET rhs_node = :new_node_id
    WHERE rhs_node = :old_node_id;
    )");
}

QString EntryHubI::QSReplaceLeafOPI() const
{
    return QStringLiteral(R"(
    UPDATE purchase_transaction
    SET rhs_node = :new_node_id
    WHERE rhs_node = :old_node_id;
    )");
}
