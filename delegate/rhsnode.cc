#include "rhsnode.h"

#include "widget/combobox.h"

RhsNode::RhsNode(CTreeModel* tree_model, QSortFilterProxyModel* filter_model, QObject* parent)
    : StyledItemDelegate { parent }
    , tree_model_ { tree_model }
    , filter_model_ { filter_model }
{
}

QWidget* RhsNode::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    auto* editor { new ComboBox(parent) };
    editor->setModel(filter_model_);

    return editor;
}

void RhsNode::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };
    if (cast_editor->hasFocus())
        return;

    auto key { index.data().toUuid() };
    if (key.isNull())
        key = last_insert_;

    const int item_index { cast_editor->findData(key) };
    cast_editor->setCurrentIndex(item_index);
}

void RhsNode::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };

    const auto key { cast_editor->currentData().toUuid() };
    last_insert_ = key;
    model->setData(index, key);
}

void RhsNode::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QUuid linked_node { index.data().toUuid() };
    if (linked_node.isNull())
        return PaintEmpty(painter, option, index);

    const QString path { tree_model_->Path(linked_node) };
    PaintText(path, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);

    // 高度自定义
    // const int text_margin { CalculateTextMargin(style, option.widget) };
    // const QRect text_rect { opt.rect.adjusted(text_margin, 0, -text_margin, 0) };
    // painter->drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, path);
}

QSize RhsNode::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString text { tree_model_->Path(index.data().toUuid()) };
    return CalculateTextSize(text, option);
}
