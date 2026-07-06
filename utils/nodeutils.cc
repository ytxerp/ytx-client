#include "nodeutils.h"

#include <QApplication>

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
