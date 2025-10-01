#include "entryhubo.h"

#include <QDate>

#include "component/constant.h"
#include "component/using.h"

EntryHubO::EntryHubO(CSectionInfo& info, QObject* parent)
    : EntryHub(info, parent)
{
}

bool EntryHubO::SearchNode(QList<const Node*>& node_list, const QList<QUuid>& partner_id_list)
{
    if (partner_id_list.empty())
        return false;

    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // const qsizetype batch_size { kBatchSize };
    // const auto total_batches { (partner_id_list.size() + batch_size - 1) / batch_size };

    // for (int batch_index = 0; batch_index != total_batches; ++batch_index) {
    //     int start = batch_index * batch_size;
    //     int end = std::min(start + batch_size, partner_id_list.size());

    //     QList<QUuid> current_batch { partner_id_list.mid(start, end - start) };

    //     QStringList placeholder { current_batch.size(), QStringLiteral("?") };
    //     QString string { QSSearchNode(placeholder.join(QStringLiteral(","))) };

    //     query.prepare(string);

    //     for (int i = 0; i != current_batch.size(); ++i)
    //         query.bindValue(i, current_batch.at(i));

    //     if (!query.exec()) {
    //         qWarning() << "Section: " << std::to_underlying(section_) << "Failed in SearchNode, batch" << batch_index << ": " << query.lastError().text();
    //         continue;
    //     }

    //     SearchNodeFunction(node_list, query);
    // }

    return true;
}

// void EntryHubO::SearchNodeFunction(QList<const Node*>& node_list, QSqlQuery& query)
// {
//     while (query.next()) {
//         const auto id { query.value(QStringLiteral("id")).toUuid() };

//         // if (auto it = node_hash_.constFind(id); it != node_hash_.constEnd()) {
//         //     node_list.emplaceBack(it.value());
//         //     continue;
//         // }

//         // auto* node { NodePool::Instance().Allocate(section_) };
//         // // node->ReadQuery(query);
//         // node_list.emplaceBack(node);
//         // node_hash_.insert(id, node);
//     }
// }

bool EntryHubO::SettlementReference(const QUuid& settlement_id) const
{
    assert(!settlement_id.isNull());

    CString sql { QString(R"(
    SELECT 1 FROM %1
    WHERE settlement_id = :settlement_id AND is_valid = TRUE
    LIMIT 1
    )")
            .arg(info_.node) };

    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // query.prepare(sql);
    // query.bindValue(QStringLiteral(":settlement_id"), settlement_id.toString(QUuid::WithoutBraces));

    // if (!query.exec()) {
    //     qWarning() << "Failed in SettlementReference" << query.lastError().text();
    //     return false;
    // }

    // return query.next();
    return true;
}

int EntryHubO::SettlementId(const QUuid& node_id) const
{
    assert(!node_id.isNull());

    CString sql { QString(R"(
    SELECT settlement_id FROM %1
    WHERE id = :node_id AND is_valid = TRUE
    )")
            .arg(info_.node) };

    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // query.prepare(sql);
    // query.bindValue(QStringLiteral(":node_id"), node_id.toString(QUuid::WithoutBraces));

    // if (!query.exec()) {
    //     qWarning() << "Failed in SettlementId" << query.lastError().text();
    //     return false;
    // }

    // if (query.next())
    //     return query.value(0).toInt();

    return 0;
}

// void EntryHubO::ApplyEntryRate(const QUuid& entry_id, const QJsonObject& data, bool /*is_parallel*/)
// {
//     auto it = entry_cache_.constFind(entry_id);
//     if (it != entry_cache_.constEnd()) {
//         auto* d_entry = static_cast<EntryO*>(it.value());

//         d_entry->unit_price = data[kUnitPrice].toString().toDouble();
//         d_entry->updated_time = QDateTime::fromString(data[kUpdatedTime].toString(), Qt::ISODate);
//         d_entry->updated_by = QUuid(data[kUpdatedBy].toString());

//         const int unit_price { std::to_underlying(EntryEnumO::kUnitPrice) };

//         emit SRefreshField(d_entry->lhs_node, entry_id, unit_price, unit_price);
//     }
// }

