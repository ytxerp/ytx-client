#include "styleditemdelegate.h"

#include <QApplication>
#include <QFontMetrics>
#include <QPainter>

const QLocale StyledItemDelegate::locale_ { QLocale::English, QLocale::UnitedStates };

StyledItemDelegate::StyledItemDelegate(QObject* parent)
    : QStyledItemDelegate { parent }
{
}

void StyledItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    editor->setMinimumWidth(option.rect.width());
    editor->setFixedHeight(option.rect.height());
    editor->setGeometry(option.rect);
}

QSize StyledItemDelegate::CalculateTextSize(CString& text, const QStyleOptionViewItem& option, int margin_factor)
{
    const QFontMetrics fm(option.font);

    if (text.isEmpty())
        return QSize(option.rect.width(), fm.height());

    const int text_margin { QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, option.widget) };
    const int width { std::max(fm.horizontalAdvance(text) + margin_factor * text_margin, option.rect.width()) };

    return QSize(width, option.rect.height());
}

void StyledItemDelegate::PaintText(
    CString& text, QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, Qt::Alignment alignment) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);
    opt.text = text;
    opt.displayAlignment = alignment;

    QStyle* style { QApplication::style() };
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
}

void StyledItemDelegate::PaintColorText(
    CString& text, const QColor& color, QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, Qt::Alignment alignment) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    if (color.isValid()) {
        opt.palette.setColor(QPalette::Text, color);
        opt.palette.setColor(QPalette::HighlightedText, color);
    }

    opt.text = text;
    opt.displayAlignment = alignment;

    QStyle* style { QApplication::style() };
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
}

void StyledItemDelegate::PaintCheckBox(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int active_status) const
{
    QStyleOptionViewItem opt { option };
    initStyleOption(&opt, index);

    QStyle* style { QApplication::style() };
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    QStyleOptionButton check_box {};
    check_box.state = opt.state & ~(QStyle::State_On | QStyle::State_Off);

    const int status { index.data().toInt() };

    // The active status is provided by the caller so this function can be
    // shared across models with different status enums. A raw int is used
    // to keep this function independent of any specific enum class.
    check_box.state |= status == active_status ? QStyle::State_On : QStyle::State_Off;

    auto rect { style->subElementRect(QStyle::SE_CheckBoxIndicator, &opt, opt.widget) };
    rect.moveCenter(opt.rect.center());
    check_box.rect = rect;

    style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &check_box, painter, opt.widget);
}

void StyledItemDelegate::PaintEmpty(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    QStyle* style { QApplication::style() };
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);
}

void StyledItemDelegate::PaintColorRect(QPainter* painter, const QStyleOptionViewItem& option, const QString& color_string) const
{
    if (color_string.isEmpty() || !QColor::isValidColorName(color_string))
        return;

    QStyle* style { QApplication::style() };
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

    const QRect color_rect { option.rect.adjusted(4, 4, -4, -4) };
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(color_string));
    painter->drawRoundedRect(color_rect, ui_const::kCornerRadius, ui_const::kCornerRadius);
    painter->restore();
}

QString StyledItemDelegate::FormatPercentage(double value)
{
    if (qIsNaN(value))
        return QStringLiteral("--");

    return locale_.toString(value * 100.0, 'f', 2) + QLatin1Char('%');
}
