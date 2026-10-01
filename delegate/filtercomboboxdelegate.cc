#include "filtercomboboxdelegate.h"

#include "widget/combobox.h"

QWidget* FilterComboBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
    auto* combo { new ComboBox(parent) };

    combo->addItem(QString {}, QVariant {});

    for (const auto& [value, text] : items_)
        combo->addItem(text, value);

    return combo;
}

void FilterComboBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* combo { static_cast<ComboBox*>(editor) };
    if (!combo)
        return;

    const int combo_index { qMax(0, combo->findData(index.data(Qt::EditRole))) };

    if (combo->currentIndex() != combo_index)
        combo->setCurrentIndex(combo_index);
}

void FilterComboBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* combo { static_cast<ComboBox*>(editor) };
    if (!combo)
        return;

    model->setData(index, combo->currentData(), Qt::EditRole);
}
