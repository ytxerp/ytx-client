#include "statusdelegate.h"

#include <QMouseEvent>

StatusDelegate::StatusDelegate(QEvent::Type type, int inactive_status, int active_status, QObject* parent)
    : StyledItemDelegate { parent }
    , type_ { type }
    , inactive_status_ { inactive_status }
    , active_status_ { active_status }
{
}

void StatusDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int status { index.data().toInt() };

    if (status != active_status_)
        return PaintEmpty(painter, option, index);

    PaintCheckBox(painter, option, index, active_status_);
}

bool StatusDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() != type_)
        return false;

    auto* mouse_event { static_cast<QMouseEvent*>(event) };
    if (mouse_event->button() != Qt::LeftButton || !option.rect.contains(mouse_event->pos()))
        return false;

    const int status { index.data().toInt() };

    // Status values are provided externally to keep this delegate independent
    // of any specific enum class. Unknown states are reset to inactive.

    const int next_status { status == inactive_status_ ? active_status_ : inactive_status_ };
    return model->setData(index, next_status, Qt::EditRole);
}
