#include "sqlt.h"

#include <QSqlQuery>

#include "component/constvalue.h"

SqlT::SqlT(CInfo& info, QObject* parent)
    : Sql(info, parent)
{
}

QString SqlT::QSReadNode() const
{
    return QStringLiteral(R"(
    SELECT name, id, code, description, note, direction_rule, node_type, unit, color, document, issued_time, is_finished, unit_cost, quantity, amount
    FROM task
    WHERE is_valid = TRUE
    )");
}

QString SqlT::QSWriteNode() const
{
    return QStringLiteral(R"(
    INSERT INTO task (name, id, code, description, note, direction_rule, node_type, unit, color, document, issued_time, is_finished, unit_cost)
    VALUES (:name, :id, :code, :description, :note, :direction_rule, :node_type, :unit, :color, :document, :issued_time, :is_finished, :unit_cost)
    )");
}

QString SqlT::QSRemoveNodeSecond() const
{
    return QStringLiteral(R"(
    UPDATE task_transaction SET
        is_valid = FALSE
    WHERE (lhs_node = :node_id OR rhs_node = :node_id) AND is_valid = TRUE
    )");
}

QString SqlT::QSInternalReference() const
{
    return QStringLiteral(R"(
    SELECT EXISTS(
        SELECT 1 FROM task_transaction
        WHERE (lhs_node = :node_id OR rhs_node = :node_id) AND is_valid = TRUE
    ) AS is_referenced
    )");
}

QString SqlT::QSSupportReference() const
{
    return QStringLiteral(R"(
    SELECT 1 FROM task_transaction
    WHERE support_id = :support_id AND is_valid = TRUE
    LIMIT 1
    )");
}

QString SqlT::QSReplaceSupport() const
{
    return QStringLiteral(R"(
    UPDATE task_transaction SET
        support_id = :new_node_id
    WHERE support_id = :old_node_id AND is_valid = TRUE
    )");
}

QString SqlT::QSRemoveSupport() const
{
    return QStringLiteral(R"(
    UPDATE task_transaction SET
        support_id = 0
    WHERE support_id = :node_id AND is_valid = TRUE
    )");
}

QString SqlT::QSLeafTotal(int /*unit*/) const
{
    return QStringLiteral(R"(
    WITH node_balance AS (
        SELECT
            lhs_debit AS initial_debit,
            lhs_credit AS initial_credit,
            unit_cost * lhs_debit AS final_debit,
            unit_cost * lhs_credit AS final_credit
        FROM task_transaction
        WHERE lhs_node = :node_id AND is_valid = TRUE

        UNION ALL

        SELECT
            rhs_debit,
            rhs_credit,
            unit_cost * rhs_debit,
            unit_cost * rhs_credit
        FROM task_transaction
        WHERE rhs_node = :node_id AND is_valid = TRUE
    )
    SELECT
        SUM(initial_credit) - SUM(initial_debit) AS initial_balance,
        SUM(final_credit) - SUM(final_debit) AS final_balance
    FROM node_balance;
    )");
}

QString SqlT::QSTransToRemove() const
{
    return QStringLiteral(R"(
    SELECT rhs_node AS node_id, id AS trans_id, support_id FROM task_transaction
    WHERE lhs_node = :node_id AND is_valid = TRUE
    UNION ALL
    SELECT lhs_node AS node_id, id AS trans_id, support_id FROM task_transaction
    WHERE rhs_node = :node_id AND is_valid = TRUE
    )");
}

QString SqlT::QSReadTrans() const
{
    return QStringLiteral(R"(
    SELECT id, lhs_node, unit_cost, lhs_debit, lhs_credit, rhs_node, rhs_debit, rhs_credit, is_checked, description, support_id, code, document, issued_time
    FROM task_transaction
    WHERE (lhs_node = :node_id OR rhs_node = :node_id) AND is_valid = TRUE
    )");
}

QString SqlT::QSReadSupportTrans() const
{
    return QStringLiteral(R"(
    SELECT id, lhs_node, unit_cost, lhs_debit, lhs_credit, rhs_node, rhs_debit, rhs_credit, is_checked, description, support_id, code, document, issued_time
    FROM task_transaction
    WHERE support_id = :node_id AND is_valid = TRUE
    )");
}

