#include "double.h"

#include "widget/doublespinbox.h"

Double::Double(const int& decimal, double min, double max, int coefficient, QObject* parent)
    : StyledItemDelegate { parent }
    , decimal_ { decimal }
    , max_ { max }
    , min_ { min }
    , coefficient_ { coefficient }
{
}

QWidget* Double::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    auto* editor { new DoubleSpinBox(parent) };
    editor->setDecimals(decimal_);
    editor->setMinimum(min_);
    editor->setMaximum(max_);

    return editor;
}

void Double::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<DoubleSpinBox*>(editor) };
    if (cast_editor->hasFocus())
        return;

    cast_editor->setValue(index.data().toDouble());
}

void Double::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<DoubleSpinBox*>(editor) };
    model->setData(index, cast_editor->value());
}

void Double::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const double value { index.data().toDouble() };
    if (value == 0)
        return PaintEmpty(painter, option, index);

    PaintText(locale_.toString(value, 'f', decimal_), painter, option, index, Qt::AlignRight | Qt::AlignVCenter);
}

QSize Double::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const double value { index.data().toDouble() };
    return CalculateTextSize(locale_.toString(value, 'f', decimal_), option, coefficient_);
}
