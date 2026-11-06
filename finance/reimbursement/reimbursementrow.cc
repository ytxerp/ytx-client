#include "reimbursementrow.h"

#include <QJsonArray>

#include "component/constant.h"
#include "component/constantint.h"
#include "utils/entryutils.h"

void reimbursement::PrimaryRow::Reset() { *this = PrimaryRow {}; }

void reimbursement::PrimaryRow::ReadJson(const QJsonObject& object)
{
    sync_state = SyncState::kSynced;

    if (const auto val = object.value(kId); val.isString())
        id = QUuid(val.toString());
    if (const auto val = object.value(kUsername); val.isString())
        username = val.toString();
    if (const auto val = object.value(kIssuedTime); val.isString())
        issued_time = QDateTime::fromString(val.toString(), Qt::ISODate).toLocalTime();
    if (const auto val = object.value(kDescription); val.isString())
        description = val.toString();
    if (const auto val = object.value(kStatus); val.isDouble())
        status = static_cast<Status>(val.toInt());
    if (const auto val = object.value(kInitialTotal); val.isString())
        initial_total = val.toString().toDouble();
    if (const auto val = object.value(kVersion); val.isDouble())
        version = val.toInt();
}

QJsonObject reimbursement::PrimaryRow::WriteJson() const
{
    QJsonObject obj {};

    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kIssuedTime, issued_time.toUTC().toString(Qt::ISODate));
    obj.insert(kDescription, description);
    obj.insert(kStatus, std::to_underlying(status));
    obj.insert(kInitialTotal, QString::number(initial_total, 'f', numeric_const::kDecimalPlaces4));
    obj.insert(kVersion, version);

    return obj;
}

void reimbursement::SecondaryRow::Reset() { *this = SecondaryRow {}; }

void reimbursement::SecondaryRow::ReadJson(const QJsonObject& object)
{
    sync_state = SyncState::kSynced;

    if (const auto val = object.value(kId); val.isString())
        id = QUuid(val.toString());
    if (const auto val = object.value(kLhsNode); val.isString())
        lhs_node = QUuid(val.toString());
    if (const auto val = object.value(kDebitNode); val.isString())
        debit_node = QUuid(val.toString());
    if (const auto val = object.value(kCreditNode); val.isString())
        credit_node = QUuid(val.toString());
    if (const auto val = object.value(kDescription); val.isString())
        description = val.toString();
    if (object.value(kDocument).isArray())
        document = utils::ReadStringList(object, kDocument);
    if (const auto val = object.value(kInitial); val.isString())
        initial = val.toString().toDouble();
    if (const auto val = object.value(kVersion); val.isDouble())
        version = val.toInt();
}

QJsonObject reimbursement::SecondaryRow::WriteJson() const
{
    QJsonObject obj {};

    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kLhsNode, lhs_node.toString(QUuid::WithoutBraces));
    obj.insert(kDebitNode, debit_node.toString(QUuid::WithoutBraces));
    obj.insert(kCreditNode, credit_node.toString(QUuid::WithoutBraces));
    obj.insert(kDescription, description);
    obj.insert(kDocument, QJsonArray::fromStringList(document));
    obj.insert(kInitial, QString::number(initial, 'f', numeric_const::kDecimalPlaces8));
    obj.insert(kVersion, version);

    return obj;
}