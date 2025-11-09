#include "node.h"

#include "component/constant.h"

void Node::ResetState()
{
    name.clear();
    id = QUuid();
    code.clear();
    description.clear();
    note.clear();
    direction_rule = false;
    kind = 0;
    unit = 0;
    final_total = 0.0;
    initial_total = 0.0;
    parent = nullptr;
    children.clear();

    user_id = QUuid();
    created_time = {};
    created_by = QUuid();
    updated_time = {};
    updated_by = QUuid();
}

void Node::InvertTotal()
{
    initial_total = -initial_total;
    final_total = -final_total;
}

void Node::ReadJson(const QJsonObject& object)
{
    if (object.contains(kName))
        name = object.value(kName).toString();
    if (object.contains(kId))
        id = QUuid(object.value(kId).toString());
    if (object.contains(kCode))
        code = object.value(kCode).toString();
    if (object.contains(kDescription))
        description = object.value(kDescription).toString();
    if (object.contains(kNote))
        note = object.value(kNote).toString();
    if (object.contains(kKind))
        kind = object.value(kKind).toInt();
    if (object.contains(kDirectionRule))
        direction_rule = object.value(kDirectionRule).toBool();
    if (object.contains(kUnit))
        unit = object.value(kUnit).toInt();
    if (object.contains(kFinalTotal))
        final_total = object.value(kFinalTotal).toString().toDouble();
    if (object.contains(kInitialTotal))
        initial_total = object.value(kInitialTotal).toString().toDouble();
    if (object.contains(kUserId))
        user_id = QUuid(object.value(kUserId).toString());
    if (object.contains(kCreatedTime))
        created_time = QDateTime::fromString(object.value(kCreatedTime).toString(), Qt::ISODate);
    if (object.contains(kCreatedBy))
        created_by = QUuid(object.value(kCreatedBy).toString());
    if (object.contains(kUpdatedTime))
        updated_time = QDateTime::fromString(object.value(kUpdatedTime).toString(), Qt::ISODate);
    if (object.contains(kUpdatedBy))
        updated_by = QUuid(object.value(kUpdatedBy).toString());
}

QJsonObject Node::WriteJson() const
{
    QJsonObject obj {};
    obj.insert(kName, name);
    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kCode, code);
    obj.insert(kDescription, description);
    obj.insert(kNote, note);
    obj.insert(kKind, kind);
    obj.insert(kDirectionRule, direction_rule);
    obj.insert(kUnit, unit);
    obj.insert(kFinalTotal, QString::number(final_total, 'f', kMaxNumericScale_4));
    obj.insert(kInitialTotal, QString::number(initial_total, 'f', kMaxNumericScale_4));
    return obj;
}

void NodeI::ResetState()
{
    Node::ResetState();
    color.clear();
    unit_price = 0.0;
    commission = 0.0;
}

void NodeI::ReadJson(const QJsonObject& object)
{
    Node::ReadJson(object);

    if (object.contains(kColor))
        color = object.value(kColor).toString();
    if (object.contains(kUnitPrice))
        unit_price = object.value(kUnitPrice).toString().toDouble();
    if (object.contains(kCommission))
        commission = object.value(kCommission).toString().toDouble();
}

QJsonObject NodeI::WriteJson() const
{
    QJsonObject obj { Node::WriteJson() };
    obj.insert(kColor, color);
    obj.insert(kUnitPrice, QString::number(unit_price, 'f', kMaxNumericScale_4));
    obj.insert(kCommission, QString::number(commission, 'f', kMaxNumericScale_4));
    return obj;
}

void NodeT::ResetState()
{
    Node::ResetState();
    color.clear();
    document.clear();
    status = 0;
}

void NodeT::ReadJson(const QJsonObject& object)
{
    Node::ReadJson(object);

    if (object.contains(kColor))
        color = object.value(kColor).toString();
    if (object.contains(kDocument))
        document = object.value(kDocument).toString().split(kSemicolon, Qt::SkipEmptyParts);
    if (object.contains(kStatus))
        status = object.value(kStatus).toInt();
    if (object.contains(kIssuedTime))
        issued_time = QDateTime::fromString(object.value(kIssuedTime).toString(), Qt::ISODate);
}

QJsonObject NodeT::WriteJson() const
{
    QJsonObject obj = Node::WriteJson();

    obj.insert(kColor, color);
    obj.insert(kDocument, document.join(kSemicolon));
    obj.insert(kIssuedTime, issued_time.toString(Qt::ISODate));
    obj.insert(kStatus, status);

    return obj;
}

void NodeP::ResetState()
{
    Node::ResetState();
    payment_term = 0;
}

