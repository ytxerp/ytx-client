#include "documentr.h"

DocumentR::DocumentR( QObject* parent)
    : StyledItemDelegate { parent }
{
}

void DocumentR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QStringList list { index.data().toStringList() };
    const auto size { list.size() };

    if (size == 0) {
        return PaintEmpty(painter, option, index);
    }

    const QString text { QString::number(size) };
    PaintText(text, painter, option, index, Qt::AlignCenter);
}

QSize DocumentR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QStringList list { index.data().toStringList() };
    const auto size { list.size() };

    const QString text { size == 0 ? QString() : QString::number(size) };
    return CalculateTextSize(text, option);
}