void SqlT::ReadTransQuery(Trans* trans, const QSqlQuery& query) const
{
    trans->lhs_node = QUuid::fromRfc4122(query.value(QStringLiteral("lhs_node")).toByteArray());
    trans->lhs_debit = query.value(QStringLiteral("lhs_debit")).toDouble();
    trans->lhs_credit = query.value(QStringLiteral("lhs_credit")).toDouble();

    trans->rhs_node = QUuid::fromRfc4122(query.value(QStringLiteral("rhs_node")).toByteArray());
    trans->rhs_debit = query.value(QStringLiteral("rhs_debit")).toDouble();
    trans->rhs_credit = query.value(QStringLiteral("rhs_credit")).toDouble();

    trans->lhs_ratio = query.value(QStringLiteral("unit_cost")).toDouble();
    trans->rhs_ratio = trans->lhs_ratio;
    trans->code = query.value(QStringLiteral("code")).toString();
    trans->description = query.value(QStringLiteral("description")).toString();
    trans->document = query.value(QStringLiteral("document")).toString().split(kSemicolon, Qt::SkipEmptyParts);
    trans->issued_time = query.value(QStringLiteral("issued_time")).toString();
    trans->is_checked = query.value(QStringLiteral("is_checked")).toBool();
    trans->support_id = QUuid::fromRfc4122(query.value(QStringLiteral("support_id")).toByteArray());
}

QString SqlT::QSWriteTrans() const
{
    return QStringLiteral(R"(
    INSERT INTO task_transaction
    (id, issued_time, lhs_node, unit_cost, lhs_debit, lhs_credit, rhs_node, rhs_debit, rhs_credit, is_checked, description, support_id, code, document)
    VALUES
    (:id, :issued_time, :lhs_node, :unit_cost, :lhs_debit, :lhs_credit, :rhs_node, :rhs_debit, :rhs_credit, :is_checked, :description, :support_id, :code, :document)
    )");
}

QString SqlT::QSReplaceLeaf() const
{
    return QStringLiteral(R"(
    UPDATE task_transaction SET
        lhs_node = CASE WHEN lhs_node = :old_node_id AND rhs_node != :new_node_id THEN :new_node_id ELSE lhs_node END,
        rhs_node = CASE WHEN rhs_node = :old_node_id AND lhs_node != :new_node_id THEN :new_node_id ELSE rhs_node END
    WHERE lhs_node = :old_node_id OR rhs_node = :old_node_id;
    )");
}

QString SqlT::QSUpdateTransValue() const
{
    return QStringLiteral(R"(
    UPDATE task_transaction SET
        lhs_node = :lhs_node, lhs_debit = :lhs_debit, lhs_credit = :lhs_credit,
        rhs_node = :rhs_node, rhs_debit = :rhs_debit, rhs_credit = :rhs_credit
    WHERE id = :trans_id
    )");
}

void SqlT::WriteTransBind(TransShadow* trans_shadow, QSqlQuery& query) const
{
    query.bindValue(QStringLiteral(":id"), trans_shadow->id->toRfc4122());
    query.bindValue(QStringLiteral(":issued_time"), *trans_shadow->issued_time);
    query.bindValue(QStringLiteral(":unit_cost"), *trans_shadow->lhs_ratio);
    query.bindValue(QStringLiteral(":is_checked"), *trans_shadow->is_checked);
    query.bindValue(QStringLiteral(":description"), *trans_shadow->description);
    query.bindValue(QStringLiteral(":code"), *trans_shadow->code);
    query.bindValue(QStringLiteral(":document"), trans_shadow->document->join(kSemicolon));
    query.bindValue(QStringLiteral(":support_id"), trans_shadow->support_id->toRfc4122());

    query.bindValue(QStringLiteral(":lhs_node"), trans_shadow->lhs_node->toRfc4122());
    query.bindValue(QStringLiteral(":lhs_debit"), *trans_shadow->lhs_debit);
    query.bindValue(QStringLiteral(":lhs_credit"), *trans_shadow->lhs_credit);

    query.bindValue(QStringLiteral(":rhs_node"), trans_shadow->rhs_node->toRfc4122());
    query.bindValue(QStringLiteral(":rhs_debit"), *trans_shadow->rhs_debit);
    query.bindValue(QStringLiteral(":rhs_credit"), *trans_shadow->rhs_credit);
}

