#include "sqls.h"

#include <QSqlError>
#include <QSqlQuery>

#include "component/constvalue.h"
#include "global/resourcepool.h"

SqlS::SqlS(CInfo& info, QObject* parent)
    : Sql(info, parent)
{
}

void SqlS::RReplaceNode(const QUuid& old_node_id, const QUuid& new_node_id, int node_type, int node_unit)
{
    emit SFreeWidget(old_node_id, node_type);
    emit SRemoveNode(old_node_id);

    if (node_type == kTypeLeaf) {
        ReplaceLeaf(old_node_id, new_node_id, node_unit);
        emit SSyncStakeholder(old_node_id, new_node_id);
        emit SSyncMultiLeafValue({ new_node_id });
    }

    if (node_type == kTypeSupport) {
        QSet<QUuid> trans_id_set {};
        ReplaceSupportFunction(trans_id_set, old_node_id, new_node_id);
        emit SMoveMultiTransS(section_, {}, new_node_id, trans_id_set);

        ReplaceSupport(old_node_id, new_node_id);
    }

    RemoveNode(old_node_id, node_type);
}

void SqlS::RRemoveNode(const QUuid& node_id, int node_type)
{
    // This function is triggered when removing a node that has internal/support references.
    emit SFreeWidget(node_id, node_type);
    emit SRemoveNode(node_id);

    if (node_type == kTypeSupport) {
        RemoveSupportFunction(node_id);
    }

    if (node_type == kTypeLeaf) {
        QMultiHash<QUuid, QUuid> leaf_trans {};
        QMultiHash<QUuid, QUuid> support_trans {};

        TransToRemove(leaf_trans, support_trans, node_id, node_type);

        emit SSyncStakeholder(node_id, {}); // Update Stakeholder's employee

        if (!support_trans.isEmpty())
            emit SRemoveMultiTransS(section_, support_trans);

        RemoveLeafFunction(leaf_trans);
    }

    // Remove node, path, trans from the sqlite3 database
    RemoveNode(node_id, node_type);
}

void SqlS::RPriceSList(const QList<PriceS>& list)
{
    for (int i = 0; i != list.size(); ++i) {
        Trans* latest_trans { nullptr };

        for (auto* trans : std::as_const(trans_hash_)) {
            if (trans->lhs_node == list[i].lhs_node && trans->rhs_node == list[i].inside_product) {
                latest_trans = trans;
                break;
            }
        }

        if (latest_trans) {
            latest_trans->lhs_ratio = list[i].unit_price;
            latest_trans->rhs_ratio = list[i].unit_price;
            latest_trans->issued_time = list[i].issued_time;
        }
    }

    QSet<QUuid> set {};
    for (const auto& item : std::as_const(list)) {
        set.insert(item.inside_product);
    }

    ReadTransRange(set);
}

void SqlS::RSyncProduct(const QUuid& old_node_id, const QUuid& new_node_id) const
{
    for (auto* trans : std::as_const(trans_hash_))
        if (trans->rhs_node == old_node_id)
            trans->rhs_node = new_node_id;
}

bool SqlS::CrossSearch(TransShadow* order_trans_shadow, const QUuid& party_id, const QUuid& product_id, bool is_inside) const
{
    const Trans* latest_trans { nullptr };

    for (const auto* trans : std::as_const(trans_hash_)) {
        if (is_inside && trans->lhs_node == party_id && trans->rhs_node == product_id) {
            latest_trans = trans;
            break;
        }

        if (!is_inside && trans->lhs_node == party_id && trans->support_id == product_id) {
            latest_trans = trans;
            break;
        }
    }

    if (!latest_trans) {
        return false;
    }

    *order_trans_shadow->lhs_ratio = latest_trans->lhs_ratio;

    if (is_inside) {
        *order_trans_shadow->support_id = latest_trans->support_id;
    } else {
        *order_trans_shadow->rhs_node = latest_trans->rhs_node;
    }

    return true;
}

