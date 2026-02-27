#include "financeforeignr.h"

#include "enum/nodeenum.h"

FinanceForeignR::FinanceForeignR(const int& decimal, const int& default_unit, CIntString& unit_symbol_map, QObject* parent)
    : StyledItemDelegate { parent }
    , decimal_ { decimal }
    , default_unit_ { default_unit }
    , unit_symbol_map_ { unit_symbol_map }
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
    if (unit == default_unit_)
        return option.rect.size();

    return CalculateTextSize(Format(index, unit), option, kCoefficient16);
}

QString FinanceForeignR::Format(const QModelIndex& index, int unit) const
{
    auto it { unit_symbol_map_.constFind(unit) };
    if (it == unit_symbol_map_.constEnd())
        return kEmptyString;

    return it.value() + locale_.toString(index.data().toDouble(), 'f', decimal_);
}
