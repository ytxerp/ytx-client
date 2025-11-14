#include "entryhubo.h"

#include <QDate>
#include <QJsonArray>

#include "component/using.h"
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

bool EntryHubO::RemoveSettlement(const QUuid& settlement_id)
{
    // QSqlQuery query(main_db_);

    CString sql_first { QSRemoveSettlementFirst() };
    CString sql_second { QSRemoveSettlementSecond() };

    // return DBTransaction([&]() {
    //     query.prepare(sql_first);
    //     query.bindValue(QStringLiteral(":node_id"), settlement_id.toString(QUuid::WithoutBraces));
    //     if (!query.exec()) {
    //         qWarning() << "Failed in RemoveSettlement 1st" << query.lastError().text();
    //         return false;
    //     }

    //     query.prepare(sql_second);
    //     query.bindValue(QStringLiteral(":node_id"), settlement_id.toString(QUuid::WithoutBraces));
    //     if (!query.exec()) {
    //         qWarning() << "Failed in RemoveSettlement 2nd" << query.lastError().text();
    //         return false;
    //     }

    //     return true;
    // });
}

bool EntryHubO::ReadSettlementPrimary(SettlementList& list, const QUuid& partner_id, const QUuid& settlement_id, bool is_finished)
{
    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // CString sql { QSReadSettlementPrimary(is_finished) };

    // query.prepare(sql);
    // query.bindValue(QStringLiteral(":partner_id"), partner_id.toString(QUuid::WithoutBraces));
    // query.bindValue(QStringLiteral(":settlement_id"), settlement_id.toString(QUuid::WithoutBraces));

    // if (!query.exec()) {
    //     qWarning() << "Failed in ReadSettlementPrimary" << query.lastError().text();
    //     return false;
    // }

    // ReadSettlementPrimaryQuery(list, query);
    return true;
}

