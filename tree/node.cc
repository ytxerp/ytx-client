#include "node.h"

#include "component/constant.h"
#include "component/constantint.h"
#include "utils/entryutils.h"

void Node::Reset() { *this = Node {}; }

void Node::InvertTotal()
{
    initial_total = -initial_total;
    final_total = -final_total;
}

void Node::ReadJson(const QJsonObject& object)
{
    if (const auto val = object.value(kName); val.isString())
        name = val.toString();
    if (const auto val = object.value(kId); val.isString())
        id = QUuid(val.toString());
    if (const auto val = object.value(kCode); val.isString())
        code = val.toString();
    if (const auto val = object.value(kDescription); val.isString())
        description = val.toString();
    if (const auto val = object.value(kKind); val.isDouble())
        kind = static_cast<NodeKind>(val.toInt());
    if (const auto val = object.value(kDirectionRule); val.isBool())
        direction_rule = val.toBool();
    if (const auto val = object.value(kUnit); val.isDouble())
        unit = static_cast<NodeUnit>(val.toInt());
    if (const auto val = object.value(kFinalTotal); val.isString())
        final_total = val.toString().toDouble();
    if (const auto val = object.value(kInitialTotal); val.isString())
        initial_total = val.toString().toDouble();
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
    if (const auto val = object.value(kColor); val.isString())
        color = val.toString();
    if (object.value(kTag).isArray())
        tag = Utils::ReadStringList(object, kTag);
    if (object.value(kDocument).isArray())
        document = Utils::ReadStringList(object, kDocument);
}

QJsonObject Node::WriteJson() const
{
    QJsonObject obj {};
    obj.insert(kName, name);
    obj.insert(kColor, color);
    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kCode, code);
    obj.insert(kDescription, description);
    obj.insert(kKind, std::to_underlying(kind));
    obj.insert(kDirectionRule, direction_rule);
    obj.insert(kUnit, std::to_underlying(unit));
    obj.insert(kFinalTotal, QString::number(final_total, 'f', NumericConst::kDecimalPlaces4));
    obj.insert(kInitialTotal, QString::number(initial_total, 'f', NumericConst::kDecimalPlaces4));
    obj.insert(kTag, Utils::WriteStringList(tag));
    obj.insert(kDocument, Utils::WriteStringList(document));

    // user_id, created_time, created_by, updated_time, updated_by, version
    // are managed by the server, not written to json
    return obj;
}

void NodeI::Reset() { *this = NodeI {}; }

void NodeI::ReadJson(const QJsonObject& object)
{
    Node::ReadJson(object);
    if (const auto val = object.value(kUnitPrice); val.isString())
        unit_price = val.toString().toDouble();
    if (const auto val = object.value(kCommission); val.isString())
        commission = val.toString().toDouble();
}

QJsonObject NodeI::WriteJson() const
{
    QJsonObject obj { Node::WriteJson() };
    obj.insert(kUnitPrice, QString::number(unit_price, 'f', NumericConst::kDecimalPlaces8));
    obj.insert(kCommission, QString::number(commission, 'f', NumericConst::kDecimalPlaces8));
    return obj;
}

void NodeP::Reset() { *this = NodeP {}; }

void NodeP::ReadJson(const QJsonObject& object)
{
    Node::ReadJson(object);
    if (const auto val = object.value(kPaymentTerm); val.isDouble())
        payment_term = val.toInt();
}

QJsonObject NodeP::WriteJson() const
{
    QJsonObject obj {};
    obj.insert(kName, name);
    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kCode, code);
    obj.insert(kDescription, description);
    obj.insert(kKind, std::to_underlying(kind));
    obj.insert(kUnit, std::to_underlying(unit));
    obj.insert(kInitialTotal, QString::number(initial_total, 'f', NumericConst::kDecimalPlaces4));
    obj.insert(kPaymentTerm, payment_term);
    obj.insert(kColor, color);
    obj.insert(kTag, Utils::WriteStringList(tag));
    obj.insert(kDocument, Utils::WriteStringList(document));

    return obj;
}

void NodeO::Reset() { *this = NodeO {}; }

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
    Node::ReadJson(object);
    if (const auto val = object.value(kEmployeeId); val.isString())
        employee_id = QUuid(val.toString());
    if (const auto val = object.value(kPartnerId); val.isString())
        partner_id = QUuid(val.toString());
    if (const auto val = object.value(kIssuedTime); val.isString())
        issued_time = QDateTime::fromString(val.toString(), Qt::ISODate);
    if (const auto val = object.value(kCountTotal); val.isString())
        count_total = val.toString().toDouble();
    if (const auto val = object.value(kMeasureTotal); val.isString())
        measure_total = val.toString().toDouble();
    if (const auto val = object.value(kDiscountTotal); val.isString())
        discount_total = val.toString().toDouble();
    if (const auto val = object.value(kStatus); val.isDouble())
        status = static_cast<NodeStatus>(val.toInt());
    if (const auto val = object.value(kSettlementId); val.isString())
        settlement_id = QUuid(val.toString());
    if (const auto val = object.value(kIsSettled); val.isBool())
        is_settled = val.toBool();
}

QJsonObject NodeO::WriteJson() const
{
    QJsonObject obj {};
    obj.insert(kName, name);
    obj.insert(kId, id.toString(QUuid::WithoutBraces));
    obj.insert(kCode, code);
    obj.insert(kDescription, description);
    obj.insert(kKind, std::to_underlying(kind));
    obj.insert(kDirectionRule, direction_rule);
    obj.insert(kUnit, std::to_underlying(unit));
    obj.insert(kFinalTotal, QString::number(final_total, 'f', NumericConst::kDecimalPlaces4));
    obj.insert(kInitialTotal, QString::number(initial_total, 'f', NumericConst::kDecimalPlaces4));
    obj.insert(kDiscountTotal, QString::number(discount_total, 'f', NumericConst::kDecimalPlaces4));
    obj.insert(kEmployeeId, employee_id.toString(QUuid::WithoutBraces));
    obj.insert(kPartnerId, partner_id.toString(QUuid::WithoutBraces));
    obj.insert(kIssuedTime, issued_time.toString(Qt::ISODate));
    obj.insert(kCountTotal, QString::number(count_total, 'f', NumericConst::kDecimalPlaces8));
    obj.insert(kMeasureTotal, QString::number(measure_total, 'f', NumericConst::kDecimalPlaces8));
    obj.insert(kStatus, std::to_underlying(status));

    return obj;
}
