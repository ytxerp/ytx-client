#include "searchpathtabler.h"

SearchPathTableR::SearchPathTableR(const QHash<QUuid, QString>& leaf_path, QObject* parent)
    : StyledItemDelegate { parent }
    , leaf_path_ { leaf_path }
{
}

void SearchPathTableR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString path { leaf_path_.value(index.data().toUuid()) };
    PaintText(path, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize SearchPathTableR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString path { leaf_path_.value(index.data().toUuid()) };
    return CalculateTextSize(path, option);
}
