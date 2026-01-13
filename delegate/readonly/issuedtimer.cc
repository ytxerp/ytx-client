#include "issuedtimer.h"

IssuedTimeR::IssuedTimeR(const QString& date_format, QObject* parent)
    : StyledItemDelegate { parent }
    , date_format_ { date_format }
{
}

void IssuedTimeR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const auto issued_time { index.data().toDateTime().toLocalTime() };
    Q_ASSERT(issued_time.isValid());

    PaintText(issued_time.toString(date_format_), painter, option, index, Qt::AlignCenter);
}

QSize IssuedTimeR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const auto text { index.data().toDateTime().toString(date_format_) };
    return CalculateTextSize(text, option, kCoefficient5);
}
