#include "nodeutils.h"

#include <QtCore/qtimezone.h>

bool node::IsDescendant(const Node* descendant, const Node* ancestor)
{
    Q_ASSERT(descendant != nullptr);
    Q_ASSERT(ancestor != nullptr);

    if (descendant == ancestor)
        return false;

    while (descendant && descendant != ancestor)
        descendant = descendant->parent;

    return descendant == ancestor;
}

std::pair<QDateTime, QDateTime> utils::DefaultLocalRange()
{
    const QDate today { QDate::currentDate() };

    const QDateTime start { today, kStartTime, QTimeZone::LocalTime };
    const QDateTime end { today.addDays(1), kStartTime, QTimeZone::LocalTime };

    return { start, end };
}
