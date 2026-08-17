#include "statusr.h"

StatusR::StatusR(int active_status, QObject* parent)
    : StyledItemDelegate { parent }
    , active_status_ { active_status }
{
}

void StatusR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int status { index.data().toInt() };

    if (status != active_status_)
        return PaintEmpty(painter, option, index);

    PaintCheckBox(painter, option, index, active_status_);
}
