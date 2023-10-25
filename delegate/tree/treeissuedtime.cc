#include "treeissuedtime.h"

#include "widget/datetimeedit.h"

TreeIssuedTime::TreeIssuedTime(const QString& date_format, QObject* parent)
    : StyledItemDelegate { parent }
    , date_format_ { date_format }
{
}

QWidget* TreeIssuedTime::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    auto* editor { new DateTimeEdit(parent) };
    editor->setDisplayFormat(date_format_);

    return editor;
}

void TreeIssuedTime::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto issued_time { index.data().toDateTime().toLocalTime() };
    if (!issued_time.isValid())
        issued_time = QDateTime::currentDateTime();

    auto* cast_ediotr { static_cast<DateTimeEdit*>(editor) };
    cast_ediotr->setDateTime(issued_time);
}

void TreeIssuedTime::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_ediotr { static_cast<DateTimeEdit*>(editor) };
    auto issued_time { cast_ediotr->dateTime().toUTC() };

    model->setData(index, issued_time);
}

void TreeIssuedTime::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto issued_time { index.data().toDateTime().toLocalTime() };
    if (!issued_time.isValid())
        return QStyledItemDelegate::paint(painter, option, index);

    PaintText(issued_time.toString(date_format_), painter, option, index, Qt::AlignCenter);
}

QSize TreeIssuedTime::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto text { index.data().toDateTime().toString(date_format_) };
    return CalculateTextSize(text, option, kCoefficient5);
}
