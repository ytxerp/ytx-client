#include "bool.h"

#include <QMouseEvent>
#include <QUuid>

Bool::Bool(QEvent::Type type, QObject* parent)
    : StyledItemDelegate { parent }
    , type_ { type }
{
}

void Bool::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.data().toBool())
        return PaintEmpty(painter, option, index);

    PaintCheckBox(painter, option, index);
}

bool Bool::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() != type_)
        return false;

    auto* mouse_event { static_cast<QMouseEvent*>(event) };
    if (mouse_event->button() != Qt::LeftButton || !option.rect.contains(mouse_event->pos()))
        return false;

    const int status { index.data().toInt() };

    int next_status {};
    switch (status) {
    case 0:
        next_status = 1;
        break;
    case 1:
        next_status = 0;
        break;
    default:
        next_status = 0;
        break;
    }

    return model->setData(index, next_status, Qt::EditRole);
}
