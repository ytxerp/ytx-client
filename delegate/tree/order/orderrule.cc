#include "orderrule.h"

#include <QMouseEvent>

#include "component/enumclass.h"

OrderRule::OrderRule(CBoolString& map, QEvent::Type type, QObject* parent)
    : StyledItemDelegate { parent }
    , map_ { map }
    , type_ { type }
{
}

void OrderRule::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int kind { index.siblingAtColumn(std::to_underlying(NodeEnumO::kKind)).data().toInt() };
    if (kind == kBranch)
        return PaintEmpty(painter, option, index);

    const bool key { index.data().toBool() };

    auto it { map_.constFind(key) };
    if (it == map_.constEnd()) {
        return PaintEmpty(painter, option, index);
    }

    PaintText(it.value(), painter, option, index, Qt::AlignCenter);
}

bool OrderRule::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() != type_)
        return false;

    const int kind { index.siblingAtColumn(std::to_underlying(NodeEnumO::kKind)).data().toInt() };
    if (kind == kBranch)
        return false;

    auto* mouse_event { static_cast<QMouseEvent*>(event) };
    if (mouse_event->button() != Qt::LeftButton || !option.rect.contains(mouse_event->pos()))
        return false;

    const bool checked { index.data().toBool() };
    return model->setData(index, !checked, Qt::EditRole);
}