void NodeP::ReadJson(const QJsonObject& object)
{
    if (object.contains(kName))
        name = object.value(kName).toString();
    if (object.contains(kId))
        id = QUuid(object.value(kId).toString());
    if (object.contains(kCode))
        code = object.value(kCode).toString();
    if (object.contains(kDescription))
        description = object.value(kDescription).toString();
    if (object.contains(kNote))
        note = object.value(kNote).toString();
    if (object.contains(kKind))
        kind = object.value(kKind).toInt();
    if (object.contains(kUnit))
        unit = object.value(kUnit).toInt();
    if (object.contains(kInitialTotal))
        initial_total = object.value(kInitialTotal).toString().toDouble();
    if (object.contains(kUserId))
        user_id = QUuid(object.value(kUserId).toString());
    if (object.contains(kCreatedTime))
        created_time = QDateTime::fromString(object.value(kCreatedTime).toString(), Qt::ISODate);
    if (object.contains(kCreatedBy))
        created_by = QUuid(object.value(kCreatedBy).toString());
    if (object.contains(kUpdatedTime))
        updated_time = QDateTime::fromString(object.value(kUpdatedTime).toString(), Qt::ISODate);
    if (object.contains(kUpdatedBy))
        updated_by = QUuid(object.value(kUpdatedBy).toString());
    if (object.contains(kPaymentTerm))
        payment_term = object.value(kPaymentTerm).toInt();
}

QJsonObject NodeP::WriteJson() const
{
    QJsonObject obj {};
    obj.insert(kName, name);
    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kCode, code);
    obj.insert(kDescription, description);
    obj.insert(kNote, note);
    obj.insert(kKind, kind);
    obj.insert(kUnit, unit);
    obj.insert(kInitialTotal, QString::number(initial_total, 'f', kMaxNumericScale_4));
    obj.insert(kPaymentTerm, payment_term);

    return obj;
}

void NodeO::ResetState()
{
    Node::ResetState();
    employee = QUuid();
    partner = QUuid();
    issued_time = {};
    count_total = 0.0;
    measure_total = 0.0;
    discount_total = 0.0;
    status = 0;
}

void NodeO::InvertTotal()
{
    count_total = -count_total;
    measure_total = -measure_total;
    discount_total = -discount_total;
    initial_total = -initial_total;
    final_total = -final_total;
}

void NodeO::ReadJson(const QJsonObject& object)
{
    if (object.contains(kName))
        name = object.value(kName).toString();
    if (object.contains(kId))
        id = QUuid(object.value(kId).toString());
    if (object.contains(kDescription))
        description = object.value(kDescription).toString();
    if (object.contains(kKind))
        kind = object.value(kKind).toInt();
    if (object.contains(kDirectionRule))
        direction_rule = object.value(kDirectionRule).toBool();
    if (object.contains(kUnit))
        unit = object.value(kUnit).toInt();
    if (object.contains(kFinalTotal))
        final_total = object.value(kFinalTotal).toString().toDouble();
    if (object.contains(kInitialTotal))
        initial_total = object.value(kInitialTotal).toString().toDouble();
    if (object.contains(kUserId))
        user_id = QUuid(object.value(kUserId).toString());
    if (object.contains(kCreatedTime))
        created_time = QDateTime::fromString(object.value(kCreatedTime).toString(), Qt::ISODate);
    if (object.contains(kCreatedBy))
        created_by = QUuid(object.value(kCreatedBy).toString());
    if (object.contains(kUpdatedTime))
        updated_time = QDateTime::fromString(object.value(kUpdatedTime).toString(), Qt::ISODate);
    if (object.contains(kUpdatedBy))
        updated_by = QUuid(object.value(kUpdatedBy).toString());
    if (object.contains(kEmployee))
        employee = QUuid(object.value(kEmployee).toString());
    if (object.contains(kPartner))
        partner = QUuid(object.value(kPartner).toString());
    if (object.contains(kIssuedTime))
        issued_time = QDateTime::fromString(object.value(kIssuedTime).toString(), Qt::ISODate);
    if (object.contains(kCountTotal))
        count_total = object.value(kCountTotal).toString().toDouble();
    if (object.contains(kMeasureTotal))
        measure_total = object.value(kMeasureTotal).toString().toDouble();
    if (object.contains(kDiscountTotal))
        discount_total = object.value(kDiscountTotal).toString().toDouble();
    if (object.contains(kStatus))
        status = object.value(kStatus).toInt();
    if (object.contains(kSettlement))
        settlement = QUuid(object.value(kSettlement).toString());
}

QJsonObject NodeO::WriteJson() const
{
    QJsonObject obj {};
    obj.insert(kName, name);
    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kDescription, description);
    obj.insert(kKind, kind);
    obj.insert(kDirectionRule, direction_rule);
    obj.insert(kUnit, unit);
    obj.insert(kFinalTotal, QString::number(final_total, 'f', kMaxNumericScale_4));
    obj.insert(kInitialTotal, QString::number(initial_total, 'f', kMaxNumericScale_4));
    obj.insert(kEmployee, employee.toString(QUuid::WithoutBraces));
    obj.insert(kPartner, partner.toString(QUuid::WithoutBraces));
    obj.insert(kSettlement, settlement.toString(QUuid::WithoutBraces));
    obj.insert(kIssuedTime, issued_time.toString(Qt::ISODate));
    obj.insert(kCountTotal, QString::number(count_total, 'f', kMaxNumericScale_4));
    obj.insert(kMeasureTotal, QString::number(measure_total, 'f', kMaxNumericScale_4));
    obj.insert(kDiscountTotal, QString::number(discount_total, 'f', kMaxNumericScale_4));
    obj.insert(kStatus, status);

    return obj;
}
