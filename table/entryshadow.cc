#include "entryshadow.h"

#include "component/constant.h"
#include "component/constantint.h"
#include "utils/entryutils.h"

void EntryShadow::Reset() { *this = EntryShadow {}; }

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
    lhs_rate = parallel ? &base->lhs_rate : &base->rhs_rate;
    lhs_debit = parallel ? &base->lhs_debit : &base->rhs_debit;
    lhs_credit = parallel ? &base->lhs_credit : &base->rhs_credit;

    rhs_rate = parallel ? &base->rhs_rate : &base->lhs_rate;
    rhs_debit = parallel ? &base->rhs_debit : &base->lhs_debit;
    rhs_credit = parallel ? &base->rhs_credit : &base->lhs_credit;

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
    obj.insert(kStatus, *status);
    obj.insert(kRhsNode, rhs_node->toString(QUuid::WithoutBraces));
    obj.insert(kLhsRate, QString::number(*lhs_rate, 'f', NumericConst::kDecimalPlaces8));
    obj.insert(kRhsRate, QString::number(*rhs_rate, 'f', NumericConst::kDecimalPlaces8));
    obj.insert(kLhsDebit, QString::number(*lhs_debit, 'f', NumericConst::kDecimalPlaces4));
    obj.insert(kLhsCredit, QString::number(*lhs_credit, 'f', NumericConst::kDecimalPlaces4));
    obj.insert(kRhsDebit, QString::number(*rhs_debit, 'f', NumericConst::kDecimalPlaces4));
    obj.insert(kRhsCredit, QString::number(*rhs_credit, 'f', NumericConst::kDecimalPlaces4));
    obj.insert(kTag, Utils::WriteStringList(*tag));
    obj.insert(kDocument, Utils::WriteStringList(*document));

    return obj;
}
