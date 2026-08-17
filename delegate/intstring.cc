#include "intstring.h"

#include <widget/combobox.h>

IntString::IntString(UnitModel* model, CIntString& map, QObject* parent)
    : StyledItemDelegate { parent }
    , model_ { model }
    , map_ { map }
{
}

QWidget* IntString::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    auto* editor { new ComboBox(parent) };

    editor->setModel(model_);
    return editor;
}

void IntString::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };

    const int value { index.data(Qt::EditRole).toInt() };
    const int idx { cast_editor->findData(value) };

    if (idx != -1)
        cast_editor->setCurrentIndex(idx);
}

void IntString::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };
    model->setData(index, cast_editor->currentData().toInt(), Qt::EditRole);
}

void IntString::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int key { index.data().toInt() };

    auto it { map_.constFind(key) };
    if (it == map_.constEnd()) {
        return QStyledItemDelegate::paint(painter, option, index);
    }

    PaintText(it.value(), painter, option, index, Qt::AlignCenter);
}

QSize IntString::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int key { index.data().toInt() };
    return CalculateTextSize(map_.value(key), option);
}
