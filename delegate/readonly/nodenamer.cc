#include "nodenamer.h"

#include <QPainter>

NodeNameR::NodeNameR(CTreeModel* tree_model, QObject* parent)
    : StyledItemDelegate { parent }
    , tree_model_ { tree_model }
{
}

void NodeNameR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString text { tree_model_->Name(index.data().toUuid()) };
    if (text.isEmpty())
        return PaintEmpty(painter, option, index);

    PaintText(text, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize NodeNameR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString text = tree_model_->Name(index.data().toUuid());
    return CalculateTextSize(text, option);
}
