#include "issuedtime.h"

#include "widget/datetimeedit.h"

IssuedTime::IssuedTime(const QString& date_format, QObject* parent)
    : StyledItemDelegate { parent }
    , date_format_ { date_format }
{
}

QWidget* IssuedTime::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    auto* editor { new DateTimeEdit(parent) };
    editor->setDisplayFormat(date_format_);

    return editor;
}

void IssuedTime::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<DateTimeEdit*>(editor) };
    if (cast_editor->hasFocus())
        return;

    auto issued_time { index.data().toDateTime().toLocalTime() };
    if (!issued_time.isValid())
        issued_time = QDateTime::currentDateTime();

    cast_editor->setDateTime(issued_time);
}

void IssuedTime::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_ediotr { static_cast<DateTimeEdit*>(editor) };
    auto issued_time { cast_ediotr->dateTime().toUTC() };

    model->setData(index, issued_time);
}

void IssuedTime::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto issued_time { index.data().toDateTime().toLocalTime() };
    if (!issued_time.isValid())
        return PaintEmpty(painter, option, index);

    PaintText(issued_time.toString(date_format_), painter, option, index, Qt::AlignCenter);
}

QSize IssuedTime::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto text { index.data().toDateTime().toString(date_format_) };
    return CalculateTextSize(text, option, kCoefficient5);
}
