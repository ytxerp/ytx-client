#include "settlement.h"

#include "component/constant.h"

void Settlement::Reset() { *this = Settlement {}; }

void Settlement::ReadJson(const QJsonObject& object)
{
    if (const auto val = object.value(kId); val.isString())
        id = QUuid(val.toString());
    if (const auto val = object.value(kPartnerId); val.isString())
        partner_id = QUuid(val.toString());
    if (const auto val = object.value(kIssuedTime); val.isString())
        issued_time = QDateTime::fromString(val.toString(), Qt::ISODate);
    if (const auto val = object.value(kDescription); val.isString())
        description = val.toString();
    if (const auto val = object.value(kStatus); val.isDouble())
        status = static_cast<SettlementStatus>(val.toInt());
    if (const auto val = object.value(kAmount); val.isString())
        amount = val.toString().toDouble();
    if (const auto val = object.value(kUserId); val.isString())
        user_id = QUuid(val.toString());
    if (const auto val = object.value(kCreatedTime); val.isString())
        created_time = QDateTime::fromString(val.toString(), Qt::ISODate);
    if (const auto val = object.value(kCreatedBy); val.isString())
        created_by = QUuid(val.toString());
    if (const auto val = object.value(kUpdatedTime); val.isString())
        updated_time = QDateTime::fromString(val.toString(), Qt::ISODate);
    if (const auto val = object.value(kUpdatedBy); val.isString())
        updated_by = QUuid(val.toString());
    if (const auto val = object.value(kVersion); val.isDouble())
        version = val.toInt();
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

void SettlementItem::Reset() { *this = SettlementItem {}; }

void SettlementItem::ReadJson(const QJsonObject& object)
{
    if (const auto val = object.value(kId); val.isString())
        id = QUuid(val.toString());
    if (const auto val = object.value(kEmployeeId); val.isString())
        employee_id = QUuid(val.toString());
    if (const auto val = object.value(kIssuedTime); val.isString())
        issued_time = QDateTime::fromString(val.toString(), Qt::ISODate);
    if (const auto val = object.value(kDescription); val.isString())
        description = val.toString();
    if (const auto val = object.value(kAmount); val.isString())
        amount = val.toString().toDouble();
    if (const auto val = object.value(kIsSettled); val.isBool())
        is_settled = val.toBool();
}
