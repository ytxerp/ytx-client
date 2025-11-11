#include "entryhubp.h"

EntryHubP::EntryHubP(CSectionInfo& info, QObject* parent)
    : EntryHub(info, parent)
{
}

void EntryHubP::RemoveLeaf(const QHash<QUuid, QSet<QUuid>>& leaf_entry) { RemoveLeafFunction(leaf_entry); }

void EntryHubP::ApplyInventoryReplace(const QUuid& old_item_id, const QUuid& new_item_id) const
{
    for (auto* entry : std::as_const(entry_cache_)) {
        auto* d_entry = static_cast<EntryP*>(entry);

        if (d_entry->rhs_node == old_item_id)
            d_entry->rhs_node = new_item_id;

        if (d_entry->external_sku == old_item_id)
            d_entry->external_sku = new_item_id;
    }
}

std::optional<std::pair<QUuid, double>> EntryHubP::ResolveFromInternal(const QUuid& partner_id, const QUuid& internal_sku) const
{
    for (const auto* trans : std::as_const(entry_cache_)) {
        auto* d_trans = static_cast<const EntryP*>(trans);

        if (d_trans->lhs_node == partner_id && d_trans->rhs_node == internal_sku) {
            return std::make_pair(d_trans->external_sku, d_trans->unit_price);
        }
    }

    return std::nullopt;
}

#if 0
std::optional<std::pair<QUuid, double>> EntryHubP::ResolveFromExternal(const QUuid& partner_id, const QUuid& external_sku) const
{
    for (const auto* trans : std::as_const(entry_cache_)) {
        auto* d_trans = static_cast<const EntryP*>(trans);

        if (d_trans->lhs_node == partner_id && d_trans->external_sku == external_sku) {
            return std::make_pair(d_trans->rhs_node, d_trans->unit_price);
        }
    }

    return std::nullopt;
}
#endif

// QString EntryHubP::QSReadTransRef(int unit) const
// {
//     QString base_query = QStringLiteral(R"(
//         SELECT
//             %4 AS section,
//             %1.rhs_node,
//             %1.unit_price,
//             %1.second,
//             %1.lhs_node,
//             %1.description,
//             %1.first,
//             %1.initial,
//             %1.support_id,
//             %1.discount_price,
//             %2.issued_time
//         FROM %1
//         INNER JOIN %2 ON %1.lhs_node = %2.id
//         WHERE %2.%3 = :node_id AND %2.is_finished = TRUE AND (%2.issued_time BETWEEN :start AND :end) AND %1.is_valid = TRUE;
//     )");

//     QString query_employee = QStringLiteral(R"(
//         SELECT
//             4 AS section,
//             st.rhs_node,
//             st.unit_price,
//             st.second,
//             st.lhs_node,
//             st.description,
//             st.first,
//             st.initial,
//             st.support_id,
//             st.discount_price,
//             sn.issued_time
//         FROM sale_transaction st
//         INNER JOIN sale_node sn ON st.lhs_node = sn.id
//         WHERE sn.employee = :node_id AND sn.is_finished = TRUE AND (sn.issued_time BETWEEN :start AND :end) AND st.is_valid = TRUE

//         UNION ALL

//         SELECT
//             5 AS section,
//             pt.rhs_node,
//             pt.unit_price,
//             pt.second,
//             pt.lhs_node,
//             pt.description,
//             pt.first,
//             pt.initial,
//             pt.support_id,
//             pt.discount_price,
//             pn.issued_time
//         FROM purchase_transaction pt
//         INNER JOIN purchase_node pn ON pt.lhs_node = pn.id
//         WHERE pn.employee = :node_id AND pn.is_finished = TRUE AND (pn.issued_time BETWEEN :start AND :end) AND pt.is_valid = TRUE
//     )");

//     QString node {};
//     QString node_trans {};
//     QString column {};
//     QString section {};

//     switch (UnitP(unit)) {
//     case UnitP::kCustomer:
//         node_trans = QStringLiteral("sale_transaction");
//         node = QStringLiteral("sale_node");
//         column = QStringLiteral("partner");
//         section = QStringLiteral("4");
//         break;
//     case UnitP::kVendor:
//         node_trans = QStringLiteral("purchase_transaction");
//         node = QStringLiteral("purchase_node");
//         column = QStringLiteral("partner");
//         section = QStringLiteral("5");
//         break;
//     case UnitP::kEmployee:
//         return query_employee;
//     default:
//         return {};
//     }

//     return base_query.arg(node_trans, node, column, section);
// }
