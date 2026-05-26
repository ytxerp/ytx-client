#include "intstringnonezeror.h"

#include <widget/combobox.h>

IntStringNoneZeroR::IntStringNoneZeroR( CIntString& map, QObject* parent)
    : StyledItemDelegate { parent }
    , map_ { map }
{
}

void IntStringNoneZeroR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int key { index.data().toInt() };
    if (key == 0)
        return PaintEmpty(painter, option, index);

    auto it { map_.constFind(key) };
    if (it == map_.constEnd()) {
        return QStyledItemDelegate::paint(painter, option, index);
    }

    PaintText(it.value(), painter, option, index, Qt::AlignCenter);
}
