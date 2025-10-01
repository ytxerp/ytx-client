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
    mark_status = false;
    document.clear();

    user_id = QUuid();
    created_time = {};
    created_by = QUuid();
    updated_time = {};
    updated_by = QUuid();
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
    if (object.contains(kMarkStatus))
        mark_status = object[kMarkStatus].toBool();
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
}

void EntryF::ResetState()
{
    Entry::ResetState();

    lhs_rate = 1.0;
    rhs_rate = 1.0;
    lhs_debit = 0.0;
    lhs_credit = 0.0;
    rhs_debit = 0.0;
    rhs_credit = 0.0;
}

void EntryF::ReadJson(const QJsonObject& object)
{
    Entry::ReadJson(object);

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

void EntryI::ResetState()
{
    Entry::ResetState();

    unit_cost = 0.0;
    lhs_debit = 0.0;
    lhs_credit = 0.0;
    rhs_debit = 0.0;
    rhs_credit = 0.0;
}

void EntryI::ReadJson(const QJsonObject& object)
{
    Entry::ReadJson(object);

    if (object.contains(kUnitCost))
        unit_cost = object[kUnitCost].toString().toDouble();
    if (object.contains(kLhsDebit))
        lhs_debit = object[kLhsDebit].toString().toDouble();
    if (object.contains(kLhsCredit))
        lhs_credit = object[kLhsCredit].toString().toDouble();
    if (object.contains(kRhsDebit))
        rhs_debit = object[kRhsDebit].toString().toDouble();
    if (object.contains(kRhsCredit))
        rhs_credit = object[kRhsCredit].toString().toDouble();
}

void EntryT::ResetState()
{
    Entry::ResetState();

    unit_cost = 0.0;
    lhs_debit = 0.0;
    lhs_credit = 0.0;
    rhs_debit = 0.0;
    rhs_credit = 0.0;
}

void EntryT::ReadJson(const QJsonObject& object)
{
    Entry::ReadJson(object);

    if (object.contains(kUnitCost))
        unit_cost = object[kUnitCost].toString().toDouble();
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
    Entry::ReadJson(object);

    if (object.contains(kUnitPrice))
        unit_price = object[kUnitPrice].toString().toDouble();
    if (object.contains(kExternalSku))
        external_sku = QUuid(object[kExternalSku].toString());
}

void EntryO::ResetState()
{
    Entry::ResetState();

    unit_price = 0.0;
    external_sku = QUuid();

    count = 0.0;
    measure = 0.0;

    initial = 0.0;
    final = 0.0;
    discount = 0.0;
    discount_price = 0.0;
}

// Note: Fields like issued_time, document, code and mark_status are ignored for Order entries
void EntryO::ReadJson(const QJsonObject& object)
{
    if (object.contains(kId))
        id = QUuid(object[kId].toString());
    if (object.contains(kDescription))
        code = object[kDescription].toString();
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
    if (object.contains(kExternalSku))
        external_sku = QUuid(object[kExternalSku].toString());
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
    if (object.contains(kDiscountPrice))
        discount_price = object[kDiscountPrice].toString().toDouble();
}