bool SqlS::ReadTransRange(const QSet<QUuid>& set)
{
    QSqlQuery query(main_db_);
    query.setForwardOnly(true);

    CString placeholder { QStringList(set.size(), "?").join(", ") };
    CString string { QString(R"(
    SELECT id, issued_time, code, outside_product, lhs_node, unit_price, description, document, is_checked, inside_product
    FROM stakeholder_transaction
    WHERE inside_product IN (%1)
    )")
            .arg(placeholder) };

    query.prepare(string);

    for (const auto& value : set) {
        query.addBindValue(value);
    }

    if (!query.exec()) {
        qWarning() << "Section: " << std::to_underlying(section_) << "Failed in RetrieveTrans" << query.lastError().text();
        return false;
    }

    TransShadowList trans_shadow_list {};

    while (query.next()) {
        const auto kID { QUuid::fromRfc4122(query.value(QStringLiteral("id")).toByteArray()) };

        if (trans_hash_.contains(kID))
            continue;

        auto* trans_shadow { ResourcePool<TransShadow>::Instance().Allocate() };
        auto* trans { ResourcePool<Trans>::Instance().Allocate() };
        trans->id = kID;

        ReadTransQuery(trans, query);
        trans_hash_.insert(kID, trans);

        ConvertTrans(trans, trans_shadow, true);
        trans_shadow_list.emplaceBack(trans_shadow);
    }

    if (!trans_shadow_list.isEmpty())
        emit SAppendMultiTrans(section_, *trans_shadow_list[0]->lhs_node, trans_shadow_list);

    return true;
}

bool SqlS::ReplaceLeafC(QSqlQuery& query, const QUuid& old_node_id, const QUuid& new_node_id)
{
    CString string { QSReplaceLeafOSP() };

    query.prepare(string);
    query.bindValue(QStringLiteral(":new_node_id"), new_node_id.toRfc4122());
    query.bindValue(QStringLiteral(":old_node_id"), old_node_id.toRfc4122());

    if (!query.exec()) {
        qWarning() << "Section: " << std::to_underlying(section_) << "Failed in ReplaceLeafC" << query.lastError().text();
        return false;
    }

    return true;
}

bool SqlS::ReplaceLeafE(QSqlQuery& query, const QUuid& old_node_id, const QUuid& new_node_id)
{
    CString stringse { QSReplaceLeafSE() };
    CString stringose { QSReplaceLeafOSE() };
    CString stringope { QSReplaceLeafOPE() };

    return DBTransaction([&]() {
        query.prepare(stringse);
        query.bindValue(QStringLiteral(":new_node_id"), new_node_id.toRfc4122());
        query.bindValue(QStringLiteral(":old_node_id"), old_node_id.toRfc4122());

        if (!query.exec()) {
            qWarning() << "Section: " << std::to_underlying(section_) << "Failed in ReplaceLeaf 1st" << query.lastError().text();
            return false;
        }

        query.prepare(stringose);
        query.bindValue(QStringLiteral(":new_node_id"), new_node_id.toRfc4122());
        query.bindValue(QStringLiteral(":old_node_id"), old_node_id.toRfc4122());

        if (!query.exec()) {
            qWarning() << "Section: " << std::to_underlying(section_) << "Failed in ReplaceLeaf 2nd" << query.lastError().text();
            return false;
        }

        query.prepare(stringope);
        query.bindValue(QStringLiteral(":new_node_id"), new_node_id.toRfc4122());
        query.bindValue(QStringLiteral(":old_node_id"), old_node_id.toRfc4122());

        if (!query.exec()) {
            qWarning() << "Section: " << std::to_underlying(section_) << "Failed in ReplaceLeaf 3rd" << query.lastError().text();
            return false;
        }

        return true;
    });
}

bool SqlS::ReplaceLeafV(QSqlQuery& query, const QUuid& old_node_id, const QUuid& new_node_id)
{
    CString string { QSReplaceLeafOPP() };

    query.prepare(string);
    query.bindValue(QStringLiteral(":new_node_id"), new_node_id.toRfc4122());
    query.bindValue(QStringLiteral(":old_node_id"), old_node_id.toRfc4122());

    if (!query.exec()) {
        qWarning() << "Section: " << std::to_underlying(section_) << "Failed in ReplaceLeafV" << query.lastError().text();
        return false;
    }

    return true;
}

