#include "settlement.h"

#include "component/constant.h"

void Settlement::ResetState()
{
    id = QUuid();
    partner = QUuid();
    issued_time = {};
    description.clear();
    status = 0;
    amount = 0.0;

    user_id = QUuid();
    created_time = {};
    created_by = QUuid();
    updated_time = {};
    updated_by = QUuid();
}

void Settlement::ReadJson(const QJsonObject& object)
{
    if (object.contains(kId))
        id = QUuid(object.value(kId).toString());

    if (object.contains(kPartner))
        partner = QUuid(object.value(kPartner).toString());

    if (object.contains(kIssuedTime))
        issued_time = QDateTime::fromString(object.value(kIssuedTime).toString(), Qt::ISODate);

    if (object.contains(kDescription))
        description = object.value(kDescription).toString();

    if (object.contains(kStatus))
        status = object.value(kStatus).toInt();

    if (object.contains(kAmount))
        amount = object.value(kAmount).toDouble();

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

QJsonObject Settlement::WriteJson() const
{
    QJsonObject obj {};

    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kPartner, partner.toString(QUuid::WithoutBraces));
    obj.insert(kIssuedTime, issued_time.toString(Qt::ISODate));
    obj.insert(kDescription, description);
    obj.insert(kStatus, status);
    obj.insert(kAmount, amount);

    return obj;
}

void SettlementNode::ResetState()
{
    id = QUuid();
    partner = QUuid();
    settlement_id = QUuid();
    employee = QUuid();
    issued_time = {};
    description.clear();
    amount = 0.0;
}

void SettlementNode::ReadJson(const QJsonObject& object)
{
    if (object.contains(kId))
        id = QUuid(object.value(kId).toString());

    if (object.contains(kPartner))
        partner = QUuid(object.value(kPartner).toString());

    if (object.contains(kSettlementId))
        settlement_id = QUuid(object.value(kSettlementId).toString());

    if (object.contains(kEmployee))
        employee = QUuid(object.value(kEmployee).toString());

    if (object.contains(kIssuedTime))
        issued_time = QDateTime::fromString(object.value(kIssuedTime).toString(), Qt::ISODate);

    if (object.contains(kDescription))
        description = object.value(kDescription).toString();

    if (object.contains(kAmount))
        amount = object.value(kAmount).toDouble();
}

QJsonObject SettlementNode::WriteJson() const
{
    QJsonObject obj {};
    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kPartner, partner.toString(QUuid::WithoutBraces));
    obj.insert(kSettlementId, settlement_id.toString(QUuid::WithoutBraces));
    obj.insert(kEmployee, employee.toString(QUuid::WithoutBraces));
    obj.insert(kIssuedTime, issued_time.toString(Qt::ISODate));
    obj.insert(kDescription, description);
    obj.insert(kAmount, amount);
    return obj;
}