void SqlT::UpdateTransValueBind(const TransShadow* trans_shadow, QSqlQuery& query) const
{
    query.bindValue(QStringLiteral(":lhs_node"), trans_shadow->lhs_node->toRfc4122());
    query.bindValue(QStringLiteral(":lhs_debit"), *trans_shadow->lhs_debit);
    query.bindValue(QStringLiteral(":lhs_credit"), *trans_shadow->lhs_credit);
    query.bindValue(QStringLiteral(":rhs_node"), trans_shadow->rhs_node->toRfc4122());
    query.bindValue(QStringLiteral(":rhs_debit"), *trans_shadow->rhs_debit);
    query.bindValue(QStringLiteral(":rhs_credit"), *trans_shadow->rhs_credit);
    query.bindValue(QStringLiteral(":trans_id"), trans_shadow->id->toRfc4122());
}

void SqlT::WriteNodeBind(Node* node, QSqlQuery& query) const
{
    query.bindValue(QStringLiteral(":name"), node->name);
    query.bindValue(QStringLiteral(":id"), node->id.toRfc4122());
    query.bindValue(QStringLiteral(":code"), node->code);
    query.bindValue(QStringLiteral(":description"), node->description);
    query.bindValue(QStringLiteral(":note"), node->note);
    query.bindValue(QStringLiteral(":direction_rule"), node->direction_rule);
    query.bindValue(QStringLiteral(":node_type"), node->node_type);
    query.bindValue(QStringLiteral(":unit"), node->unit);
    query.bindValue(QStringLiteral(":color"), node->color);
    query.bindValue(QStringLiteral(":issued_time"), node->issued_time);
    query.bindValue(QStringLiteral(":unit_cost"), node->first);
    query.bindValue(QStringLiteral(":is_finished"), node->is_finished);
    query.bindValue(QStringLiteral(":document"), node->document.join(kSemicolon));
}

void SqlT::ReadNodeQuery(Node* node, const QSqlQuery& query) const
{
    node->id = QUuid::fromRfc4122(query.value(QStringLiteral("id")).toByteArray());
    node->name = query.value(QStringLiteral("name")).toString();
    node->code = query.value(QStringLiteral("code")).toString();
    node->description = query.value(QStringLiteral("description")).toString();
    node->note = query.value(QStringLiteral("note")).toString();
    node->direction_rule = query.value(QStringLiteral("direction_rule")).toBool();
    node->node_type = query.value(QStringLiteral("node_type")).toInt();
    node->unit = query.value(QStringLiteral("unit")).toInt();
    node->initial_total = query.value(QStringLiteral("quantity")).toDouble();
    node->final_total = query.value(QStringLiteral("amount")).toDouble();
    node->color = query.value(QStringLiteral("color")).toString();
    node->first = query.value(QStringLiteral("unit_cost")).toDouble();
    node->issued_time = query.value(QStringLiteral("issued_time")).toString();
    node->is_finished = query.value(QStringLiteral("is_finished")).toBool();
    node->document = query.value(QStringLiteral("document")).toString().split(kSemicolon, Qt::SkipEmptyParts);
}

QString SqlT::QSUpdateLeafValue() const
{
    return QStringLiteral(R"(
    UPDATE task SET
        quantity = :quantity, amount = :amount
    WHERE id = :node_id
    )");
}

void SqlT::UpdateLeafValueBind(const Node* node, QSqlQuery& query) const
{
    query.bindValue(QStringLiteral(":quantity"), node->initial_total);
    query.bindValue(QStringLiteral(":amount"), node->final_total);
    query.bindValue(QStringLiteral(":node_id"), node->id.toRfc4122());
}

QString SqlT::QSSearchTransValue() const
{
    return QStringLiteral(R"(
    SELECT id, lhs_node, unit_cost, lhs_debit, lhs_credit, rhs_node, rhs_debit, rhs_credit, is_checked, description, support_id, code, document, issued_time
    FROM task_transaction
    WHERE ((lhs_debit BETWEEN :value - :tolerance AND :value + :tolerance)
        OR (lhs_credit BETWEEN :value - :tolerance AND :value + :tolerance)
        OR (rhs_debit BETWEEN :value - :tolerance AND :value + :tolerance)
        OR (rhs_credit BETWEEN :value - :tolerance AND :value + :tolerance))
        AND is_valid = TRUE
    )");
}

QString SqlT::QSSearchTransText() const
{
    return QStringLiteral(R"(
    SELECT id, lhs_node, unit_cost, lhs_debit, lhs_credit, rhs_node, rhs_debit, rhs_credit, is_checked, description, support_id, code, document, issued_time
    FROM task_transaction
    WHERE description LIKE :description AND is_valid = TRUE
    )");
}
