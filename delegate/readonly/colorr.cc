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

    QStyle* style { option.widget ? option.widget->style() : QApplication::style() };
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

    const QRect color_rect { option.rect.adjusted(2, 2, -2, -2) };
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(color_string));
    painter->drawRoundedRect(color_rect, 2, 2);
    painter->restore();
}
