#include "dboolstringr.h"

#include <QMouseEvent>

DBoolStringR::DBoolStringR(CBoolString& map, QObject* parent)
    : StyledItemDelegate { parent }
    , map_ { map }
{
}

void DBoolStringR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const bool key { index.data().toBool() };

    auto it { map_.constFind(key) };
    if (it == map_.constEnd()) {
        return QStyledItemDelegate::paint(painter, option, index);
    }

    PaintText(it.value(), painter, option, index, Qt::AlignCenter);
}
