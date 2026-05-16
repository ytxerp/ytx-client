#include "doublenonedecimalr.h"

#include "component/constantstring.h"

DoubleNoneDecimalR::DoubleNoneDecimalR(CString& placeholder, QObject* parent)
    : StyledItemDelegate { parent }
    , placeholder_ { placeholder }
{
}

void DoubleNoneDecimalR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const double value { index.data().toDouble() };
    const QString text { value == 0.0 ? string_const::kEmpty : locale_.toString(value) };

    PaintText(text, painter, option, index, Qt::AlignRight | Qt::AlignVCenter);
}

QSize DoubleNoneDecimalR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString text { locale_.toString(index.data().toDouble()) };
    const QString& str { text.size() > placeholder_.size() ? text : placeholder_ };
    return CalculateTextSize(str, option);
}