bool SqlS::ReplaceLeaf(const QUuid& old_node_id, const QUuid& new_node_id, int node_unit)
{
    QSqlQuery query(main_db_);

    switch (UnitS(node_unit)) {
    case UnitS::kCust:
        ReplaceLeafC(query, old_node_id, new_node_id);
        break;
    case UnitS::kEmp:
        ReplaceLeafE(query, old_node_id, new_node_id);
        break;
    case UnitS::kVend:
        ReplaceLeafV(query, old_node_id, new_node_id);
        break;
    default:
        break;
    }

    return true;
}

bool SqlS::ReadTrans(const QUuid& node_id)
{
    QSqlQuery query(main_db_);
    query.setForwardOnly(true);

    CString string { QSReadTrans() };
    query.prepare(string);
    query.bindValue(QStringLiteral(":node_id"), node_id.toRfc4122());

    if (!query.exec()) {
        qWarning() << "Section: " << std::to_underlying(section_) << "Failed in ReadTrans" << query.lastError().text();
        return false;
    }

    ReadTransS(query);
    return true;
}

QString SqlS::QSReadNode() const
{
    return QStringLiteral(R"(
    SELECT name, id, code, description, note, node_type, unit, employee, deadline, payment_term, tax_rate, amount
    FROM stakeholder
    WHERE is_valid = TRUE
    )");
}

QString SqlS::QSWriteNode() const
{
    return QStringLiteral(R"(
    INSERT INTO stakeholder (name, id, code, description, note, node_type, unit, employee, deadline, payment_term, tax_rate, amount)
    VALUES (:name, :id, :code, :description, :note, :node_type, :unit, :employee, :deadline, :payment_term, :tax_rate, :amount)
    )");
}

void SqlS::WriteNodeBind(Node* node, QSqlQuery& query) const
{
    query.bindValue(QStringLiteral(":name"), node->name);
    query.bindValue(QStringLiteral(":id"), node->id.toRfc4122());
    query.bindValue(QStringLiteral(":code"), node->code);
    query.bindValue(QStringLiteral(":description"), node->description);
    query.bindValue(QStringLiteral(":note"), node->note);
    query.bindValue(QStringLiteral(":node_type"), node->node_type);
    query.bindValue(QStringLiteral(":unit"), node->unit);
    query.bindValue(QStringLiteral(":employee"), node->employee.toRfc4122());
    query.bindValue(QStringLiteral(":deadline"), node->issued_time);
    query.bindValue(QStringLiteral(":payment_term"), node->first);
    query.bindValue(QStringLiteral(":tax_rate"), node->second);
    query.bindValue(QStringLiteral(":amount"), node->final_total);
}

QString SqlS::QSRemoveNodeSecond() const
{
    return QStringLiteral(R"(
    DELETE FROM stakeholder_transaction
    WHERE lhs_node = :node_id;
    )");
}

QString SqlS::QSInternalReference() const
{
    return QStringLiteral(R"(
    SELECT
    EXISTS(SELECT 1 FROM stakeholder_transaction WHERE lhs_node = :node_id) OR
    EXISTS(SELECT 1 FROM stakeholder WHERE employee = :node_id AND is_valid = TRUE)
    AS is_referenced;
    )");
}

QString SqlS::QSExternalReference() const
{
    return QStringLiteral(R"(
    SELECT
        EXISTS(SELECT 1 FROM sales WHERE (party = :node_id OR employee = :node_id) AND is_valid = TRUE) OR
        EXISTS(SELECT 1 FROM purchase WHERE (party = :node_id OR employee = :node_id) AND is_valid = TRUE) OR
        EXISTS(SELECT 1 FROM sales_transaction WHERE outside_product = :node_id AND is_valid = TRUE) OR
        EXISTS(SELECT 1 FROM purchase_transaction WHERE outside_product = :node_id AND is_valid = TRUE)
    AS is_referenced;
    )");
}

QString SqlS::QSSupportReference() const
{
    return QStringLiteral(R"(
    SELECT 1 FROM stakeholder_transaction
    WHERE outside_product = :support_id
    LIMIT 1
    )");
}

