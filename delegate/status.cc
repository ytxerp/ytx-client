#include "status.h"

#include <QMouseEvent>

#include "enum/entryenum.h"

Status::Status(QEvent::Type type, QObject* parent)
    : StyledItemDelegate { parent }
    , type_ { type }
{
}

void Status::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data().toInt() == std::to_underlying(EntryStatus::kUnmarked))
        return PaintEmpty(painter, option, index);

    PaintCheckBox(painter, option, index);
}

bool Status::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() != type_)
        return false;

    auto* mouse_event { static_cast<QMouseEvent*>(event) };
    if (mouse_event->button() != Qt::LeftButton || !option.rect.contains(mouse_event->pos()))
        return false;

    const int status { index.data().toInt() };

    // Note:
    // Here, status = 0 represents a "base" state:
    //   - For Entry tables, it corresponds to EntryStatus::kUnmarked.
    //   - For Node tables, it corresponds to NodeStatus::kEditable.
    // Since this delegate is shared across different models,
    // we cannot bind it directly to a specific enum class,
    // and must toggle the raw int value instead.

    int next_status {};
    switch (status) {
    case 0:
        next_status = 1;
        break;
    case 1:
        next_status = 0;
        break;
    default:
        // Future extension point:
        // e.g. handle "2 = Voided" or other states
        next_status = 0;
        break;
    }

    return model->setData(index, next_status, Qt::EditRole);
}
