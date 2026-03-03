#include "double.h"

#include "component/constantdouble.h"
#include "widget/doublespinbox.h"

Double::Double(const int& decimal, double min, double max, CString& placeholder, QObject* parent)
    : StyledItemDelegate { parent }
    , decimal_ { decimal }
    , max_ { max }
    , min_ { min }
    , placeholder_ { placeholder }
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
    if (std::abs(value) < kTolerance)
        return PaintEmpty(painter, option, index);

    PaintText(locale_.toString(value, 'f', decimal_), painter, option, index, Qt::AlignRight | Qt::AlignVCenter);
}

QSize Double::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString text { locale_.toString(index.data().toDouble(), 'f', decimal_) };
    const QString& str { text.size() > placeholder_.size() ? text : placeholder_ };
    return CalculateTextSize(str, option);
}
