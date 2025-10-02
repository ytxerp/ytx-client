#include "doublespinnonezeror.h"

DoubleSpinNoneZeroR::DoubleSpinNoneZeroR(const int& decimal, int coefficient, QObject* parent)
    : StyledItemDelegate { parent }
    , decimal_ { decimal }
    , coefficient_ { coefficient }
{
}

void DoubleSpinNoneZeroR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const double value { index.data().toDouble() };

    if (std::abs(value) < kTolerance)
        return PaintEmpty(painter, option, index);

    PaintText(locale_.toString(value, 'f', decimal_), painter, option, index, Qt::AlignRight | Qt::AlignVCenter);
}

QSize DoubleSpinNoneZeroR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const double value { index.data().toDouble() };
    return CalculateTextSize(locale_.toString(value, 'f', decimal_), option, coefficient_);
}
