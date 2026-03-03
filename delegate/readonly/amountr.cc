#include "amountr.h"

#include "component/constantstring.h"

AmountR::AmountR(const int& decimal, const int& unit, CIntString& unit_symbol_map, CString& placeholder, QObject* parent)
    : StyledItemDelegate { parent }
    , decimal_ { decimal }
    , unit_ { unit }
    , unit_symbol_map_ { unit_symbol_map }
    , placeholder_ { placeholder }
{
}

void AmountR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    PaintText(Format(index), painter, option, index, Qt::AlignRight | Qt::AlignVCenter);
}

QSize AmountR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString text { Format(index) };
    const QString& str { text.size() > placeholder_.size() ? text : placeholder_ };
    return CalculateTextSize(str, option);
}

QString AmountR::Format(const QModelIndex& index) const
{
    auto it { unit_symbol_map_.constFind(unit_) };
    auto symbol { (it != unit_symbol_map_.constEnd()) ? it.value() : StringConst::kEmpty };

    return symbol + locale_.toString(index.data().toDouble(), 'f', decimal_);
}