QString SqlS::QSReplaceSupport() const
{
    return QStringLiteral(R"(
    UPDATE stakeholder_transaction SET
        outside_product = :new_node_id
    WHERE outside_product = :old_node_id
    )");
}

QString SqlS::QSUpdateLeafValue() const
{
    return QStringLiteral(R"(
    UPDATE stakeholder SET
        amount = :amount
    WHERE id = :node_id
    )");
}

void SqlS::UpdateLeafValueBind(const Node* node, QSqlQuery& query) const
{
    query.bindValue(":amount", node->final_total);
    query.bindValue(":node_id", node->id);
}

QString SqlS::QSRemoveSupport() const
{
    return QStringLiteral(R"(
    UPDATE stakeholder_transaction SET
        outside_product = 0
    WHERE outside_product = :node_id
    )");
}

QString SqlS::QSReadTrans() const
{
    return QStringLiteral(R"(
    SELECT id, issued_time, code, outside_product, lhs_node, unit_price, description, document, is_checked, inside_product
    FROM stakeholder_transaction
    WHERE lhs_node = :node_id
    )");
}

QString SqlS::QSReadSupportTrans() const
{
    return QStringLiteral(R"(
    SELECT id, issued_time, code, outside_product, lhs_node, unit_price, description, document, is_checked, inside_product
    FROM stakeholder_transaction
    WHERE outside_product = :node_id
    )");
}

QString SqlS::QSWriteTrans() const
{
    return QStringLiteral(R"(
    INSERT INTO stakeholder_transaction
    (id, issued_time, code, outside_product, lhs_node, unit_price, description, document, is_checked, inside_product)
    VALUES
    (:id, :issued_time, :code, :outside_product, :lhs_node, :unit_price, :description, :document, :is_checked, :inside_product)
    )");
}

QString SqlS::QSReplaceLeafSE() const
{
    return QStringLiteral(R"(
    UPDATE stakeholder
    SET employee = :new_node_id
    WHERE employee = :old_node_id;
    )");
}

QString SqlS::QSReplaceLeafOSE() const
{
    return QStringLiteral(R"(
    UPDATE sales
    SET employee = :new_node_id
    WHERE employee = :old_node_id;
    )");
}

QString SqlS::QSReplaceLeafOPE() const
{
    return QStringLiteral(R"(
    UPDATE purchase
    SET employee = :new_node_id
    WHERE employee = :old_node_id;
    )");
}

QString SqlS::QSReplaceLeafOSP() const
{
    return QStringLiteral(R"(
    UPDATE sales
    SET party = :new_node_id
    WHERE party = :old_node_id;
    )");
}

QString SqlS::QSReplaceLeafOPP() const
{
    return QStringLiteral(R"(
    UPDATE purchase
    SET party = :new_node_id
    WHERE party = :old_node_id;
    )");
}

QString SqlS::QSSearchTransValue() const
{
    return QStringLiteral(R"(
    SELECT id, issued_time, code, outside_product, lhs_node, unit_price, description, document, is_checked, inside_product
    FROM stakeholder_transaction
    WHERE unit_price BETWEEN :value - :tolerance AND :value + :tolerance
    )");
}

QString SqlS::QSSearchTransText() const
{
    return QStringLiteral(R"(
    SELECT id, issued_time, code, outside_product, lhs_node, unit_price, description, document, is_checked, inside_product
    FROM stakeholder_transaction
    WHERE description LIKE :description
    )");
}

QString SqlS::QSRemoveNodeFirst() const
{
    return QStringLiteral(R"(
    UPDATE stakeholder
    SET
        removed = CASE WHEN id = :node_id THEN 1 ELSE removed END,
        employee = CASE WHEN employee = :node_id THEN NULL ELSE employee END
    WHERE (id = :node_id OR employee = :node_id) AND is_valid = TRUE;
    )");
}

QString SqlS::QSTransToRemove() const
{
    return QStringLiteral(R"(
    SELECT lhs_node AS node_id, id AS trans_id, outside_product AS support_id FROM stakeholder_transaction
    WHERE lhs_node = :node_id
    )");
}

