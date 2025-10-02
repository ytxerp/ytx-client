#include "statusr.h"

StatusR::StatusR(QObject* parent)
    : StyledItemDelegate { parent }
{
}

void StatusR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.data().toBool())
        return PaintEmpty(painter, option, index);

    PaintCheckBox(painter, option, index);
}
