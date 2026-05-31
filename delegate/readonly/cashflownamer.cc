#include "cashflownamer.h"

CashFlowNameR::CashFlowNameR(CTreeModel* model, int column, QObject* parent)
    : StyledItemDelegate { parent }
    , model_ { model }
    , column_ { column }
{
}

void CashFlowNameR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    PaintText(GetPath(index), painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize CashFlowNameR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const { return CalculateTextSize(GetPath(index), option); }

QString CashFlowNameR::GetPath(const QModelIndex& index) const
{
    const auto name { index.data().toString() };
    return name.isEmpty() ? model_->Path(index.siblingAtColumn(column_).data().toUuid()) : name;
}