QString SqlS::QSReadTransRef(int unit) const
{
    QString base_query = QStringLiteral(R"(
        SELECT
            %4 AS section,
            %1.inside_product,
            %1.unit_price,
            %1.second,
            %1.lhs_node,
            %1.description,
            %1.first,
            %1.gross_amount,
            %1.outside_product,
            %1.discount_price,
            %2.issued_time
        FROM %1
        INNER JOIN %2 ON %1.lhs_node = %2.id
        WHERE %2.%3 = :node_id AND %2.is_finished = TRUE AND (%2.issued_time BETWEEN :start AND :end) AND %1.is_valid = TRUE;
    )");

    QString query_employee = QStringLiteral(R"(
        SELECT
            4 AS section,
            st.inside_product,
            st.unit_price,
            st.second,
            st.lhs_node,
            st.description,
            st.first,
            st.gross_amount,
            st.outside_product,
            st.discount_price,
            sn.issued_time
        FROM sales_transaction st
        INNER JOIN sales sn ON st.lhs_node = sn.id
        WHERE sn.employee = :node_id AND sn.is_finished = TRUE AND (sn.issued_time BETWEEN :start AND :end) AND st.is_valid = TRUE

        UNION ALL

        SELECT
            5 AS section,
            pt.inside_product,
            pt.unit_price,
            pt.second,
            pt.lhs_node,
            pt.description,
            pt.first,
            pt.gross_amount,
            pt.outside_product,
            pt.discount_price,
            pn.issued_time
        FROM purchase_transaction pt
        INNER JOIN purchase pn ON pt.lhs_node = pn.id
        WHERE pn.employee = :node_id AND pn.is_finished = TRUE AND (pn.issued_time BETWEEN :start AND :end) AND pt.is_valid = TRUE
    )");

    QString node {};
    QString node_trans {};
    QString column {};
    QString section {};

    switch (UnitS(unit)) {
    case UnitS::kCust:
        node_trans = QStringLiteral("sales_transaction");
        node = QStringLiteral("sales");
        column = QStringLiteral("party");
        section = QStringLiteral("4");
        break;
    case UnitS::kVend:
        node_trans = QStringLiteral("purchase_transaction");
        node = QStringLiteral("purchase");
        column = QStringLiteral("party");
        section = QStringLiteral("5");
        break;
    case UnitS::kEmp:
        return query_employee;
    default:
        return {};
    }

    return base_query.arg(node_trans, node, column, section);
}

QString SqlS::QSRemoveTrans() const
{
    return QStringLiteral(R"(
    DELETE FROM stakeholder_transaction
    WHERE id = :trans_id
    )");
}

QString SqlS::QSLeafTotal(int unit) const
{
    switch (UnitS(unit)) {
    case UnitS::kCust:
        return QStringLiteral(R"(
        SELECT SUM(gross_amount) AS final_balance
        FROM sales
        WHERE party = :node_id AND is_finished = TRUE AND settlement_id = 0 AND unit = 1 AND is_valid = TRUE;
        )");
        break;
    case UnitS::kVend:
        return QStringLiteral(R"(
        SELECT SUM(gross_amount) AS final_balance
        FROM purchase
        WHERE party = :node_id AND is_finished = TRUE AND settlement_id = 0 AND unit = 1 AND is_valid = TRUE;
        )");
        break;
    default:
        break;
    }

    return {};
}

void SqlS::ReadTransS(QSqlQuery& query)
{
    Trans* trans {};

    while (query.next()) {
        const auto kID { QUuid::fromRfc4122(query.value(QStringLiteral("id")).toByteArray()) };

        if (trans_hash_.contains(kID))
            continue;

        trans = ResourcePool<Trans>::Instance().Allocate();
        trans->id = kID;

        ReadTransQuery(trans, query);
        trans_hash_.insert(kID, trans);
    }
}

