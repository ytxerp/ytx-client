#include "doubler.h"

DoubleR::DoubleR(const int& decimal, CString& placeholder, QObject* parent)
    : StyledItemDelegate { parent }
    , decimal_ { decimal }
    , placeholder_ { placeholder }
{
}

void DoubleR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const double value { index.data().toDouble() };
    PaintText(locale_.toString(value, 'f', decimal_), painter, option, index, Qt::AlignRight | Qt::AlignVCenter);
}

QSize DoubleR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString text { locale_.toString(index.data().toDouble(), 'f', decimal_) };
    const QString& str { text.size() > placeholder_.size() ? text : placeholder_ };
    return CalculateTextSize(str, option);
}
