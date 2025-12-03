#include "entryhubp.h"

#include "global/entrypool.h"
#include "utils/entryutils.h"

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

void EntryHubP::SearchEntry(QList<Entry*>& entry_list, CString& name) const
{
    entry_list.reserve(entry_cache_.size() / 2);

    for (const auto& [id, entry] : entry_cache_.asKeyValueRange()) {
        if (!entry) {
            qCritical().noquote() << "[EntryHubP::SearchEntry] Unexpected null entry detected!"
                                  << "id =" << id.toString(QUuid::WithoutBraces);
            Q_ASSERT(entry && "EntryHubP::SearchEntry encountered null entry in cache");
            continue;
        }

        if (entry->description.contains(name, Qt::CaseInsensitive)) {
            entry_list.emplaceBack(entry);
        }
    }
}

void EntryHubP::PushEntry(const QUuid& node_id)
{
    EntryList entry_list {};

    for (const auto& [id, entry] : entry_cache_.asKeyValueRange()) {
        if (!entry) {
            qCritical().noquote() << "[EntryHubP::PushEntry] Unexpected null entry detected!"
                                  << "id =" << id.toString(QUuid::WithoutBraces);
            Q_ASSERT(entry && "EntryHubP::PushEntry encountered null entry in cache");
            continue;
        }

        if (entry->lhs_node == node_id) {
            entry_list.emplaceBack(entry);
        }
    }

    emit SAppendMultiEntry(node_id, entry_list);
}

void EntryHubP::InsertEntry(const QJsonObject& data)
{
    auto* entry = EntryPool::Instance().Allocate(section_);
    entry->ReadJson(data);

    entry_cache_.insert(entry->id, entry);

    emit SAppendOneEntry(entry->lhs_node, entry);
}

void EntryHubP::RemoveEntry(const QUuid& entry_id)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* entry = it.value();

        emit SRemoveOneEntry(entry->lhs_node, entry_id);

        EntryPool::Instance().Recycle(entry, section_);
    }
}

void EntryHubP::UpdateEntry(const QUuid& id, const QJsonObject& update)
{
    auto it = entry_cache_.constFind(id);
    if (it != entry_cache_.constEnd()) {
        auto* entry = it.value();

        entry->ReadJson(update);

        const auto [start, end] = EntryUtils::CacheColumnRange(section_);
        emit SRefreshField(entry->lhs_node, id, start, end);
    };
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

QString EntryHubP::QSReadTransRef(int unit) const
{
    QString base_query = QStringLiteral(R"(
        SELECT
            4 AS section,
            se.rhs_node,
            se.unit_price,
            se.second,
            se.lhs_node,
            se.description,
            se.first,
            se.initial,
            se.support_id,
            se.discount_price,
            sn.issued_time
        FROM sale_entry se
        INNER JOIN sale_node sn ON se.lhs_node = sn.id
        WHERE sn.partner = :node_id AND sn.status = TRUE AND (sn.issued_time BETWEEN :start AND :end) AND se.is_valid = TRUE
        ORDER BY sn.issued_time DESC
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
        FROM sale_entry st
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
        FROM purchase_entry pt
        INNER JOIN purchase_node pn ON pt.lhs_node = pn.id
        WHERE pn.employee = :node_id AND pn.is_finished = TRUE AND (pn.issued_time BETWEEN :start AND :end) AND pt.is_valid = TRUE
    )");

    // QString node {};
    // QString node_trans {};
    // QString column {};
    // QString section {};

    // switch (UnitP(unit)) {
    // case UnitP::kCustomer:
    //     node_trans = QStringLiteral("sale_entry");
    //     node = QStringLiteral("sale_node");
    //     column = QStringLiteral("partner");
    //     section = QStringLiteral("4");
    //     break;
    // case UnitP::kVendor:
    //     node_trans = QStringLiteral("purchase_entry");
    //     node = QStringLiteral("purchase_node");
    //     column = QStringLiteral("partner");
    //     section = QStringLiteral("5");
    //     break;
    // case UnitP::kEmployee:
    //     return query_employee;
    // default:
    //     return {};
    // }

    // return base_query.arg(node_trans, node, column, section);

    return {};
}
