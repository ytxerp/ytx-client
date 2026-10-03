#include "filterlineeditdelegate.h"

FilterLineEditDelegate::FilterLineEditDelegate(QObject* parent)
    : StyledItemDelegate(parent)
{
}

QWidget* FilterLineEditDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
    auto* editor { new QLineEdit(parent) };

    editor->setPlaceholderText(tr("Filter"));

    auto* self { const_cast<FilterLineEditDelegate*>(this) };

    connect(editor, &QLineEdit::textChanged, self, [self, editor] { emit self->commitData(editor); });

    return editor;
}

void FilterLineEditDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* edit { qobject_cast<QLineEdit*>(editor) };

    if (!edit)
        return;

    const QString text { index.data(Qt::EditRole).toString() };

    if (edit->text() != text)
        edit->setText(text);
}

void FilterLineEditDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* edit { qobject_cast<QLineEdit*>(editor) };

    if (!edit)
        return;

    model->setData(index, edit->text(), Qt::EditRole);
}

void FilterLineEditDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const
{
    editor->setGeometry(option.rect);
}
