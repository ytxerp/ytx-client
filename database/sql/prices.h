#ifndef PRICES_H
#define PRICES_H

#include <QString>
#include <QUuid>

struct PriceS {
    QString issued_time {};
    QUuid lhs_node {};
    QUuid inside_product {};
    double unit_price {};
};

#endif // PRICES_H
