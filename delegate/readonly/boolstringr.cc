#include "boolstringr.h"

#include <QMouseEvent>

BoolStringR::BoolStringR(CBoolString& map, QObject* parent)
    : StyledItemDelegate { parent }
    , map_ { map }
{
}

void BoolStringR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const bool key { index.data().toBool() };

    auto it { map_.constFind(key) };
    if (it == map_.constEnd()) {
        return PaintEmpty(painter, option, index);
    }

    PaintText(it.value(), painter, option, index, Qt::AlignCenter);
}
