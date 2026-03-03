#include "financeforeignr.h"

#include "component/constantstring.h"
#include "enum/nodeenum.h"

FinanceForeignR::FinanceForeignR(const int& decimal, const int& default_unit, CIntString& unit_symbol_map, CString& placeholder, QObject* parent)
    : StyledItemDelegate { parent }
    , decimal_ { decimal }
    , default_unit_ { default_unit }
    , unit_symbol_map_ { unit_symbol_map }
    , placeholder_ { placeholder }
{
}

void FinanceForeignR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int unit { index.siblingAtColumn(std::to_underlying(NodeEnumF::kUnit)).data().toInt() };
    if (unit == default_unit_)
        return PaintEmpty(painter, option, index);

    PaintText(Format(index, unit), painter, option, index, Qt::AlignRight | Qt::AlignVCenter);
}

QSize FinanceForeignR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int unit { index.siblingAtColumn(std::to_underlying(NodeEnumF::kUnit)).data().toInt() };
    const QString text { Format(index, unit) };
    const QString& str { text.size() > placeholder_.size() ? text : placeholder_ };
    return CalculateTextSize(str, option);
}

QString FinanceForeignR::Format(const QModelIndex& index, int unit) const
{
    auto it { unit_symbol_map_.constFind(unit) };
    if (it == unit_symbol_map_.constEnd())
        return StringConst::kEmpty;

    return it.value() + locale_.toString(index.data().toDouble(), 'f', decimal_);
}
