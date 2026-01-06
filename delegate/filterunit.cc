#include "filterunit.h"

#include <QPainter>

#include "widget/combobox.h"

FilterUnit::FilterUnit(CTreeModel* tree_model, QSortFilterProxyModel* filter_model, QObject* parent)
    : StyledItemDelegate { parent }
    , tree_model_ { tree_model }
    , filter_model_ { filter_model }
{
}

QWidget* FilterUnit::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    Q_UNUSED(option);

    auto* editor { new ComboBox(parent) };
    editor->setModel(filter_model_);

    return editor;
}

void FilterUnit::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };
    if (cast_editor->hasFocus())
        return;

    auto key { index.data().toUuid() };
    int item_index { cast_editor->findData(key) };
    cast_editor->setCurrentIndex(item_index);
}

void FilterUnit::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };
    const auto key { cast_editor->currentData().toUuid() };
    model->setData(index, key);
}

void FilterUnit::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString& text { tree_model_->Path(index.data().toUuid()) };
    if (text.isEmpty())
        return PaintEmpty(painter, option, index);

    PaintText(text, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize FilterUnit::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString& text = tree_model_->Path(index.data().toUuid());
    return CalculateTextSize(text, option);
}

void FilterUnit::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int bar_width { QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent) };
    const QSize text_size { CalculateTextSize(tree_model_->Path(index.data().toUuid()), option) };

    const int width { std::max(option.rect.width(), text_size.width() + bar_width) };
    const int height { std::max(option.rect.height(), text_size.height()) };

    QRect geom { option.rect };
    geom.setWidth(width);
    geom.setHeight(height);

    editor->setGeometry(geom);
}
