#include "doubleguard.h"

#include "widget/doublespinbox.h"

DoubleGuard::DoubleGuard(const int& decimal, double min, double max, int coefficient, QObject* parent)
    : StyledItemDelegate { parent }
    , decimal_ { decimal }
    , max_ { max }
    , min_ { min }
    , coefficient_ { coefficient }
{
}

QWidget* DoubleGuard::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    auto* editor { new DoubleSpinBox(parent) };
    editor->setDecimals(decimal_);
    editor->setMinimum(min_);
    editor->setMaximum(max_);

    return editor;
}

void DoubleGuard::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<DoubleSpinBox*>(editor) };
    if (cast_editor->hasFocus())
        return;

    cast_editor->setValue(index.data().toDouble());
}

void DoubleGuard::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<DoubleSpinBox*>(editor) };
    model->setData(index, cast_editor->value());
}

void DoubleGuard::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const double value { index.data().toDouble() };
    if (value == 0)
        return QStyledItemDelegate::paint(painter, option, index);

    PaintText(locale_.toString(value, 'f', decimal_), painter, option, index, Qt::AlignRight | Qt::AlignVCenter);
}

QSize DoubleGuard::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const double value { index.data().toDouble() };
    return CalculateTextSize(locale_.toString(value, 'f', decimal_), option, coefficient_);
}
