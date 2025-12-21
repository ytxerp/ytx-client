#include "settlement.h"

#include "component/constant.h"

void Settlement::ResetState()
{
    id = QUuid();
    partner_id = QUuid();
    issued_time = {};
    description.clear();
    status = SettlementStatus::kRecalled;
    amount = 0.0;

    user_id = QUuid();
    created_time = {};
    created_by = QUuid();
    updated_time = {};
    updated_by = QUuid();
    version = 0;
}

void Settlement::ReadJson(const QJsonObject& object)
{
    if (object.contains(kId))
        id = QUuid(object.value(kId).toString());

    if (object.contains(kPartnerId))
        partner_id = QUuid(object.value(kPartnerId).toString());

    if (object.contains(kIssuedTime))
        issued_time = QDateTime::fromString(object.value(kIssuedTime).toString(), Qt::ISODate);

    if (object.contains(kDescription))
        description = object.value(kDescription).toString();

    if (object.contains(kStatus))
        status = SettlementStatus(object.value(kStatus).toInt());

    if (object.contains(kAmount))
        amount = object.value(kAmount).toString().toDouble();

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

    if (object.contains(kVersion))
        version = object.value(kVersion).toInt();
}

QJsonObject Settlement::WriteJson() const
{
    QJsonObject obj {};

    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kPartnerId, partner_id.toString(QUuid::WithoutBraces));
    obj.insert(kIssuedTime, issued_time.toString(Qt::ISODate));
    obj.insert(kDescription, description);
    obj.insert(kStatus, std::to_underlying(status));
    obj.insert(kAmount, QString::number(amount, 'f', kMaxNumericScale_4));

    return obj;
}

void SettlementItem::ResetState()
{
    id = QUuid();
    employee_id = QUuid();
    issued_time = {};
    description.clear();
    amount = 0.0;
    is_selected = false;
}

void SettlementItem::ReadJson(const QJsonObject& object)
{
    if (object.contains(kId))
        id = QUuid(object.value(kId).toString());

    if (object.contains(kEmployeeId))
        employee_id = QUuid(object.value(kEmployeeId).toString());

    if (object.contains(kIssuedTime))
        issued_time = QDateTime::fromString(object.value(kIssuedTime).toString(), Qt::ISODate);

    if (object.contains(kDescription))
        description = object.value(kDescription).toString();

    if (object.contains(kAmount))
        amount = object.value(kAmount).toString().toDouble();

    if (object.contains(kIsSelected))
        is_selected = object.value(kIsSelected).toBool();
}
