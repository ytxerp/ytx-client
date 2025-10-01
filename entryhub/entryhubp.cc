#include "entryhubp.h"

EntryHubP::EntryHubP(CSectionInfo& info, QObject* parent)
    : EntryHub(info, parent)
{
}

void EntryHubP::RemoveLeaf(const QHash<QUuid, QSet<QUuid>>& leaf_entry) { RemoveLeafFunction(leaf_entry); }

void EntryHubP::RPriceSList(const QList<PriceS>& list)
{
    for (int i = 0; i != list.size(); ++i) {
        EntryP* latest_trans { nullptr };

        for (auto* trans : std::as_const(entry_cache_)) {
            if (trans->lhs_node == list[i].lhs_node && trans->rhs_node == list[i].rhs_node) {
                latest_trans = static_cast<EntryP*>(trans);
                break;
            }
        }

        if (latest_trans) {
            latest_trans->unit_price = list[i].unit_price;
            latest_trans->unit_price = list[i].unit_price;
            latest_trans->issued_time = list[i].issued_time;
        }
    }

    QSet<QUuid> set {};
    for (const auto& item : std::as_const(list)) {
        set.insert(item.rhs_node);
    }

    ReadTransRange(set);
}

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

bool EntryHubP::CrossSearch(EntryShadowO* order_entry_shadow, const QUuid& partner_id, const QUuid& item_id, bool is_internal) const
{
    const EntryP* latest_trans { nullptr };

    for (const auto* trans : std::as_const(entry_cache_)) {
        auto* d_trans = static_cast<const EntryP*>(trans);

        if (is_internal && d_trans->lhs_node == partner_id && d_trans->rhs_node == item_id) {
            latest_trans = d_trans;
            break;
        }

        if (!is_internal && d_trans->lhs_node == partner_id && d_trans->external_sku == item_id) {
            latest_trans = d_trans;
            break;
        }
    }

    if (!latest_trans) {
        return false;
    }

    *order_entry_shadow->unit_price = latest_trans->unit_price;

    if (is_internal) {
        *order_entry_shadow->external_sku = latest_trans->external_sku;
    } else {
        *order_entry_shadow->rhs_node = latest_trans->rhs_node;
    }

    return true;
}

// void EntryHubS::ApplyEntryRate(const QUuid& entry_id, const QJsonObject& data, bool /*is_parallel*/)
// {
//     auto it = entry_cache_.constFind(entry_id);
//     if (it != entry_cache_.constEnd()) {
//         auto* d_entry = static_cast<EntryS*>(it.value());

//         d_entry->unit_price = data[kUnitPrice].toString().toDouble();
//         d_entry->updated_time = QDateTime::fromString(data[kUpdatedTime].toString(), Qt::ISODate);
//         d_entry->updated_by = QUuid(data[kUpdatedBy].toString());

//         const int unit_price { std::to_underlying(EntryEnumS::kUnitPrice) };

//         emit SRefreshField(d_entry->lhs_node, entry_id, unit_price, unit_price);
//     }
// }

bool EntryHubP::ReadTransRange(const QSet<QUuid>& set)
{
    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // CString placeholder { QStringList(set.size(), "?").join(", ") };
    // CString sql { BuildSelect(kStakeholderEntry, QString("rhs_node IN (%1)").arg(placeholder)) };

    // query.prepare(sql);

    // for (const auto& value : set) {
    //     query.addBindValue(value);
    // }

    // if (!query.exec()) {
    //     qWarning() << "Section: " << std::to_underlying(section_) << "Failed in RetrieveTrans" << query.lastError().text();
    //     return false;
    // }

    // EntryShadowList entry_shadow_list {};

    // while (query.next()) {
    //     const auto id { query.value(QStringLiteral("id")).toUuid() };

    //     if (entry_hash_.contains(id))
    //         continue;

    //     auto* entry_shadow { EntryShadowPool::Instance().Allocate(section_) };
    //     auto* entry { EntryPool::Instance().Allocate(section_) };
    //     entry->id = id;

    //     // trans->ReadQuery(query);
    //     entry_hash_.insert(id, entry);

    //     entry_shadow->BindEntry(entry, true);
    //     entry_shadow_list.emplaceBack(entry_shadow);
    // }

    // if (!entry_shadow_list.isEmpty())
    // emit SAppendMultiEntry( *entry_shadow_list[0]->lhs_node, entry_shadow_list);

    return true;
}

