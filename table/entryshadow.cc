#include "entryshadow.h"

#include "component/constant.h"
#include "utils/entryutils.h"

void EntryShadow::ResetState()
{
    id = nullptr;
    issued_time = nullptr;
    code = nullptr;
    lhs_node = nullptr;
    description = nullptr;
    rhs_node = nullptr;
    status = nullptr;
    document = nullptr;
    tag = nullptr;
    lhs_rate = nullptr;
    rhs_rate = nullptr;
    lhs_debit = nullptr;
    lhs_credit = nullptr;
    rhs_debit = nullptr;
    rhs_credit = nullptr;

    user_id = nullptr;
    created_time = nullptr;
    created_by = nullptr;
    updated_time = nullptr;
    updated_by = nullptr;
    entry = nullptr;
    version = nullptr;

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
    tag = &base->tag;
    status = &base->status;

    is_parallel = parallel;

    lhs_node = parallel ? &base->lhs_node : &base->rhs_node;
    rhs_node = parallel ? &base->rhs_node : &base->lhs_node;
    lhs_rate = is_parallel ? &entry->lhs_rate : &entry->rhs_rate;
    lhs_debit = is_parallel ? &entry->lhs_debit : &entry->rhs_debit;
    lhs_credit = is_parallel ? &entry->lhs_credit : &entry->rhs_credit;

    rhs_rate = is_parallel ? &entry->rhs_rate : &entry->lhs_rate;
    rhs_debit = is_parallel ? &entry->rhs_debit : &entry->lhs_debit;
    rhs_credit = is_parallel ? &entry->rhs_credit : &entry->lhs_credit;

    user_id = &base->user_id;
    created_time = &base->created_time;
    created_by = &base->created_by;
    updated_time = &base->updated_time;
    updated_by = &base->updated_by;
    version = &base->version;
}

QJsonObject EntryShadow::WriteJson() const
{
    QJsonObject obj {};
    obj.insert(kId, id->toString(QUuid::WithoutBraces));
    obj.insert(kIssuedTime, issued_time->toString(Qt::ISODate));
    obj.insert(kCode, *code);
    obj.insert(kLhsNode, lhs_node->toString(QUuid::WithoutBraces));
    obj.insert(kDescription, *description);
    obj.insert(kDocument, document->join(kSemicolon));
    obj.insert(kStatus, *status);
    obj.insert(kRhsNode, rhs_node->toString(QUuid::WithoutBraces));
    obj.insert(kLhsRate, QString::number(*lhs_rate, 'f', kMaxNumericScale_8));
    obj.insert(kRhsRate, QString::number(*rhs_rate, 'f', kMaxNumericScale_8));
    obj.insert(kLhsDebit, QString::number(*lhs_debit, 'f', kMaxNumericScale_4));
    obj.insert(kLhsCredit, QString::number(*lhs_credit, 'f', kMaxNumericScale_4));
    obj.insert(kRhsDebit, QString::number(*rhs_debit, 'f', kMaxNumericScale_4));
    obj.insert(kRhsCredit, QString::number(*rhs_credit, 'f', kMaxNumericScale_4));
    obj.insert(kTag, Utils::WriteUuidArray(*tag));

    return obj;
}