void SqlS::ReadTransRefQuery(TransList& trans_list, QSqlQuery& query) const
{
    // remind to recycle these trans
    while (query.next()) {
        auto* trans { ResourcePool<Trans>::Instance().Allocate() };

        trans->id = QUuid::fromRfc4122(query.value(QStringLiteral("section")).toByteArray());
        trans->rhs_node = QUuid::fromRfc4122(query.value(QStringLiteral("inside_product")).toByteArray());
        trans->lhs_ratio = query.value(QStringLiteral("unit_price")).toDouble();
        trans->lhs_credit = query.value(QStringLiteral("second")).toDouble();
        trans->description = query.value(QStringLiteral("description")).toString();
        trans->lhs_debit = query.value(QStringLiteral("first")).toDouble();
        trans->rhs_debit = query.value(QStringLiteral("gross_amount")).toDouble();
        trans->support_id = QUuid::fromRfc4122(query.value(QStringLiteral("outside_product")).toByteArray());
        trans->rhs_ratio = query.value(QStringLiteral("discount_price")).toDouble();
        trans->issued_time = query.value(QStringLiteral("issued_time")).toString();
        trans->lhs_node = QUuid::fromRfc4122(query.value(QStringLiteral("lhs_node")).toByteArray());

        trans_list.emplaceBack(trans);
    }
}

void SqlS::CalculateLeafTotal(Node* node, QSqlQuery& query) const
{
    if (query.next())
        node->final_total = query.value(QStringLiteral("final_balance")).toDouble();
}

void SqlS::WriteTransBind(TransShadow* trans_shadow, QSqlQuery& query) const
{
    query.bindValue(QStringLiteral(":id"), trans_shadow->id->toRfc4122());
    query.bindValue(QStringLiteral(":issued_time"), *trans_shadow->issued_time);
    query.bindValue(QStringLiteral(":code"), *trans_shadow->code);
    query.bindValue(QStringLiteral(":lhs_node"), trans_shadow->lhs_node->toRfc4122());
    query.bindValue(QStringLiteral(":unit_price"), *trans_shadow->lhs_ratio);
    query.bindValue(QStringLiteral(":description"), *trans_shadow->description);
    query.bindValue(QStringLiteral(":is_checked"), *trans_shadow->is_checked);
    query.bindValue(QStringLiteral(":document"), trans_shadow->document->join(kSemicolon));
    query.bindValue(QStringLiteral(":inside_product"), trans_shadow->rhs_node->toRfc4122());
    query.bindValue(QStringLiteral(":outside_product"), trans_shadow->support_id->toRfc4122());
}

void SqlS::ReadNodeQuery(Node* node, const QSqlQuery& query) const
{
    node->id = QUuid::fromRfc4122(query.value(QStringLiteral("id")).toByteArray());
    node->name = query.value(QStringLiteral("name")).toString();
    node->code = query.value(QStringLiteral("code")).toString();
    node->description = query.value(QStringLiteral("description")).toString();
    node->note = query.value(QStringLiteral("note")).toString();
    node->node_type = query.value(QStringLiteral("node_type")).toInt();
    node->unit = query.value(QStringLiteral("unit")).toInt();
    node->employee = QUuid::fromRfc4122(query.value(QStringLiteral("employee")).toByteArray());
    node->issued_time = query.value(QStringLiteral("deadline")).toString();
    node->first = query.value(QStringLiteral("payment_term")).toDouble();
    node->second = query.value(QStringLiteral("tax_rate")).toDouble();
    node->final_total = query.value(QStringLiteral("amount")).toDouble();
}

void SqlS::ReadTransQuery(Trans* trans, const QSqlQuery& query) const
{
    trans->support_id = QUuid::fromRfc4122(query.value(QStringLiteral("outside_product")).toByteArray());
    trans->lhs_node = QUuid::fromRfc4122(query.value(QStringLiteral("lhs_node")).toByteArray());
    trans->rhs_node = QUuid::fromRfc4122(query.value(QStringLiteral("inside_product")).toByteArray());
    trans->lhs_ratio = query.value(QStringLiteral("unit_price")).toDouble();
    trans->rhs_ratio = trans->lhs_ratio;
    trans->code = query.value(QStringLiteral("code")).toString();
    trans->description = query.value(QStringLiteral("description")).toString();
    trans->is_checked = query.value(QStringLiteral("is_checked")).toBool();
    trans->document = query.value(QStringLiteral("document")).toString().split(kSemicolon, Qt::SkipEmptyParts);
    trans->issued_time = query.value(QStringLiteral("issued_time")).toString();
}
