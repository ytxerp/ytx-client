#include "cashflownamer.h"

#include "dashboard/cash_flow_statement/cashflowstatementrow.h"

CashFlowNameR::CashFlowNameR(const QHash<QUuid, QString>& leaf_path, QObject* parent)
    : StyledItemDelegate { parent }
    , leaf_path_ { leaf_path }
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
    return leaf_path_.value(row->id);
}
