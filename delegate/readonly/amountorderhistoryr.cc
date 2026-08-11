#include "amountorderhistoryr.h"

#include <QMouseEvent>

#include "enum/nodeenum.h"
#include "tree/node.h"

AmountOrderHistoryR::AmountOrderHistoryR(
    Section section, const int& decimal, const int& unit, CIntString& unit_symbol_map, CString& placeholder, QObject* parent)
    : StyledItemDelegate { parent }
    , decimal_ { decimal }
    , unit_ { unit }
    , section_ { section }
    , unit_symbol_map_ { unit_symbol_map }
    , placeholder_ { placeholder }
{
}

void AmountOrderHistoryR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    PaintText(Format(index), painter, option, index, Qt::AlignRight | Qt::AlignVCenter);
}

QSize AmountOrderHistoryR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString text { Format(index) };
    const QString& str { text.size() > placeholder_.size() ? text : placeholder_ };
    return CalculateTextSize(str, option);
}

QString AmountOrderHistoryR::Format(const QModelIndex& index) const
{
    auto it { unit_symbol_map_.constFind(unit_) };
    auto symbol { (it != unit_symbol_map_.constEnd()) ? it.value() : QString() };

    return symbol + locale_.toString(index.data().toDouble(), 'f', decimal_);
}

bool AmountOrderHistoryR::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    auto* node { static_cast<Node*>(index.internalPointer()) };

    if (node->kind == NodeKind::kLeaf && event->type() == QEvent::MouseButtonDblClick && option.rect.contains(static_cast<QMouseEvent*>(event)->pos()))
        emit SShowOrderHistoryWidget(node->id, node->unit);

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
