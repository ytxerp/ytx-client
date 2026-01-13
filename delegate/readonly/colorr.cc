#include "colorr.h"

#include <QtWidgets/qapplication.h>

#include <QPainter>

ColorR::ColorR(QObject* parent)
    : StyledItemDelegate { parent }
{
}

void ColorR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString color_string { index.data().toString() };

    if (color_string.isEmpty() || !QColor::isValidColorName(color_string))
        return PaintEmpty(painter, option, index);

    PaintColorRect(painter, option, color_string);
}
