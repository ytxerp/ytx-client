#include "boolcolorstringr.h"

BoolColorStringR::BoolColorStringR(CBoolString& map, const QString& true_color, const QString& false_color, QObject* parent)
    : StyledItemDelegate { parent }
    , map_ { map }
    , true_color_ { true_color }
    , false_color_ { false_color }
{
}

void BoolColorStringR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const bool key { index.data().toBool() };

    auto it { map_.constFind(key) };
    if (it == map_.constEnd()) {
        return;
    }

    const QString& color { key ? true_color_ : false_color_ };
    PaintColorText(it.value(), color, painter, option, index, Qt::AlignCenter);
}
