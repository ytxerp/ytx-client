#include "entryshadow.h"

#include "component/constant.h"

void EntryShadow::ResetState()
{
    id = nullptr;
    issued_time = nullptr;
    code = nullptr;
    lhs_node = nullptr;
    description = nullptr;
    rhs_node = nullptr;
    is_checked = nullptr;
    document = nullptr;

    user_id = nullptr;
    created_time = nullptr;
    created_by = nullptr;
    updated_time = nullptr;
    updated_by = nullptr;
    entry = nullptr;

    balance = 0.0;
    is_parallel = {};
}

void EntryShadow::BindEntry(Entry* base, bool parallel)
{
    entry = base;

    id = &base->id;
    issued_time = &base->issued_time;
    code = &base->code;
    description = &base->description;
    document = &base->document;
    is_checked = &base->is_checked;

    is_parallel = parallel;

    lhs_node = parallel ? &base->lhs_node : &base->rhs_node;
    rhs_node = parallel ? &base->rhs_node : &base->lhs_node;

    user_id = &base->user_id;
    created_time = &base->created_time;
    created_by = &base->created_by;
    updated_time = &base->updated_time;
    updated_by = &base->updated_by;
}

QJsonObject EntryShadow::WriteJson() const
{
    Q_ASSERT(id != nullptr);

    QJsonObject obj {};
    obj.insert(kId, id->toString(QUuid::WithoutBraces));
    obj.insert(kIssuedTime, issued_time->toString(Qt::ISODate));
    obj.insert(kCode, *code);
    obj.insert(kLhsNode, lhs_node->toString(QUuid::WithoutBraces));
    obj.insert(kDescription, *description);
    obj.insert(kDocument, document->join(kSemicolon));
    obj.insert(kIsChecked, *is_checked);
    obj.insert(kRhsNode, rhs_node->toString(QUuid::WithoutBraces));

    return obj;
}

void EntryShadowF::BindEntry(Entry* base, bool is_parallel)
{
    EntryShadow::BindEntry(base, is_parallel);

    auto* entry = static_cast<EntryF*>(base);

    lhs_rate = is_parallel ? &entry->lhs_rate : &entry->rhs_rate;
    lhs_debit = is_parallel ? &entry->lhs_debit : &entry->rhs_debit;
    lhs_credit = is_parallel ? &entry->lhs_credit : &entry->rhs_credit;

    rhs_rate = is_parallel ? &entry->rhs_rate : &entry->lhs_rate;
    rhs_debit = is_parallel ? &entry->rhs_debit : &entry->lhs_debit;
    rhs_credit = is_parallel ? &entry->rhs_credit : &entry->lhs_credit;
}

void EntryShadowF::ResetState()
{
    EntryShadow::ResetState();

    lhs_rate = nullptr;
    rhs_rate = nullptr;
    lhs_debit = nullptr;
    lhs_credit = nullptr;
    rhs_debit = nullptr;
    rhs_credit = nullptr;
}

QJsonObject EntryShadowF::WriteJson() const
{
    QJsonObject obj = EntryShadow::WriteJson();

    obj.insert(kLhsRate, QString::number(*lhs_rate, 'f', kMaxNumericScale_8));
    obj.insert(kRhsRate, QString::number(*rhs_rate, 'f', kMaxNumericScale_8));
    obj.insert(kLhsDebit, QString::number(*lhs_debit, 'f', kMaxNumericScale_4));
    obj.insert(kLhsCredit, QString::number(*lhs_credit, 'f', kMaxNumericScale_4));
    obj.insert(kRhsDebit, QString::number(*rhs_debit, 'f', kMaxNumericScale_4));
    obj.insert(kRhsCredit, QString::number(*rhs_credit, 'f', kMaxNumericScale_4));

    return obj;
}

void EntryShadowI::BindEntry(Entry* base, bool is_parallel)
{
    EntryShadow::BindEntry(base, is_parallel);

    auto* entry = static_cast<EntryI*>(base);
    unit_cost = &entry->unit_cost;

    lhs_debit = is_parallel ? &entry->lhs_debit : &entry->rhs_debit;
    lhs_credit = is_parallel ? &entry->lhs_credit : &entry->rhs_credit;
    rhs_debit = is_parallel ? &entry->rhs_debit : &entry->lhs_debit;
    rhs_credit = is_parallel ? &entry->rhs_credit : &entry->lhs_credit;
}

void EntryShadowI::ResetState()
{
    EntryShadow::ResetState();

    unit_cost = nullptr;
    lhs_debit = nullptr;
    lhs_credit = nullptr;
    rhs_debit = nullptr;
    rhs_credit = nullptr;
}

QJsonObject EntryShadowI::WriteJson() const
{
    QJsonObject obj = EntryShadow::WriteJson();

    obj.insert(kUnitCost, QString::number(*unit_cost, 'f', kMaxNumericScale_4));
    obj.insert(kLhsDebit, QString::number(*lhs_debit, 'f', kMaxNumericScale_4));
    obj.insert(kLhsCredit, QString::number(*lhs_credit, 'f', kMaxNumericScale_4));
    obj.insert(kRhsDebit, QString::number(*rhs_debit, 'f', kMaxNumericScale_4));
    obj.insert(kRhsCredit, QString::number(*rhs_credit, 'f', kMaxNumericScale_4));

    return obj;
}