bool EntryHubO::AddSettlementPrimary(const QUuid& node_id, const QUuid& settlement_id) const
{
    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // CString sql { QString("UPDATE %1 SET settlement_id = :settlement_id, final_total = initial_total - discount_total WHERE id = :node_id").arg(info_.node)
    // };

    // query.prepare(sql);
    // query.bindValue(QStringLiteral(":node_id"), node_id.toString(QUuid::WithoutBraces));
    // query.bindValue(QStringLiteral(":settlement_id"), settlement_id.toString(QUuid::WithoutBraces));

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

    // CString sql { QString("UPDATE %1 SET settlement_id = 0, final_total = 0 WHERE id = :node_id").arg(info_.node) };

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

bool EntryHubO::ReadStatement(StatementList& list, int unit, const QDateTime& start, const QDateTime& end) const
{
    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // CString sql { QSReadStatement(unit) };

    // query.prepare(sql);
    // query.bindValue(QStringLiteral(":start"), start.toString(Qt::ISODate));
    // query.bindValue(QStringLiteral(":end"), end.toString(Qt::ISODate));

    // if (!query.exec()) {
    //     qWarning() << "Failed in ReadStatement" << query.lastError().text();
    //     return false;
    // }

    // ReadStatementQuery(list, query);

    return true;
}

bool EntryHubO::ReadBalance(double& pbalance, double& cdelta, const QUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end) const
{
    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // CString sql { QSReadBalance(unit) };

    // query.prepare(sql);
    // query.bindValue(QStringLiteral(":start"), start.toString(kDateTimeFST));
    // query.bindValue(QStringLiteral(":end"), end.toString(kDateTimeFST));
    // query.bindValue(QStringLiteral(":partner_id"), partner_id.toString(QUuid::WithoutBraces));

    // if (!query.exec()) {
    //     qWarning() << "Failed in ReadStatement" << query.lastError().text();
    //     return false;
    // }

    // if (query.next()) {
    //     pbalance = query.value(QStringLiteral("pbalance")).toDouble();
    //     cdelta = query.value(QStringLiteral("cgross_amount")).toDouble() - query.value(QStringLiteral("csettlement")).toDouble();
    // }

    return true;
}

bool EntryHubO::ReadStatementPrimary(StatementPrimaryList& list, const QUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end) const
{
    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // CString sql { QSReadStatementPrimary(unit) };

    // query.prepare(sql);
    // query.bindValue(QStringLiteral(":start"), start.toString(Qt::ISODate));
    // query.bindValue(QStringLiteral(":end"), end.toString(Qt::ISODate));
    // query.bindValue(QStringLiteral(":partner"), partner_id.toString(QUuid::WithoutBraces));
    // query.bindValue(QStringLiteral(":unit"), unit);

    // if (!query.exec()) {
    //     qWarning() << "Failed in ReadStatementPrimary" << query.lastError().text();
    //     return false;
    // }

    // ReadStatementPrimaryQuery(list, query);
    return true;
}

bool EntryHubO::ReadStatementSecondary(StatementSecondaryList& list, const QUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end) const
{
    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // CString sql { QSReadStatementSecondary(unit) };

    // query.prepare(sql);
    // query.bindValue(QStringLiteral(":start"), start.toString(Qt::ISODate));
    // query.bindValue(QStringLiteral(":end"), end.toString(Qt::ISODate));
    // query.bindValue(QStringLiteral(":partner"), partner_id.toString(QUuid::WithoutBraces));
    // query.bindValue(QStringLiteral(":unit"), unit);

    // if (!query.exec()) {
    //     qWarning() << "Failed in ReadStatementPrimary" << query.lastError().text();
    //     return false;
    // }

    // ReadStatementSecondaryQuery(list, query);
    return true;
}

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
                    s.id AS partner,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end THEN o.initial_total ELSE 0 END) AS cgross_amount,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end THEN o.first        ELSE 0 END) AS cfirst,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end THEN o.second       ELSE 0 END) AS csecond
                FROM stakeholder_node s
                INNER JOIN %1 o ON s.id = o.partner
                WHERE o.unit = 0 AND o.is_finished = TRUE AND o.is_valid = TRUE
                GROUP BY s.id
            )
            SELECT
                partner,
                0 AS pbalance,
                0 AS cbalance,
                cgross_amount,
                cgross_amount AS csettlement,
                cfirst,
                csecond
            FROM Statement
            )")
            .arg(info_.node);
    case UnitO::kMonthly:
        return QString(R"(
            WITH Statement AS (
                SELECT
                    s.id AS partner,

                    SUM(CASE WHEN o.issued_time < :start AND o.settlement_id = 0 THEN o.initial_total                 ELSE 0 END) AS pbalance,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end THEN o.initial_total                          ELSE 0 END) AS cgross_amount,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end AND o.settlement_id != 0 THEN o.initial_total ELSE 0 END) AS csettlement,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end THEN o.first                                  ELSE 0 END) AS cfirst,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end THEN o.second                                 ELSE 0 END) AS csecond

                FROM stakeholder_node s
                INNER JOIN %1 o ON s.id = o.partner
                WHERE o.unit = 1 AND o.is_finished = TRUE AND o.is_valid = TRUE
                GROUP BY s.id
            )
            SELECT
                partner,
                pbalance,
                cgross_amount,
                csettlement,
                pbalance + cgross_amount - csettlement AS cbalance,
                cfirst,
                csecond
            FROM Statement;
            )")
            .arg(info_.node);
    case UnitO::kPending:
        return QString(R"(
            WITH Statement AS (
                SELECT
                    s.id AS partner,
                    SUM(CASE WHEN o.issued_time < :start THEN o.initial_total                ELSE 0 END) AS pbalance,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end THEN o.initial_total ELSE 0 END) AS cgross_amount,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end THEN o.first         ELSE 0 END) AS cfirst,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end THEN o.second        ELSE 0 END) AS csecond
                FROM stakeholder_node s
                INNER JOIN %1 o ON s.id = o.partner
                WHERE o.unit = 2 AND o.is_valid = TRUE
                GROUP BY s.id
            )
            SELECT
                partner,
                pbalance,
                cgross_amount,
                0 AS csettlement,
                pbalance + cgross_amount AS cbalance,
                cfirst,
                csecond
            FROM Statement;
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

                    SUM(CASE WHEN o.issued_time < :start AND o.settlement_id = 0 THEN o.initial_total                 ELSE 0 END) AS pbalance,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end THEN o.initial_total                          ELSE 0 END) AS cgross_amount,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end AND o.settlement_id != 0 THEN o.initial_total ELSE 0 END) AS csettlement

                FROM stakeholder_node s
                INNER JOIN %1 o ON s.id = o.partner
                WHERE o.partner = :partner_id AND o.unit = 1 AND o.is_finished = TRUE AND o.is_valid = TRUE
            )")
            .arg(info_.node);
    case UnitO::kPending:
        return QString(R"(
                SELECT

                    SUM(CASE WHEN o.issued_time < :start THEN o.initial_total                 ELSE 0 END) AS pbalance,
                    SUM(CASE WHEN o.issued_time BETWEEN :start AND :end THEN o.initial_total  ELSE 0 END) AS cgross_amount,
                    0 AS csettlement

                FROM stakeholder_node s
                INNER JOIN %1 o ON s.id = o.partner
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
        SELECT description, employee, issued_time, first, second, initial_total, %1 AS final_total
        FROM %2
        WHERE unit = :unit AND partner = :partner AND (issued_time BETWEEN :start AND :end) %3 AND is_valid = TRUE
    )";

    QString settlement_expr {};
    QString is_finished_condition {};

    switch (UnitO(unit)) {
    case UnitO::kImmediate:
        settlement_expr = QStringLiteral("initial_total");
        is_finished_condition = QStringLiteral("AND is_finished = TRUE");
        break;
    case UnitO::kMonthly:
        settlement_expr = QStringLiteral("0");
        is_finished_condition = QStringLiteral("AND is_finished = TRUE");
        break;
    case UnitO::kPending:
        settlement_expr = QStringLiteral("0");
        is_finished_condition = QLatin1String("");
        break;
    default:
        return {};
    }

    return kBaseQuery.arg(settlement_expr, info_.node, is_finished_condition);
}

