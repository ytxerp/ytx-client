#ifndef INCOMESTATEMENTROW_H
#define INCOMESTATEMENTROW_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUuid>

#include "component/constant.h"
#include "component/constantstring.h"
#include "enum/nodeenum.h"
#include "utils/nodeutils.h"

namespace income_statement {

struct Row final {
    QString name {};
    QUuid id {};
    QString code {};
    QString description {};
    NodeKind kind {};
    bool direction_rule {};

    double final_total {};
    double yoy_final_total {};
    double yoy_growth_rate {};

    double mom_final_total {};
    double mom_growth_rate {};

    Row* parent {};
    QList<Row*> children {};

    inline void Reset() { *this = Row {}; }
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
        if (const auto val = object.value(income_statement::kYoyFinalTotal); val.isString())
            yoy_final_total = val.toString().toDouble();
        if (const auto val = object.value(income_statement::kMomFinalTotal); val.isString())
            mom_final_total = val.toString().toDouble();

        yoy_growth_rate = utils::GrowthRate(final_total, yoy_final_total);
        mom_growth_rate = utils::GrowthRate(final_total, mom_final_total);
    }
};
}

#endif // INCOMESTATEMENTROW_H
