#include "growthrater.h"

GrowthRateR::GrowthRateR(QObject* parent)
    : StyledItemDelegate { parent }
{
}

void GrowthRateR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const double value { index.data().toDouble() * 100 };
    const QString text { qFuzzyIsNull(value) ? QStringLiteral("-") : locale_.toString(value, 'f', 2) + QLatin1Char('%') };
    PaintText(text, painter, option, index, Qt::AlignCenter);
}

QSize GrowthRateR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const double value { index.data().toDouble() * 100 };
    const QString text { locale_.toString(value, 'f', 2) + QLatin1Char('%') };
    return CalculateTextSize(text, option);
}
