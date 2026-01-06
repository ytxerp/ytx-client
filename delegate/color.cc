#include "color.h"

#include <QtWidgets/qapplication.h>

#include <QColorDialog>
#include <QKeyEvent>
#include <QPainter>

Color::Color(QObject* parent)
    : StyledItemDelegate { parent }
{
}

void Color::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString color_string { index.data().toString() };

    if (color_string.isEmpty() || !QColor::isValidColorName(color_string))
        return PaintEmpty(painter, option, index);

    QStyle* style = option.widget ? option.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

    const QRect color_rect { option.rect.adjusted(2, 2, -2, -2) };
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(color_string));
    painter->drawRoundedRect(color_rect, 2, 2);
    painter->restore();
}

bool Color::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() != QEvent::MouseButtonDblClick)
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    const auto* mouse_event = static_cast<const QMouseEvent*>(event);
    if (mouse_event->button() != Qt::LeftButton || !option.rect.contains(mouse_event->pos()))
        return false;

    QColor color(index.data().toString());
    if (!color.isValid())
        color = Qt::white;

    QColor selected_color { QColorDialog::getColor(color, nullptr, tr("Choose Color"), QColorDialog::ShowAlphaChannel) };

    if (selected_color.isValid())
        model->setData(index, selected_color.name(QColor::HexArgb));

    return true;
}
