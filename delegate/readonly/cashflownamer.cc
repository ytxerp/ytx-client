#include "cashflownamer.h"

#include "charts/cash_flow_statement/cashflowstatementrow.h"

CashFlowNameR::CashFlowNameR(CTreeModel* model, QObject* parent)
    : StyledItemDelegate { parent }
    , model_ { model }
{
}

void CashFlowNameR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    PaintText(GetPath(index), painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize CashFlowNameR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const { return CalculateTextSize(GetPath(index), option); }

QString CashFlowNameR::GetPath(const QModelIndex& index) const
{
    const QString name { index.data().toString() };

    if (!name.isEmpty()) {
        return name;
    }

    const auto* row { static_cast<const cash_flow::Row*>(index.internalPointer()) };
    return model_->Path(row->id);
}
