#include "financerolesdelegater.h"

#include "tree/finance_role.h"

FinanceRolesDelegateR::FinanceRolesDelegateR(QObject* parent)
    : StyledItemDelegate { parent }
{
}

void FinanceRolesDelegateR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int value { index.data().toInt() };
    if (value == 0)
        return PaintEmpty(painter, option, index);

    const QString text { finance::RolesDisplay(finance::Role(value)) };
    PaintText(text, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize FinanceRolesDelegateR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int value { index.data().toInt() };
    const QString text { finance::RolesDisplay(finance::Role(value)) };

    return CalculateTextSize(text, option);
}