QString EntryHubO::QSReadStatementSecondary(int unit) const
{
    static const QString kBaseQuery = R"(
        SELECT
            trans.rhs_node,
            trans.unit_price,
            trans.second,
            trans.description,
            trans.first,
            trans.initial,
            trans.support_id,
            node.issued_time
        FROM %2 trans
        INNER JOIN %1 node ON trans.lhs_node = node.id
        WHERE node.unit = :unit AND node.partner = :partner AND (node.issued_time BETWEEN :start AND :end) %3 AND trans.is_valid = TRUE
    )";

    QString is_finished_condition {};

    switch (UnitO(unit)) {
    case UnitO::kImmediate:
    case UnitO::kMonthly:
        is_finished_condition = QStringLiteral("AND node.is_finished = TRUE");
        break;
    case UnitO::kPending:
        is_finished_condition = QLatin1String("");
        break;
    default:
        return {};
    }

    return kBaseQuery.arg(info_.node, info_.entry, is_finished_condition);
}

QString EntryHubO::QSInvertTransValue() const
{
    return QString(R"(
        UPDATE %1
        SET
            initial = -initial,
            final = -final,
            discount = -discount,
            first = -first,
            second = -second
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
        settlement_id = 0,
        final_total = 0
    WHERE settlement_id = :node_id
    )")
        .arg(info_.node);
}

QString EntryHubO::QSReadSettlementPrimary(bool is_finished) const
{
    CString is_finished_string { is_finished ? QString() : "OR settlement_id = 0" };

    return QString(R"(
    SELECT id, issued_time, description, initial_total, employee, settlement_id
    FROM %1
    WHERE partner = :partner_id AND unit = 1 AND is_finished = TRUE AND (settlement_id = :settlement_id %2) AND is_valid = TRUE
    )")
        .arg(info_.node, is_finished_string);
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
//         settlement->is_finished = !query.value(QStringLiteral("settlement_id")).toUuid().isNull();

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
//         statement->cfirst = query.value(QStringLiteral("cfirst")).toDouble();
//         statement->csecond = query.value(QStringLiteral("csecond")).toDouble();

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
//         statement_primary->first = query.value(QStringLiteral("first")).toDouble();
//         statement_primary->second = query.value(QStringLiteral("second")).toDouble();
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
//         trans->second = query.value(QStringLiteral("second")).toDouble();
//         trans->description = query.value(QStringLiteral("description")).toString();
//         trans->first = query.value(QStringLiteral("first")).toDouble();
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
//         settlement->is_finished = query.value(QStringLiteral("is_finished")).toBool();
//         settlement->initial_total = query.value(QStringLiteral("initial_total")).toDouble();

//         settlement_list.emplaceBack(settlement);
//     }
// }

QString EntryHubO::QSReadSettlement() const
{
    // return BuildSelect(info_.settlement, QStringLiteral("(issued_time BETWEEN :start AND :end) AND is_valid = TRUE"));
    return {};
}
