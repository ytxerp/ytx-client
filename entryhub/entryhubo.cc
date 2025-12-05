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

bool EntryHubO::ReadSettlement(SettlementList& list, const QDateTime& start, const QDateTime& end) const
{
    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // CString sql { QSReadSettlement() };

    // query.prepare(sql);
    // query.bindValue(QStringLiteral(":start"), start.toString(Qt::ISODate));
    // query.bindValue(QStringLiteral(":end"), end.toString(Qt::ISODate));

    // if (!query.exec()) {
    //     qWarning() << "Failed in ReadStatement" << query.lastError().text();
    //     return false;
    // }

    // ReadSettlementQuery(list, query);
    return true;
}

bool EntryHubO::WriteSettlement(const Settlement* settlement) const
{
    // QSqlQuery query(main_db_);

    // CString sql { QString(R"(
    // INSERT INTO %1 (id, issued_time)
    // VALUES (:id, :issued_time)
    // )")
    //         .arg(info_.settlement) };

    // query.prepare(sql);
    // query.bindValue(QStringLiteral(":issued_time"), settlement->issued_time);
    // query.bindValue(QStringLiteral(":id"), settlement->id.toString(QUuid::WithoutBraces));

    // if (!query.exec()) {
    //     qWarning() << "Failed in WriteSettlement" << query.lastError().text();
    //     return false;
    // }

    return true;
}

bool EntryHubO::RemoveSettlement(const QUuid& settlement)
{
    // QSqlQuery query(main_db_);

    CString sql_count { QSRemoveSettlementFirst() };
    CString sql_measure { QSRemoveSettlementSecond() };

    // return DBTransaction([&]() {
    //     query.prepare(sql_count);
    //     query.bindValue(QStringLiteral(":node_id"), settlement.toString(QUuid::WithoutBraces));
    //     if (!query.exec()) {
    //         qWarning() << "Failed in RemoveSettlement 1st" << query.lastError().text();
    //         return false;
    //     }

    //     query.prepare(sql_measure);
    //     query.bindValue(QStringLiteral(":node_id"), settlement.toString(QUuid::WithoutBraces));
    //     if (!query.exec()) {
    //         qWarning() << "Failed in RemoveSettlement 2nd" << query.lastError().text();
    //         return false;
    //     }

    //     return true;
    // });
}

bool EntryHubO::ReadSettlementPrimary(SettlementList& list, const QUuid& partner_id, const QUuid& settlement, bool status)
{
    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // CString sql { QSReadSettlementPrimary(status) };

    // query.prepare(sql);
    // query.bindValue(QStringLiteral(":partner_id"), partner_id.toString(QUuid::WithoutBraces));
    // query.bindValue(QStringLiteral(":settlement"), settlement.toString(QUuid::WithoutBraces));

    // if (!query.exec()) {
    //     qWarning() << "Failed in ReadSettlementPrimary" << query.lastError().text();
    //     return false;
    // }

    // ReadSettlementPrimaryQuery(list, query);
    return true;
}

bool EntryHubO::AddSettlementPrimary(const QUuid& node_id, const QUuid& settlement) const
{
    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // CString sql { QString("UPDATE %1 SET settlement = :settlement, final_total = initial_total - discount_total WHERE id = :node_id").arg(info_.node)
    // };

    // query.prepare(sql);
    // query.bindValue(QStringLiteral(":node_id"), node_id.toString(QUuid::WithoutBraces));
    // query.bindValue(QStringLiteral(":settlement"), settlement.toString(QUuid::WithoutBraces));

    // if (!query.exec()) {
    //     qWarning() << "Failed in SyncNewSettlement" << query.lastError().text();
    //     return false;
    // }

    // auto* node { node_hash_.value(node_id) };
    // if (node) {
    //     node->final_total = node->initial_total - static_cast<NodeO*>(node)->discount_total;
    // }

    return true;
}

bool EntryHubO::RemoveSettlementPrimary(const QUuid& node_id) const
{
    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // CString sql { QString("UPDATE %1 SET settlement = 0, final_total = 0 WHERE id = :node_id").arg(info_.node) };

    // query.prepare(sql);
    // query.bindValue(QStringLiteral(":node_id"), node_id.toString(QUuid::WithoutBraces));

    // if (!query.exec()) {
    //     qWarning() << "Failed in SyncNewSettlement" << query.lastError().text();
    //     return false;
    // }

    // auto* node { node_hash_.value(node_id) };
    // if (node) {
    //     node->final_total = 0.0;
    // }

    return true;
}

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

