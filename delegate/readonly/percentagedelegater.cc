#include "percentagedelegater.h"

PercentageDelegateR::PercentageDelegateR(QObject* parent)
    : StyledItemDelegate { parent }
{
}

void PercentageDelegateR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const double value { index.data().toDouble() };
    PaintText(FormatPercentage(value), painter, option, index, Qt::AlignCenter);
}

QSize PercentageDelegateR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const double value { index.data().toDouble() };
    return CalculateTextSize(FormatPercentage(value), option);
}
