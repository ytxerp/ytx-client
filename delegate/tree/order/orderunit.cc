#include "orderunit.h"

#include "widget/combobox.h"

OrderUnit::OrderUnit(CIntString& map, ItemModel* model, QObject* parent)
    : StyledItemDelegate { parent }
    , map_ { map }
    , model_ { model }
{
}

QWidget* OrderUnit::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto* editor { new ComboBox(parent) };
    editor->setModel(model_);
    return editor;
}

void OrderUnit::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };

    int item_index { cast_editor->findData(index.data().toInt()) };
    if (item_index != -1)
        cast_editor->setCurrentIndex(item_index);
}

void OrderUnit::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };
    model->setData(index, cast_editor->currentData().toInt());
}

void OrderUnit::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    PaintText(MapValue(index.data().toInt()), painter, option, index, Qt::AlignCenter);
}

QString OrderUnit::MapValue(int key) const
{
    auto it { map_.constFind(key) };
    return (it != map_.constEnd()) ? it.value() : kEmptyString;
}
