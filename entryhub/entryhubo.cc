#include "entryhubo.h"

#include <QDate>
#include <QJsonArray>

#include "component/using.h"
#include "enum/nodeenum.h"
#include "global/entrypool.h"

EntryHubO::EntryHubO(CSectionInfo& info, QObject* parent)
    : EntryHub(info, parent)
{
}

void EntryHubO::RemoveLeaf(const QHash<QUuid, QSet<QUuid>>& leaf_entry) { RemoveLeafFunction(leaf_entry); }

/**
 * @brief Convert a JSON array of entries (from server) into an EntryList.
 *
 * This function iterates over the provided QJsonArray, creates Entry objects
 * allocated from the EntryPool according to the current section_, loads data
 * using Entry::ReadJson(), and finally returns them as an EntryList.
 *
 * **Design Notes**
 * 1. For the Order section:
 *    - The entries are managed by TableModelO.
 *
 * 2. For the Search function:
 *    - The entries are managed by SearchEntryModelO.
 *
 * 3. Entries fetched from the server are treated as **snapshot data**:
 *    - Once fetched, they are not affected by any external modifications.
 *    - Any changes can only be reflected by **fetching the data again from the server**.
 *
 * @param array The JSON array of entries retrieved from the server.
 * @return EntryList The constructed list of entries.
 */
EntryList EntryHubO::ProcessEntryArray(const QJsonArray& array)
{
    EntryList list {};

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        Entry* entry { EntryPool::Instance().Allocate(section_) };
        entry->ReadJson(obj);

        list.emplaceBack(entry);
    }

    return list;
}

QString EntryHubO::QSReadBalance(int unit) const
{
    switch (UnitO(unit)) {
    case UnitO::kMonthly:
        return QString(R"(
                SELECT

                    SUM(CASE WHEN o.issued_time < :start AND o.settlement = 0 THEN o.initial_total                 ELSE 0 END) AS pbalance,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end THEN o.initial_total                          ELSE 0 END) AS cgross_amount,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end AND o.settlement != 0 THEN o.initial_total ELSE 0 END) AS csettlement

                FROM partner_node p
                INNER JOIN %1 o ON p.id = o.partner
                WHERE o.partner = :partner_id AND o.unit = 1 AND o.status = TRUE AND o.is_valid = TRUE
            )")
            .arg(info_.node);
    case UnitO::kPending:
        return QString(R"(
                SELECT

                    SUM(CASE WHEN o.issued_time < :start THEN o.initial_total                 ELSE 0 END) AS pbalance,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end THEN o.initial_total  ELSE 0 END) AS cgross_amount,
                    0 AS csettlement

                FROM partner_node p
                INNER JOIN %1 o ON p.id = o.partner
                WHERE o.partner = :partner_id AND o.unit = 2 AND o.is_valid = TRUE
            )")
            .arg(info_.node);
    default:
        return {};
    }
}

QString EntryHubO::QSReadStatementSecondary(int unit) const
{
    static const QString kBaseQuery = R"(
        SELECT
            trans.rhs_node,
            trans.unit_price,
            trans.measure,
            trans.description,
            trans.count,
            trans.initial,
            trans.support_id,
            node.issued_time
        FROM %2 trans
        INNER JOIN %1 node ON trans.lhs_node = node.id
        WHERE node.unit = :unit AND node.partner = :partner AND (node.issued_time BETWEEN :start AND :end) %3 AND trans.is_valid = TRUE
    )";

    QString status_condition {};

    switch (UnitO(unit)) {
    case UnitO::kImmediate:
    case UnitO::kMonthly:
        status_condition = QStringLiteral("AND node.status = TRUE");
        break;
    case UnitO::kPending:
        status_condition = QLatin1String("");
        break;
    default:
        return {};
    }

    return kBaseQuery.arg(info_.node, info_.entry, status_condition);
}

QString EntryHubO::QSWriteSettlement() const
{
    return QString(R"(
    INSERT INTO %1 (id, issued_time)
    VALUES (:id, :issued_time)
    )")
        .arg(info_.settlement);
}

QString EntryHubO::QSRemoveSettlementFirst() const
{
    return QString(R"(
    UPDATE %1 SET
        is_valid = FALSE
    WHERE id = :node_id
    )")
        .arg(info_.settlement);
}

QString EntryHubO::QSRemoveSettlementSecond() const
{
    return QString(R"(
    UPDATE %1 SET
        settlement = 0,
        final_total = 0
    WHERE settlement = :node_id
    )")
        .arg(info_.node);
}

QString EntryHubO::QSReadSettlementPrimary(bool status) const
{
    CString status_string { status ? QString() : "OR settlement = 0" };

    return QString(R"(
    SELECT id, issued_time, description, initial_total, employee, settlement
    FROM %1
    WHERE partner = :partner_id AND unit = 1 AND status = TRUE AND (settlement = :settlement %2) AND is_valid = TRUE
    )")
        .arg(info_.node, status_string);
}

QString EntryHubO::QSReadSettlement() const
{
    // return BuildSelect(info_.settlement, QStringLiteral("(issued_time BETWEEN :start AND :end) AND is_valid = TRUE"));
    return {};
}
