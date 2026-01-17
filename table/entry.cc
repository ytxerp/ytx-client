#include "entry.h"

#include <QJsonObject>

#include "component/constant.h"

void Entry::ResetState()
{
    id = QUuid();
    issued_time = {};
    code.clear();
    lhs_node = QUuid();
    description.clear();
    rhs_node = QUuid();
    status = 0;
    document.clear();
    lhs_rate = 0.0;
    rhs_rate = 0.0;
    lhs_debit = 0.0;
    lhs_credit = 0.0;
    rhs_debit = 0.0;
    rhs_credit = 0.0;

    user_id = QUuid();
    created_time = {};
    created_by = QUuid();
    updated_time = {};
    updated_by = QUuid();
    version = 0;
}

void Entry::ReadJson(const QJsonObject& object)
{
    if (object.contains(kId))
        id = QUuid(object[kId].toString());
    if (object.contains(kIssuedTime))
        issued_time = QDateTime::fromString(object[kIssuedTime].toString(), Qt::ISODate);
    if (object.contains(kCode))
        code = object[kCode].toString();
    if (object.contains(kLhsNode))
        lhs_node = QUuid(object[kLhsNode].toString());
    if (object.contains(kDescription))
        description = object[kDescription].toString();
    if (object.contains(kDocument))
        document = object[kDocument].toString().split(kSemicolon, Qt::SkipEmptyParts);
    if (object.contains(kStatus))
        status = object[kStatus].toInt();
    if (object.contains(kRhsNode))
        rhs_node = QUuid(object[kRhsNode].toString());
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
    if (object.contains(kLhsRate))
        lhs_rate = object[kLhsRate].toString().toDouble();
    if (object.contains(kRhsRate))
        rhs_rate = object[kRhsRate].toString().toDouble();
    if (object.contains(kLhsDebit))
        lhs_debit = object[kLhsDebit].toString().toDouble();
    if (object.contains(kLhsCredit))
        lhs_credit = object[kLhsCredit].toString().toDouble();
    if (object.contains(kRhsDebit))
        rhs_debit = object[kRhsDebit].toString().toDouble();
    if (object.contains(kRhsCredit))
        rhs_credit = object[kRhsCredit].toString().toDouble();
}

void EntryP::ResetState()
{
    Entry::ResetState();

    unit_price = 0.0;
    external_sku = QUuid();
}

void EntryP::ReadJson(const QJsonObject& object)
{
    if (object.contains(kId))
        id = QUuid(object[kId].toString());
    if (object.contains(kIssuedTime))
        issued_time = QDateTime::fromString(object[kIssuedTime].toString(), Qt::ISODate);
    if (object.contains(kCode))
        code = object[kCode].toString();
    if (object.contains(kLhsNode))
        lhs_node = QUuid(object[kLhsNode].toString());
    if (object.contains(kDescription))
        description = object[kDescription].toString();
    if (object.contains(kDocument))
        document = object[kDocument].toString().split(kSemicolon, Qt::SkipEmptyParts);
    if (object.contains(kStatus))
        status = object[kStatus].toInt();
    if (object.contains(kRhsNode))
        rhs_node = QUuid(object[kRhsNode].toString());
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
    if (object.contains(kUnitPrice))
        unit_price = object[kUnitPrice].toString().toDouble();
    if (object.contains(kExternalSku))
        external_sku = QUuid(object[kExternalSku].toString());
}

QJsonObject EntryP::WriteJson() const
{
    QJsonObject obj {};

    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kIssuedTime, issued_time.toString(Qt::ISODate));
    obj.insert(kCode, code);
    obj.insert(kLhsNode, lhs_node.toString(QUuid::WithoutBraces));
    obj.insert(kDescription, description);
    obj.insert(kDocument, document.join(kSemicolon));
    obj.insert(kStatus, status);
    obj.insert(kRhsNode, rhs_node.toString(QUuid::WithoutBraces));

    obj.insert(kUnitPrice, QString::number(unit_price, 'f', kMaxNumericScale_8));
    obj.insert(kExternalSku, external_sku.toString(QUuid::WithoutBraces));
    return obj;
}

void EntryO::ResetState()
{
    Entry::ResetState();

    unit_price = 0.0;

    count = 0.0;
    measure = 0.0;

    initial = 0.0;
    final = 0.0;
    discount = 0.0;
    unit_discount = 0.0;
}

// Note: Fields like issued_time, document, code and status are ignored for Order entries
void EntryO::ReadJson(const QJsonObject& object)
{
    if (object.contains(kId))
        id = QUuid(object[kId].toString());
    if (object.contains(kDescription))
        description = object[kDescription].toString();
    if (object.contains(kLhsNode))
        lhs_node = QUuid(object[kLhsNode].toString());
    if (object.contains(kRhsNode))
        rhs_node = QUuid(object[kRhsNode].toString());
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
    if (object.contains(kUnitPrice))
        unit_price = object[kUnitPrice].toString().toDouble();
    if (object.contains(kCount))
        count = object[kCount].toString().toDouble();
    if (object.contains(kMeasure))
        measure = object[kMeasure].toString().toDouble();
    if (object.contains(kInitial))
        initial = object[kInitial].toString().toDouble();
    if (object.contains(kFinal))
        final = object[kFinal].toString().toDouble();
    if (object.contains(kDiscount))
        discount = object[kDiscount].toString().toDouble();
    if (object.contains(kUnitDiscount))
        unit_discount = object[kUnitDiscount].toString().toDouble();
    if (object.contains(kVersion))
        version = object.value(kVersion).toInt();
}

QJsonObject EntryO::WriteJson() const
{
    QJsonObject obj {};

    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kDescription, description);
    obj.insert(kLhsNode, lhs_node.toString(QUuid::WithoutBraces));
    obj.insert(kRhsNode, rhs_node.toString(QUuid::WithoutBraces));

    obj.insert(kUnitPrice, QString::number(unit_price, 'f', kMaxNumericScale_8));
    obj.insert(kUnitDiscount, QString::number(unit_discount, 'f', kMaxNumericScale_8));
    obj.insert(kCount, QString::number(count, 'f', kMaxNumericScale_8));
    obj.insert(kMeasure, QString::number(measure, 'f', kMaxNumericScale_8));
    obj.insert(kInitial, QString::number(initial, 'f', kMaxNumericScale_4));
    obj.insert(kFinal, QString::number(final, 'f', kMaxNumericScale_4));
    obj.insert(kDiscount, QString::number(discount, 'f', kMaxNumericScale_4));

    return obj;
}
