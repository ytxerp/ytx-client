#include "nodepathr.h"

#include <QPainter>

NodePathR::NodePathR(CTreeModel* tree_model, QObject* parent)
    : StyledItemDelegate { parent }
    , tree_model_ { tree_model }
{
}

void NodePathR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString& text { tree_model_->Path(index.data().toUuid()) };
    if (text.isEmpty())
        return PaintEmpty(painter, option, index);

    PaintText(text, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize NodePathR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString& text = tree_model_->Path(index.data().toUuid());
    return CalculateTextSize(text, option);
}