QString EntryHubP::QSReadTransRef(int unit) const
{
    QString base_query = QStringLiteral(R"(
        SELECT
            %4 AS section,
            %1.rhs_node,
            %1.unit_price,
            %1.second,
            %1.lhs_node,
            %1.description,
            %1.first,
            %1.initial,
            %1.support_id,
            %1.discount_price,
            %2.issued_time
        FROM %1
        INNER JOIN %2 ON %1.lhs_node = %2.id
        WHERE %2.%3 = :node_id AND %2.is_finished = TRUE AND (%2.issued_time BETWEEN :start AND :end) AND %1.is_valid = TRUE;
    )");

    QString query_employee = QStringLiteral(R"(
        SELECT
            4 AS section,
            st.rhs_node,
            st.unit_price,
            st.second,
            st.lhs_node,
            st.description,
            st.first,
            st.initial,
            st.support_id,
            st.discount_price,
            sn.issued_time
        FROM sale_transaction st
        INNER JOIN sale_node sn ON st.lhs_node = sn.id
        WHERE sn.employee = :node_id AND sn.is_finished = TRUE AND (sn.issued_time BETWEEN :start AND :end) AND st.is_valid = TRUE

        UNION ALL

        SELECT
            5 AS section,
            pt.rhs_node,
            pt.unit_price,
            pt.second,
            pt.lhs_node,
            pt.description,
            pt.first,
            pt.initial,
            pt.support_id,
            pt.discount_price,
            pn.issued_time
        FROM purchase_transaction pt
        INNER JOIN purchase_node pn ON pt.lhs_node = pn.id
        WHERE pn.employee = :node_id AND pn.is_finished = TRUE AND (pn.issued_time BETWEEN :start AND :end) AND pt.is_valid = TRUE
    )");

    QString node {};
    QString node_trans {};
    QString column {};
    QString section {};

    switch (UnitS(unit)) {
    case UnitS::kCustomer:
        node_trans = QStringLiteral("sale_transaction");
        node = QStringLiteral("sale_node");
        column = QStringLiteral("partner");
        section = QStringLiteral("4");
        break;
    case UnitS::kVendor:
        node_trans = QStringLiteral("purchase_transaction");
        node = QStringLiteral("purchase_node");
        column = QStringLiteral("partner");
        section = QStringLiteral("5");
        break;
    case UnitS::kEmployee:
        return query_employee;
    default:
        return {};
    }

    return base_query.arg(node_trans, node, column, section);
}

// QString EntryHubS::QSLeafTotal(int unit) const
// {
//     switch (UnitS(unit)) {
//     case UnitS::kCust:
//         return QStringLiteral(R"(
//         SELECT SUM(initial) AS final_balance
//         FROM sale_node
//         WHERE partner = :node_id AND is_finished = TRUE AND settlement_id = 0 AND unit = 1 AND is_valid = TRUE;
//         )");
//         break;
//     case UnitS::kVend:
//         return QStringLiteral(R"(
//         SELECT SUM(initial) AS final_balance
//         FROM purchase_node
//         WHERE partner = :node_id AND is_finished = TRUE AND settlement_id = 0 AND unit = 1 AND is_valid = TRUE;
//         )");
//         break;
//     default:
//         break;
//     }

//     return {};
// }

// void EntryHubS::ReadTransS(QSqlQuery& query)
// {
//     Entry* trans {};

//     while (query.next()) {
//         const auto id { query.value(QStringLiteral("id")).toUuid() };

//         if (entry_hash_.contains(id))
//             continue;

//         trans = EntryPool::Instance().Allocate(section_);
//         trans->id = id;

//         // trans->ReadQuery(query);
//         entry_hash_.insert(id, trans);
//     }
// }
