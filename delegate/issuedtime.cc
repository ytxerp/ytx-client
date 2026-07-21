#include "issuedtime.h"

#include "widget/datetimeedit.h"

IssuedTime::IssuedTime(const QString& date_format, bool remember_last, QObject* parent)
    : StyledItemDelegate { parent }
    , date_format_ { date_format }
    , remember_last_ { remember_last }
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

    auto issued_time { index.data().toDateTime() };

    if (!issued_time.isValid())
        issued_time = (remember_last_ && last_issued_.isValid()) ? last_issued_ : QDateTime::currentDateTime();

    cast_editor->setDateTime(issued_time);
}

void IssuedTime::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<DateTimeEdit*>(editor) };
    auto issued_time { cast_editor->dateTime() };

    if (remember_last_)
        last_issued_ = issued_time;

    model->setData(index, issued_time);
}

void IssuedTime::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto issued_time { index.data().toDateTime() };
    if (!issued_time.isValid())
        return QStyledItemDelegate::paint(painter, option, index);

    PaintText(issued_time.toString(date_format_), painter, option, index, Qt::AlignCenter);
}

QSize IssuedTime::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto text { index.data().toDateTime().toString(date_format_) };
    return CalculateTextSize(text, option);
}
