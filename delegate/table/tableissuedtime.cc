#include "tableissuedtime.h"

#include "widget/datetimeeditautoselect.h"

TableIssuedTime::TableIssuedTime(const QString& date_format, QObject* parent)
    : StyledItemDelegate { parent }
    , date_format_ { date_format }
{
}

QWidget* TableIssuedTime::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    auto* editor { new DateTimeEditAutoSelect(parent) };
    editor->setDisplayFormat(date_format_);

    return editor;
}

void TableIssuedTime::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<DateTimeEditAutoSelect*>(editor) };
    if (cast_editor->hasFocus())
        return;

    auto issued_time { index.data().toDateTime().toLocalTime() };
    if (!issued_time.isValid())
        issued_time = last_insert_.isValid() ? last_insert_.addSecs(1) : QDateTime::currentDateTime();

    cast_editor->setDateTime(issued_time);
}

void TableIssuedTime::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_ediotr { static_cast<DateTimeEditAutoSelect*>(editor) };
    auto issued_time { cast_ediotr->dateTime() };

    last_insert_ = issued_time.date() == QDate::currentDate() ? QDateTime() : issued_time;
    model->setData(index, issued_time.toUTC());
}

void TableIssuedTime::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto issued_time { index.data().toDateTime().toLocalTime() };
    if (!issued_time.isValid())
        return PaintEmpty(painter, option, index);

    PaintText(issued_time.toString(date_format_), painter, option, index, Qt::AlignCenter);
}

QSize TableIssuedTime::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto text { index.data().toDateTime().toString(date_format_) };
    return CalculateTextSize(text, option, kCoefficient5);
}
