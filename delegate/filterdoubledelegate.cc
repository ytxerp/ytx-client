#include "filterdoubledelegate.h"

#include <QDoubleValidator>
#include <QLineEdit>
#include <QLocale>

FilterDoubleDelegate::FilterDoubleDelegate(QObject* parent)
    : StyledItemDelegate(parent)
{
}

QWidget* FilterDoubleDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
    auto* editor { new QLineEdit(parent) };

    editor->setPlaceholderText(tr("Filter"));
    editor->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto* validator { new QDoubleValidator(editor) };
    validator->setNotation(QDoubleValidator::StandardNotation);
    validator->setLocale(QLocale::c()); // '.' as decimal point, consistent with QString::toDouble
    editor->setValidator(validator);

    auto* self { const_cast<FilterDoubleDelegate*>(this) };
    connect(editor, &QLineEdit::textChanged, self, [self, editor] { emit self->commitData(editor); });

    return editor;
}

void FilterDoubleDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* line_edit { static_cast<QLineEdit*>(editor) };
    if (line_edit->hasFocus())
        return; // do not overwrite what the user is typing

    const auto value { index.data(Qt::EditRole) };
    line_edit->setText(value.isValid() ? QString::number(value.toDouble()) : QString {});
}

void FilterDoubleDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    const auto text { static_cast<QLineEdit*>(editor)->text().trimmed() };

    // Empty input clears the filter (invalid QVariant)
    model->setData(index, text.isEmpty() ? QVariant {} : QVariant { text }, Qt::EditRole);
}

void FilterDoubleDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const
{
    editor->setGeometry(option.rect);
}

void FilterDoubleDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const auto text { index.data(Qt::DisplayRole).toString() };
    PaintText(text, painter, option, index, Qt::AlignRight | Qt::AlignVCenter);
}