#ifndef PRICES_H
#define PRICES_H

#include <QDateTime>
#include <QUuid>

struct PriceS {
    QDateTime issued_time {};
    QUuid lhs_node {};
    QUuid rhs_node {};
    double unit_price {};
};

#endif // PRICES_H
