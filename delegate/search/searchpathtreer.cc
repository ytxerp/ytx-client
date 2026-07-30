#include "searchpathtreer.h"

SearchPathTreeR::SearchPathTreeR(CTreeModel* model, QObject* parent)
    : StyledItemDelegate { parent }
    , model_ { model }
{
}

void SearchPathTreeR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    PaintText(GetPath(index), painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize SearchPathTreeR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const { return CalculateTextSize(GetPath(index), option); }

QString SearchPathTreeR::GetPath(const QModelIndex& index) const
{
    auto* node { static_cast<Node*>(index.internalPointer()) };
    return model_->Path(node->id);
}
