#ifndef INCOMESTATEMENTROW_H
#define INCOMESTATEMENTROW_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUuid>

#include "component/constant.h"
#include "enum/nodeenum.h"

struct IncomeStatementRow final {
    QString name {};
    QUuid id {};
    QString code {};
    QString description {};
    NodeKind kind {};
    bool direction_rule {};

    double final_total {};

    IncomeStatementRow* parent {};
    QList<IncomeStatementRow*> children {};

    inline void Reset() { *this = IncomeStatementRow {}; }
    inline void ReadJson(const QJsonObject& object)
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
        if (const auto val = object.value(kFinalTotal); val.isString())
            final_total = val.toString().toDouble();
    }
};

#endif // INCOMESTATEMENTROW_H
