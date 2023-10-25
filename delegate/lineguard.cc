#include "lineguard.h"

#include "widget/lineedit.h"

LineGuard::LineGuard(QObject* parent)
    : StyledItemDelegate { parent }
{
}

QWidget* LineGuard::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const { return new LineEdit(parent); }

void LineGuard::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    assert(qobject_cast<LineEdit*>(editor));

    auto* cast_editor { static_cast<LineEdit*>(editor) };
    if (cast_editor->hasFocus())
        return;

    cast_editor->setText(index.data().toString());
}

void LineGuard::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<LineEdit*>(editor) };
    model->setData(index, cast_editor->text().trimmed());
}
