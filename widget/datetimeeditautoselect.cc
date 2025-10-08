#include "datetimeeditautoselect.h"

void DateTimeEditAutoSelect::focusInEvent(QFocusEvent* event)
{
    DateTimeEdit::focusInEvent(event);
    setSelectedSection(QDateTimeEdit::DaySection);
}

DateTimeEditAutoSelect::DateTimeEditAutoSelect(QWidget* parent)
    : DateTimeEdit { parent }
{
}
