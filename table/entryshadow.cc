#include "entryshadow.h"

#include "component/constant.h"
#include "component/constantint.h"
#include "utils/entryutils.h"

void EntryShadow::Reset() { *this = EntryShadow {}; }

void EntryShadow::BindEntry(Entry* base, BindingMode mode)
{
    entry = base;

    id = &base->id;
    issued_time = &base->issued_time;
    code = &base->code;
    description = &base->description;
    document = &base->document;
    tag = &base->tag;
    status = &base->status;
    sync_state = &base->sync_state;
    version = &base->version;

    binding_mode = mode;

    switch (mode) {
    case BindingMode::kParallel:
        lhs_node = &base->lhs_node;
        rhs_node = &base->rhs_node;

        lhs_rate = &base->lhs_rate;
        lhs_debit = &base->lhs_debit;
        lhs_credit = &base->lhs_credit;

        rhs_rate = &base->rhs_rate;
        rhs_debit = &base->rhs_debit;
        rhs_credit = &base->rhs_credit;
        break;

    case BindingMode::kCross:
        lhs_node = &base->rhs_node;
        rhs_node = &base->lhs_node;

        lhs_rate = &base->rhs_rate;
        lhs_debit = &base->rhs_debit;
        lhs_credit = &base->rhs_credit;

        rhs_rate = &base->lhs_rate;
        rhs_debit = &base->lhs_debit;
        rhs_credit = &base->lhs_credit;
        break;
    }
}

QJsonObject EntryShadow::WriteJson() const
{
    QJsonObject obj {};
    obj.insert(kId, id->toString(QUuid::WithoutBraces));
    obj.insert(kIssuedTime, issued_time->toUTC().toString(Qt::ISODate));
    obj.insert(kCode, *code);
    obj.insert(kLhsNode, lhs_node->toString(QUuid::WithoutBraces));
    obj.insert(kDescription, *description);
    obj.insert(kStatus, *status);
    obj.insert(kRhsNode, rhs_node->toString(QUuid::WithoutBraces));
    obj.insert(kLhsRate, QString::number(*lhs_rate, 'f', numeric_const::kDecimalPlaces8));
    obj.insert(kRhsRate, QString::number(*rhs_rate, 'f', numeric_const::kDecimalPlaces8));
    obj.insert(kLhsDebit, QString::number(*lhs_debit, 'f', numeric_const::kDecimalPlaces8));
    obj.insert(kLhsCredit, QString::number(*lhs_credit, 'f', numeric_const::kDecimalPlaces8));
    obj.insert(kRhsDebit, QString::number(*rhs_debit, 'f', numeric_const::kDecimalPlaces8));
    obj.insert(kRhsCredit, QString::number(*rhs_credit, 'f', numeric_const::kDecimalPlaces8));
    obj.insert(kTag, utils::WriteStringList(*tag));
    obj.insert(kDocument, utils::WriteStringList(*document));
    obj.insert(kVersion, *version);

    return obj;
}

void EntryShadowF::BindEntry(Entry* base, BindingMode mode)
{
    EntryShadow::BindEntry(base, mode);
    cash_kind = &(static_cast<EntryF*>(base)->cash_kind);
}

void EntryShadowF::Reset() { *this = EntryShadowF {}; }

QJsonObject EntryShadowF::WriteJson() const
{
    QJsonObject obj { EntryShadow::WriteJson() };
    obj.insert(kCashKind, std::to_underlying(*cash_kind));

    return obj;
}
