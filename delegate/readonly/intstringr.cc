#include "intstringr.h"

#include <widget/combobox.h>

IntStringR::IntStringR(CIntString& map, QObject* parent)
    : StyledItemDelegate { parent }
    , map_ { map }
{
}

void IntStringR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int key { index.data().toInt() };

    auto it { map_.constFind(key) };
    if (it == map_.constEnd()) {
        return QStyledItemDelegate::paint(painter, option, index);
    }

    PaintText(it.value(), painter, option, index, Qt::AlignCenter);
}