void EntryHubO::RemoveLeaf(const QHash<QUuid, QSet<QUuid>>& leaf_entry) { RemoveLeafFunction(leaf_entry); }

void EntryHubO::ApplyInventoryReplace(const QUuid& old_item_id, const QUuid& new_item_id) const
{
    for (auto* entry : std::as_const(entry_cache_)) {
        auto* d_entry = static_cast<EntryO*>(entry);

        if (d_entry->rhs_node == old_item_id)
            d_entry->rhs_node = new_item_id;

        if (d_entry->external_sku == old_item_id)
            d_entry->external_sku = new_item_id;
    }
}

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

bool EntryHubO::SyncPrice(const QUuid& node_id)
{
    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // CString sql_first { QSSyncPriceFirst() };

    // query.prepare(sql_first);
    // query.bindValue(QStringLiteral(":node_id"), node_id.toString(QUuid::WithoutBraces));

    // if (!query.exec()) {
    //     qWarning() << "SQL execution failed in QSSyncStakeholderPriceFirst:" << query.lastError().text();
    //     return false;
    // }

    // CString sql_second { QSSyncPriceSecond() };

    // query.prepare(sql_second);
    // query.bindValue(QStringLiteral(":node_id"), node_id.toString(QUuid::WithoutBraces));

    // if (!query.exec()) {
    //     qWarning() << "SQL execution failed in QSSyncStakeholderPriceSecond:" << query.lastError().text();
    //     return false;
    // }

    // QList<PriceS> list {};

    // while (query.next()) {
    //     PriceS item {};
    //     item.issued_time = query.value(QStringLiteral("issued_time")).toDateTime();
    //     item.lhs_node = query.value(QStringLiteral("lhs_node")).toUuid();
    //     item.rhs_node = query.value(QStringLiteral("rhs_node")).toUuid();
    //     item.unit_price = query.value(QStringLiteral("unit_price")).toDouble();

    //     list.append(std::move(item));
    // }

    // emit SSyncPrice(list);
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

bool EntryHubO::WriteTransRange(const QList<EntryShadow*>& list)
{
    // if (list.isEmpty())
    //     return false;

    // QSqlQuery query(main_db_);

    // query.exec(QStringLiteral("PRAGMA synchronous = OFF"));
    // query.exec(QStringLiteral("PRAGMA journal_mode = MEMORY"));

    // if (!main_db_.transaction()) {
    //     qDebug() << "Failed to start transaction" << main_db_.lastError();
    //     return false;
    // }

    // CString sql {};

    // // 插入多条记录的 SQL 语句
    // query.prepare(sql);
    // WriteTransRangeFunction(list, query);

    // // 执行批量插入
    // if (!query.execBatch()) {
    //     qDebug() << "Failed in WriteTransRange" << query.lastError();
    //     main_db_.rollback();
    //     return false;
    // }

    // // 提交事务
    // if (!main_db_.commit()) {
    //     qDebug() << "Failed to commit transaction" << main_db_.lastError();
    //     main_db_.rollback();
    //     return false;
    // }

    // query.exec(QStringLiteral("PRAGMA synchronous = FULL"));
    // query.exec(QStringLiteral("PRAGMA journal_mode = DELETE"));

    return true;
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

QString EntryHubO::QSSyncPriceFirst() const
{
    return QString(R"(
        INSERT INTO stakeholder_transaction(issued_time, lhs_node, rhs_node, unit_price)
        SELECT
            node.issued_time,
            node.partner AS lhs_node,
            trans.rhs_node,
            trans.unit_price
        FROM %2 trans
        JOIN %1 node ON trans.lhs_node = node.id
        JOIN item_node p ON trans.rhs_node = p.id
        WHERE trans.lhs_node = :node_id AND trans.unit_price <> p.unit_price AND trans.is_valid = TRUE
        ON CONFLICT(lhs_node, rhs_node) DO UPDATE SET
        issued_time = excluded.issued_time,
        unit_price = excluded.unit_price,
        is_valid = TRUE;
    )")
        .arg(info_.node, info_.entry);
}

QString EntryHubO::QSSyncPriceSecond() const
{
    return QString(R"(
        SELECT
            node.issued_time,
            node.partner AS lhs_node,
            trans.rhs_node,
            trans.unit_price
        FROM %2 trans
        JOIN %1 node ON trans.lhs_node = node.id
        JOIN item_node p ON trans.rhs_node = p.id
        WHERE trans.lhs_node = :node_id AND trans.unit_price <> p.unit_price AND trans.is_valid = TRUE
    )")
        .arg(info_.node, info_.entry);
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

// void EntryHubO::WriteTransRangeFunction(const QList<EntryShadow*>& list, QSqlQuery& query) const
// {
//     const int size = list.size();

//     QVariantList code_list {};
//     QVariantList rhs_node_list {};
//     QVariantList unit_price_list {};
//     QVariantList description_list {};
//     QVariantList second_list {};
//     QVariantList lhs_node_list {};
//     QVariantList first_list {};
//     QVariantList initial_list {};
//     QVariantList discount_list {};
//     QVariantList final_list {};
//     QVariantList support_id_list {};
//     QVariantList discount_price_list {};

//     code_list.reserve(size);
//     rhs_node_list.reserve(size);
//     unit_price_list.reserve(size);
//     description_list.reserve(size);
//     second_list.reserve(size);
//     lhs_node_list.reserve(size);
//     first_list.reserve(size);
//     initial_list.reserve(size);
//     discount_list.reserve(size);
//     final_list.reserve(size);
//     support_id_list.reserve(size);
//     discount_price_list.reserve(size);

//     for (const EntryShadow* entry_shadow : list) {
//         if (entry_shadow->rhs_node->isNull() || entry_shadow->lhs_node->isNull())
//             continue;

//         auto* o_entry_shadow { static_cast<const EntryShadowO*>(entry_shadow) };

//         code_list.emplaceBack(*o_entry_shadow->code);
//         rhs_node_list.emplaceBack(o_entry_shadow->rhs_node->toString(QUuid::WithoutBraces));
//         unit_price_list.emplaceBack(*o_entry_shadow->unit_price);
//         description_list.emplaceBack(*o_entry_shadow->description);
//         second_list.emplaceBack(*o_entry_shadow->second);
//         lhs_node_list.emplaceBack(o_entry_shadow->lhs_node->toString(QUuid::WithoutBraces));
//         first_list.emplaceBack(*o_entry_shadow->first);
//         initial_list.emplaceBack(*o_entry_shadow->initial);
//         discount_list.emplaceBack(*o_entry_shadow->discount);
//         final_list.emplaceBack(*o_entry_shadow->final);
//         support_id_list.emplaceBack(o_entry_shadow->external_item->toString(QUuid::WithoutBraces));
//         discount_price_list.emplaceBack(*o_entry_shadow->discount_price);
//     }

//     // 批量绑定 QVariantList
//     query.bindValue(QStringLiteral(":code"), code_list);
//     query.bindValue(QStringLiteral(":rhs_node"), rhs_node_list);
//     query.bindValue(QStringLiteral(":unit_price"), unit_price_list);
//     query.bindValue(QStringLiteral(":description"), description_list);
//     query.bindValue(QStringLiteral(":second"), second_list);
//     query.bindValue(QStringLiteral(":lhs_node"), lhs_node_list);
//     query.bindValue(QStringLiteral(":first"), first_list);
//     query.bindValue(QStringLiteral(":initial"), initial_list);
//     query.bindValue(QStringLiteral(":discount"), discount_list);
//     query.bindValue(QStringLiteral(":final"), final_list);
//     query.bindValue(QStringLiteral(":support_id"), support_id_list);
//     query.bindValue(QStringLiteral(":discount_price"), discount_price_list);
// }

QString EntryHubO::QSSearchNode(CString& in_list) const { /*return BuildSelect(info_.node, QString("partner IN (%1) AND is_valid = TRUE").arg(in_list));*/
    return {}; }

QString EntryHubO::QSReadSettlement() const
{
    // return BuildSelect(info_.settlement, QStringLiteral("(issued_time BETWEEN :start AND :end) AND is_valid = TRUE"));
    return {};
}
