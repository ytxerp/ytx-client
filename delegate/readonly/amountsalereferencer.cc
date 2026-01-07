#include "amountsalereferencer.h"

#include <QMouseEvent>

#include "enum/nodeenum.h"
#include "utils/nodeutils.h"

AmountSaleReferenceR::AmountSaleReferenceR(Section section, const int& decimal, const int& unit, CIntString& unit_symbol_map, QObject* parent)
    : StyledItemDelegate { parent }
    , decimal_ { decimal }
    , unit_ { unit }
    , section_ { section }
    , unit_symbol_map_ { unit_symbol_map }
{
}

void AmountSaleReferenceR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    PaintText(Format(index), painter, option, index, Qt::AlignRight | Qt::AlignVCenter);
}

QSize AmountSaleReferenceR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return { CalculateTextSize(Format(index), option, kCoefficient16) };
}

QString AmountSaleReferenceR::Format(const QModelIndex& index) const
{
    auto it { unit_symbol_map_.constFind(unit_) };
    auto symbol { (it != unit_symbol_map_.constEnd()) ? it.value() : kEmptyString };

    return symbol + locale_.toString(index.data().toDouble(), 'f', decimal_);
}

bool AmountSaleReferenceR::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    const int kind_column { Utils::KindColumn(section_) };
    const int unit_column { Utils::UnitColumn(section_) };

    const bool is_leaf { index.siblingAtColumn(kind_column).data().toInt() == std::to_underlying(NodeKind::kLeaf) };

    const QUuid node_id { index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() };
    const int unit { index.siblingAtColumn(unit_column).data().toInt() };

    if (is_leaf && event->type() == QEvent::MouseButtonDblClick && option.rect.contains(static_cast<QMouseEvent*>(event)->pos()))
        emit SSaleReferencePrimary(node_id, unit);

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