void EntryShadowT::BindEntry(Entry* base, bool is_parallel)
{
    EntryShadow::BindEntry(base, is_parallel);

    auto* entry = static_cast<EntryT*>(base);
    unit_cost = &entry->unit_cost;

    lhs_debit = is_parallel ? &entry->lhs_debit : &entry->rhs_debit;
    lhs_credit = is_parallel ? &entry->lhs_credit : &entry->rhs_credit;
    rhs_debit = is_parallel ? &entry->rhs_debit : &entry->lhs_debit;
    rhs_credit = is_parallel ? &entry->rhs_credit : &entry->lhs_credit;
}

void EntryShadowT::ResetState()
{
    EntryShadow::ResetState();

    unit_cost = nullptr;
    lhs_debit = nullptr;
    lhs_credit = nullptr;
    rhs_debit = nullptr;
    rhs_credit = nullptr;
}

QJsonObject EntryShadowT::WriteJson() const
{
    QJsonObject obj = EntryShadow::WriteJson();

    obj.insert(kUnitCost, QString::number(*unit_cost, 'f', kMaxNumericScale_4));
    obj.insert(kLhsDebit, QString::number(*lhs_debit, 'f', kMaxNumericScale_4));
    obj.insert(kLhsCredit, QString::number(*lhs_credit, 'f', kMaxNumericScale_4));
    obj.insert(kRhsDebit, QString::number(*rhs_debit, 'f', kMaxNumericScale_4));
    obj.insert(kRhsCredit, QString::number(*rhs_credit, 'f', kMaxNumericScale_4));

    return obj;
}

void EntryShadowS::BindEntry(Entry* base, bool /* is_parallel */)
{
    EntryShadow::BindEntry(base, true); // Always parallel for EntryS

    auto* entry = static_cast<EntryS*>(base);
    unit_price = &entry->unit_price;
    external_item = &entry->external_item;
}

void EntryShadowS::ResetState()
{
    EntryShadow::ResetState();

    unit_price = nullptr;
    external_item = nullptr;
}

QJsonObject EntryShadowS::WriteJson() const
{
    QJsonObject obj = EntryShadow::WriteJson();
    obj.insert(kUnitPrice, QString::number(*unit_price, 'f', kMaxNumericScale_4));
    obj.insert(kExternalItem, external_item->toString(QUuid::WithoutBraces));
    return obj;
}

// Note:
// Order entries do not use some common fields from EntryShadow
// (issued_time, document, is_checked, code). Therefore, we do NOT call
// the base class BindEntry() here, and only bind the fields that
// are relevant for order entries.
void EntryShadowO::BindEntry(Entry* base, bool /* is_parallel */)
{
    entry = base;

    id = &base->id;
    description = &base->description;

    is_parallel = true;

    lhs_node = &base->lhs_node;
    rhs_node = &base->rhs_node;

    user_id = &base->user_id;
    created_time = &base->created_time;
    created_by = &base->created_by;
    updated_time = &base->updated_time;
    updated_by = &base->updated_by;

    auto* entry = static_cast<EntryO*>(base);
    unit_price = &entry->unit_price;
    external_item = &entry->external_item;

    first = &entry->first;
    second = &entry->second;

    initial = &entry->initial;
    final = &entry->final;
    discount = &entry->discount;
    discount_price = &entry->discount_price;
}

void EntryShadowO::ResetState()
{
    EntryShadow::ResetState();

    unit_price = nullptr;
    external_item = nullptr;
    first = nullptr;
    second = nullptr;
    initial = nullptr;
    final = nullptr;
    discount = nullptr;
    discount_price = nullptr;
}

QJsonObject EntryShadowO::WriteJson() const
{
    QJsonObject obj {};
    obj.insert(kId, id->toString(QUuid::WithoutBraces));
    obj.insert(kDescription, *description);
    obj.insert(kLhsNode, lhs_node->toString(QUuid::WithoutBraces));
    obj.insert(kRhsNode, rhs_node->toString(QUuid::WithoutBraces));

    obj.insert(kUnitPrice, QString::number(*unit_price, 'f', kMaxNumericScale_4));
    obj.insert(kFirst, QString::number(*first, 'f', kMaxNumericScale_4));
    obj.insert(kSecond, QString::number(*second, 'f', kMaxNumericScale_4));
    obj.insert(kInitial, QString::number(*initial, 'f', kMaxNumericScale_4));
    obj.insert(kFinal, QString::number(*final, 'f', kMaxNumericScale_4));
    obj.insert(kDiscount, QString::number(*discount, 'f', kMaxNumericScale_4));
    obj.insert(kDiscountPrice, QString::number(*discount_price, 'f', kMaxNumericScale_4));
    obj.insert(kExternalItem, external_item->toString(QUuid::WithoutBraces));

    return obj;
}

QJsonObject State::WriteJson() const
{
    QJsonObject json {};
    json.insert(kEntry, entry);

    if (!lhs_node.isEmpty())
        json.insert(kLhsNode, lhs_node);

    if (!rhs_node.isEmpty())
        json.insert(kRhsNode, rhs_node);

    return json;
}
