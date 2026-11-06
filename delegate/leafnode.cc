#include "leafnode.h"

#include "widget/combobox.h"

LeafNode::LeafNode(const QHash<QUuid, QString>& leaf_path, ItemModel* leaf_model, QObject* parent)
    : StyledItemDelegate { parent }
    , leaf_model_ { leaf_model }
    , leaf_path_ { leaf_path }
{
}

QWidget* LeafNode::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    auto* editor { new ComboBox(parent) };
    editor->setModel(leaf_model_);

    return editor;
}

void LeafNode::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };
    if (cast_editor->hasFocus())
        return;

    auto key { index.data().toUuid() };
  
    const int item_index { cast_editor->findData(key) };
    cast_editor->setCurrentIndex(item_index);
}

void LeafNode::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };

    const auto key { cast_editor->currentData().toUuid() };
    model->setData(index, key);
}

void LeafNode::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QUuid linked_node { index.data().toUuid() };
    if (linked_node.isNull())
        return PaintEmpty(painter, option, index);

    const QString path { leaf_path_.value(linked_node) };
    PaintText(path, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize LeafNode::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString text { leaf_path_.value(index.data().toUuid()) };
    return CalculateTextSize(text, option);
}