QString EntryHubO::QSReadStatement(int unit) const
{
    switch (UnitO(unit)) {
    case UnitO::kImmediate:
        return QString(R"(
        WITH Statement AS (
            SELECT
                p.id AS partner,
                SUM(o.initial_total)  AS camount,
                SUM(o.count_total)    AS ccount,
                SUM(o.measure_total)  AS cmeasure
            FROM partner_node p
            INNER JOIN %1 o
                ON p.id = o.partner
            WHERE o.unit = 0
                AND o.status = 1
                AND o.is_valid = TRUE
                AND o.issued_time >= :start
                AND o.issued_time  < :end
            GROUP BY p.id
        )
        SELECT
            partner,
            0.0 AS pbalance,
            0.0 AS cbalance,
            camount,
            camount AS csettlement,
            ccount,
            cmeasure
        FROM Statement
        )")
            .arg(info_.node);
    case UnitO::kMonthly:
        return QString(R"(
        WITH pbalance_statement AS (
            SELECT
                p.id AS partner,
                SUM(o.initial_total) AS pbalance
            FROM partner_node p
            INNER JOIN %1 o ON p.id = o.partner
            WHERE o.unit = 1
                AND o.status = 1
                AND o.is_valid = TRUE
                AND o.issued_time < :start
                AND o.settlement = '00000000-0000-0000-0000-000000000000'::uuid
            GROUP BY p.id
        ),
        current_statement AS (
            SELECT
                p.id AS partner,
                SUM(o.initial_total) AS camount,
                SUM(CASE WHEN o.settlement != '00000000-0000-0000-0000-000000000000'::uuid THEN o.initial_total ELSE 0 END) AS csettlement,
                SUM(o.count_total) AS ccount,
                SUM(o.measure_total) AS cmeasure
            FROM partner_node p
            INNER JOIN %1 o ON p.id = o.partner
            WHERE o.unit = 1
                AND o.status = 1
                AND o.is_valid = TRUE
                AND o.issued_time >= :start
                AND o.issued_time  < :end
            GROUP BY p.id
        )
        SELECT
            COALESCE(p.partner, c.partner) AS partner,
            COALESCE(p.pbalance, 0.0) AS pbalance,
            COALESCE(c.camount, 0.0) AS camount,
            COALESCE(c.csettlement, 0.0) AS csettlement,
            COALESCE(p.pbalance, 0.0) + COALESCE(c.camount, 0.0) - COALESCE(c.csettlement, 0.0) AS cbalance,
            COALESCE(c.ccount, 0.0) AS ccount,
            COALESCE(c.cmeasure, 0.0) AS cmeasure
        FROM pbalance_statement p
        FULL OUTER JOIN current_statement c ON p.partner = c.partner;
            )")
            .arg(info_.node);
    case UnitO::kPending:
        return QString(R"(
        WITH pbalance_statement AS (
            SELECT
                p.id AS partner,
                SUM(o.initial_total) AS pbalance
            FROM partner_node p
            INNER JOIN %1 o ON p.id = o.partner
            WHERE o.unit = 2
                AND o.is_valid = TRUE
                AND o.issued_time < :start
            GROUP BY p.id
        ),
        current_statement AS (
            SELECT
                p.id AS partner,
                SUM(o.initial_total)       AS camount,
                SUM(o.count_total)         AS ccount,
                SUM(o.measure_total)       AS cmeasure
            FROM partner_node p
            INNER JOIN %1 o ON p.id = o.partner
            WHERE o.unit = 2
                AND o.is_valid = TRUE
                AND o.issued_time >= :start
                AND o.issued_time < :end
            GROUP BY p.id
        )
        SELECT
            COALESCE(p.partner, c.partner) AS partner,
            COALESCE(p.pbalance, 0.0) AS pbalance,
            COALESCE(c.camount, 0.0) AS camount,
            0.0 AS csettlement,
            COALESCE(p.pbalance, 0.0) + COALESCE(c.camount, 0.0) AS cbalance,
            COALESCE(c.ccount, 0.0) AS ccount,
            COALESCE(c.cmeasure, 0.0) AS cmeasure
        FROM pbalance_statement p
        FULL OUTER JOIN current_statement c ON p.partner = c.partner;
            )")
            .arg(info_.node);
    default:
        return {};
    }
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

QString EntryHubO::QSReadStatementPrimary(int unit) const
{
    static const QString kBaseQuery = R"(
        SELECT description, employee, issued_time, count, measure, initial_total, %1 AS final_total
        FROM %2
        WHERE unit = :unit AND partner = :partner AND (issued_time BETWEEN :start AND :end) %3 AND is_valid = TRUE
    )";

    QString settlement_expr {};
    QString status_condition {};

    switch (UnitO(unit)) {
    case UnitO::kImmediate:
        settlement_expr = QStringLiteral("initial_total");
        status_condition = QStringLiteral("AND status = TRUE");
        break;
    case UnitO::kMonthly:
        settlement_expr = QStringLiteral("0");
        status_condition = QStringLiteral("AND status = TRUE");
        break;
    case UnitO::kPending:
        settlement_expr = QStringLiteral("0");
        status_condition = QLatin1String("");
        break;
    default:
        return {};
    }

    return kBaseQuery.arg(settlement_expr, info_.node, status_condition);
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

QString EntryHubO::QSInvertTransValue() const
{
    return QString(R"(
        UPDATE %1
        SET
            initial = -initial,
            final = -final,
            discount = -discount,
            count = -count,
            measure = -measure
        WHERE lhs_node = :lhs_node;
        )")
        .arg(info_.entry);
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

// void EntryHubO::ReadSettlementPrimaryQuery(SettlementList& node_list, QSqlQuery& query)
// {
//     while (query.next()) {
//         auto* settlement { ResourcePool<Settlement>::Instance().Allocate() };

//         settlement->id = query.value(QStringLiteral("id")).toUuid();
//         settlement->employee = query.value(QStringLiteral("employee")).toUuid();
//         settlement->description = query.value(QStringLiteral("description")).toString();
//         settlement->issued_time = query.value(QStringLiteral("issued_time")).toDateTime();
//         settlement->initial_total = query.value(QStringLiteral("initial_total")).toDouble();
//         settlement->status = !query.value(QStringLiteral("settlement")).toUuid().isNull();

//         node_list.emplaceBack(settlement);
//     }
// }

// void EntryHubO::ReadStatementQuery(StatementList& list, QSqlQuery& query) const
// {
//     while (query.next()) {
//         auto* statement { ResourcePool<Statement>::Instance().Allocate() };

//         statement->partner = query.value(QStringLiteral("partner")).toUuid();
//         statement->pbalance = query.value(QStringLiteral("pbalance")).toDouble();
//         statement->cgross_amount = query.value(QStringLiteral("cgross_amount")).toDouble();
//         statement->csettlement = query.value(QStringLiteral("csettlement")).toDouble();
//         statement->cbalance = query.value(QStringLiteral("cbalance")).toDouble();
//         statement->ccount = query.value(QStringLiteral("ccount")).toDouble();
//         statement->cmeasure = query.value(QStringLiteral("cmeasure")).toDouble();

//         list.emplaceBack(statement);
//     }
// }

// void EntryHubO::ReadStatementPrimaryQuery(StatementPrimaryList& list, QSqlQuery& query) const
// {
//     while (query.next()) {
//         auto* statement_primary { ResourcePool<StatementPrimary>::Instance().Allocate() };

//         statement_primary->description = query.value(QStringLiteral("description")).toString();
//         statement_primary->employee = query.value(QStringLiteral("employee")).toUuid();
//         statement_primary->issued_time = query.value(QStringLiteral("issued_time")).toDateTime();
//         statement_primary->count = query.value(QStringLiteral("count")).toDouble();
//         statement_primary->measure = query.value(QStringLiteral("measure")).toDouble();
//         statement_primary->initial_total = query.value(QStringLiteral("initial_total")).toDouble();
//         statement_primary->final_total = query.value(QStringLiteral("final_total")).toDouble();

//         list.emplaceBack(statement_primary);
//     }
// }

// void EntryHubO::ReadStatementSecondaryQuery(StatementSecondaryList& list, QSqlQuery& query) const
// {
//     while (query.next()) {
//         auto* trans { ResourcePool<StatementSecondary>::Instance().Allocate() };

//         trans->unit_price = query.value(QStringLiteral("unit_price")).toDouble();
//         trans->measure = query.value(QStringLiteral("measure")).toDouble();
//         trans->description = query.value(QStringLiteral("description")).toString();
//         trans->count = query.value(QStringLiteral("count")).toDouble();
//         trans->initial = query.value(QStringLiteral("initial")).toDouble();
//         trans->support_id = query.value(QStringLiteral("support_id")).toUuid();
//         trans->rhs_node = query.value(QStringLiteral("rhs_node")).toUuid();
//         trans->issued_time = query.value(QStringLiteral("issued_time")).toDateTime();

//         list.emplaceBack(trans);
//     }
// }

// void EntryHubO::ReadSettlementQuery(SettlementList& settlement_list, QSqlQuery& query) const
// {
//     while (query.next()) {
//         auto* settlement { ResourcePool<Settlement>::Instance().Allocate() };

//         settlement->id = query.value(QStringLiteral("id")).toUuid();
//         settlement->partner = query.value(QStringLiteral("partner")).toUuid();
//         settlement->description = query.value(QStringLiteral("description")).toString();
//         settlement->issued_time = query.value(QStringLiteral("issued_time")).toDateTime();
//         settlement->status = query.value(QStringLiteral("status")).toBool();
//         settlement->initial_total = query.value(QStringLiteral("initial_total")).toDouble();

//         settlement_list.emplaceBack(settlement);
//     }
// }

QString EntryHubO::QSReadSettlement() const
{
    // return BuildSelect(info_.settlement, QStringLiteral("(issued_time BETWEEN :start AND :end) AND is_valid = TRUE"));
    return {};
}
