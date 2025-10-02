#include "boolstring.h"

#include <QMouseEvent>

BoolString::BoolString(CBoolString& map, QEvent::Type type, QObject* parent)
    : StyledItemDelegate { parent }
    , map_ { map }
    , type_ { type }
{
}

void BoolString::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const bool key { index.data().toBool() };

    auto it { map_.constFind(key) };
    if (it == map_.constEnd()) {
        return;
    }

    PaintText(it.value(), painter, option, index, Qt::AlignCenter);
}

bool BoolString::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() != type_)
        return false;

    auto* mouse_event { static_cast<QMouseEvent*>(event) };
    if (mouse_event->button() != Qt::LeftButton || !option.rect.contains(mouse_event->pos()))
        return false;

    const bool checked { index.data().toBool() };
    return model->setData(index, !checked, Qt::EditRole);
}
