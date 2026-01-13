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
    editor->setGeometry(option.rect);
}

QSize StyledItemDelegate::CalculateTextSize(CString& text, const QStyleOptionViewItem& option, int coefficient)
{
    const QFontMetrics fm(option.font);

    if (text.isEmpty())
        return QSize(option.rect.width(), fm.height());

    const int text_margin { QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, option.widget) };
    const int width { std::max(fm.horizontalAdvance(text) + coefficient * text_margin, option.rect.width()) };
    const int height { std::max(fm.height(), option.rect.height()) };

    return QSize(width, height);
}

void StyledItemDelegate::PaintText(
    CString& text, QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, Qt::Alignment alignment) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);
    opt.text = text;
    opt.displayAlignment = alignment;

    static QStyle* style { QApplication::style() };
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
}

void StyledItemDelegate::PaintCheckBox(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt { option };
    initStyleOption(&opt, index);

    static QStyle* style { QApplication::style() };
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    QStyleOptionButton check_box {};
    check_box.state = opt.state & ~(QStyle::State_On | QStyle::State_Off);

    const int status { index.data().toInt() };

    // Note:
    // This function is shared by multiple models:
    //   - In Entry models, 0 = EntryStatus::kUnmarked, 1 = EntryStatus::kMarked.
    //   - In Node models, 0 = NodeStatus::kEditable, 1 = NodeStatus::kReviewed, 2 = NodeStatus::kVoided.
    // We therefore use a raw int instead of binding to a single enum class.
    switch (status) {
    case 0:
        check_box.state |= QStyle::State_Off;
        break;
    case 1:
        check_box.state |= QStyle::State_On;
        break;
    // If more states are added later (e.g., Reviewed, Voided),
    // they can be handled here explicitly.
    default:
        check_box.state |= QStyle::State_Off;
        break;
    }

    auto rect { style->subElementRect(QStyle::SE_CheckBoxIndicator, &opt, opt.widget) };
    rect.moveCenter(opt.rect.center());
    check_box.rect = rect;

    style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &check_box, painter, opt.widget);
}

void StyledItemDelegate::PaintEmpty(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    static QStyle* style { QApplication::style() };
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);
}

void StyledItemDelegate::PaintColorRect(QPainter* painter, const QStyleOptionViewItem& option, const QString& color_string) const
{
    if (color_string.isEmpty() || !QColor::isValidColorName(color_string))
        return;

    static QStyle* style { QApplication::style() };
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

    const QRect color_rect { option.rect.adjusted(2, 2, -2, -2) };
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(color_string));
    painter->drawRoundedRect(color_rect, 2, 2);
    painter->restore();
}
