#include "doublenonezeror.h"

#include "component/constantdouble.h"

DoubleNoneZeroR::DoubleNoneZeroR(const int& decimal, CString& placeholder, QObject* parent)
    : StyledItemDelegate { parent }
    , decimal_ { decimal }
    , placeholder_ { placeholder }
{
}

void DoubleNoneZeroR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const double value { index.data().toDouble() };

    if (std::abs(value) < kTolerance)
        return PaintEmpty(painter, option, index);

    PaintText(locale_.toString(value, 'f', decimal_), painter, option, index, Qt::AlignRight | Qt::AlignVCenter);
}

QSize DoubleNoneZeroR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString text { locale_.toString(index.data().toDouble(), 'f', decimal_) };
    const QString& str { text.size() > placeholder_.size() ? text : placeholder_ };
    return CalculateTextSize(str, option);
}
