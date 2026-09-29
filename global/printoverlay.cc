#include "printoverlay.h"

#include <QtGui/qpainter.h>

#include "component/constantstring.h"
#include "printhub.h"
#include "utils/nodeutils.h"

bool PrintOverlay::LoadTemplate(QSettings& settings)
{
    field_position_.clear();
    columns_.clear();
    column_widths_.clear();

    font_size_ = 12;
    rows_ = 7;
    row_height_ = 0.8;

    // Page
    settings.beginGroup(QStringLiteral("page"));

    font_size_ = settings.value(QStringLiteral("font_size"), 12).toInt();

    settings.endGroup();

    // Header
    settings.beginGroup(QStringLiteral("header"));

    ReadFieldPosition(settings, QStringLiteral("partner"));

    ReadFieldPosition(settings, QStringLiteral("issued_time"));

    ReadFieldPosition(settings, QStringLiteral("code"));

    settings.endGroup();

    // Table
    settings.beginGroup(QStringLiteral("table"));

    ReadFieldPosition(settings, QStringLiteral("left_top"));

    rows_ = settings.value(QStringLiteral("rows"), 7).toInt();

    row_height_ = settings.value(QStringLiteral("row_height"), 0.8).toDouble();

    columns_ = settings.value(QStringLiteral("columns")).toStringList();

    for (QString& column : columns_)
        column = column.trimmed();

    const QStringList widths { settings.value(QStringLiteral("column_widths")).toStringList() };

    column_widths_.reserve(widths.size());

    bool widths_valid { true };

    for (const QString& value : widths) {
        bool ok {};

        const qreal width { value.trimmed().toDouble(&ok) };

        if (!ok) {
            qWarning() << "Invalid column width value:" << value;

            widths_valid = false;
            break;
        }

        column_widths_.emplaceBack(width);
    }

    settings.endGroup();

    // Footer
    settings.beginGroup(QStringLiteral("footer"));

    ReadFieldPosition(settings, QStringLiteral("employee"));

    ReadFieldPosition(settings, QStringLiteral("unit"));

    ReadFieldPosition(settings, QStringLiteral("initial_total"));

    ReadFieldPosition(settings, QStringLiteral("initial_total_upper"));

    ReadFieldPosition(settings, QStringLiteral("page_info"));

    settings.endGroup();

    if (!widths_valid)
        return false;

    if (rows_ <= 0 || row_height_ <= 0.0) {
        qWarning() << "Invalid overlay table configuration:"
                   << "rows:" << rows_ << "row_height:" << row_height_;

        return false;
    }

    if (columns_.isEmpty() || columns_.size() != column_widths_.size()) {
        qWarning() << "Overlay column configuration mismatch:"
                   << "columns:" << columns_.size() << "widths:" << column_widths_.size();

        return false;
    }

    if (std::ranges::any_of(column_widths_, [](qreal width) { return width <= 0.0; })) {
        qWarning() << "Overlay column width must be greater than 0";

        return false;
    }

    return true;
}

void PrintOverlay::Render(QPrinter* printer, const NodeO* node_o, const QList<Entry*>& entry_list, const CSectionConfig* section_config)
{
    if (!printer || !node_o || !section_config)
        return;

    printer_ = printer;
    node_o_ = node_o;
    entry_list_ = &entry_list;
    section_config_ = section_config;

    RenderAllPages(printer);
}

void PrintOverlay::RenderAllPages(QPrinter* printer)
{
    const long long total_pages { qMax<long long>(1, (entry_list_->size() + rows_ - 1) / rows_) };

    QPainter painter(printer);
    if (!painter.isActive())
        return;

    painter.setPen(QPen(Qt::black, 0));

    QFont font { painter.font() };
    font.setPointSize(font_size_);
    painter.setFont(font);

    for (long long page_num = 0; page_num != total_pages; ++page_num) {
        if (page_num != 0) {
            printer->newPage();
        }

        DrawHeader(&painter);

        const long long start_index { page_num * rows_ };
        const long long end_index { qMin((page_num + 1) * rows_, entry_list_->size()) };

        DrawTable(&painter, start_index, end_index);
        DrawFooter(&painter, page_num + 1, total_pages);
    }
}

void PrintOverlay::DrawHeader(QPainter* painter)
{
    painter->save();

    DrawText(painter, QStringLiteral("partner"), MasterDataRegistry::Instance().PartnerName(node_o_->partner_id));
    DrawText(painter, QStringLiteral("issued_time"), node_o_->issued_time.toString(datetime_format::kDashedDate));
    DrawText(painter, QStringLiteral("code"), node_o_->code);

    painter->restore();
}

/*!
 * \brief Draw table rows with auto-fit text (shrink to fit cell width)
 *
 * Table columns and their order are defined by the template configuration.
 *
 * \param painter Painter object for drawing
 * \param start_index Start index in entry_list_
 * \param end_index End index in entry_list_ (exclusive)
 *
 * \note Text automatically shrinks to fit column width (min font size: 1pt)
 * \note Numbers are right-aligned, text is left-aligned
 */
void PrintOverlay::DrawTable(QPainter* painter, long long start_index, long long end_index)
{
    painter->save();

    const qreal left { CmToPixel(GetFieldX(QStringLiteral("left_top"))) };

    const qreal top { CmToPixel(GetFieldY(QStringLiteral("left_top"))) };

    const qreal row_height { CmToPixel(row_height_) };

    const QFont original_font { painter->font() };

    const QFontMetricsF fm { original_font, painter->device() };

    const int max_font_size { original_font.pointSize() };

    constexpr qreal kPaddingCm { 0.1 };
    const qreal padding { CmToPixel(kPaddingCm) };

    const auto& master { MasterDataRegistry::Instance() };

    const auto& partner { PartnerInventoryRegistry::Instance() };

    for (long long row = 0; row != end_index - start_index; ++row) {
        const auto* entry { entry_list_->at(start_index + row) };

        qreal x { left };

        for (qsizetype col = 0; col != columns_.size(); ++col) {
            const qreal col_width { CmToPixel(column_widths_.at(col)) };

            const QRectF cell_rect { x, top + static_cast<qreal>(row) * row_height, col_width, row_height };

            const QString text { GetColumnText(columns_.at(col), entry, master, partner) };

            const qreal text_width { fm.horizontalAdvance(text) };

            const qreal available_width { qMax<qreal>(1.0, col_width - padding * 2) };

            if (text_width > available_width) {
                QFont font { original_font };

                const int best_size { PrintHub::FindBestFontSize(font, painter->device(), text, static_cast<int>(available_width), max_font_size) };

                font.setPointSize(best_size);
                painter->setFont(font);
            } else {
                painter->setFont(original_font);
            }

            Qt::Alignment align { Qt::AlignVCenter };

            align |= PrintHub::IsNumber(text) ? Qt::AlignRight : Qt::AlignLeft;

            painter->drawText(cell_rect.adjusted(padding, 0, -padding, 0), static_cast<int>(align), text);

            x += col_width;
        }
    }

    painter->restore();
}

void PrintOverlay::DrawFooter(QPainter* painter, int page_num, int total_pages)
{
    painter->save();

    DrawText(painter, QStringLiteral("employee"), MasterDataRegistry::Instance().PartnerName(node_o_->employee_id));

    // unit, direction_rule
    {
        painter->save();

        QFont font { painter->font() };
        font.setBold(true);
        font.setPointSize(font.pointSize() + 2);

        painter->setFont(font);

        const QString text { node::UnitString(NodeUnit(node_o_->unit)) + "/" + PrintHub::DirectionRuleString(node_o_->direction_rule) };

        DrawText(painter, QStringLiteral("unit"), text);

        painter->restore();
    }

    const QString amount_str { QString::number(node_o_->initial_total, 'f', section_config_->amount_decimal) };

    DrawText(painter, QStringLiteral("initial_total"), amount_str);
    DrawText(painter, QStringLiteral("initial_total_upper"), QObject::tr("Uppercase: ") + PrintHub::NumberToChineseUpper(amount_str.toDouble()));
    DrawText(painter, QStringLiteral("page_info"), QString::asprintf("%d/%d", page_num, total_pages));

    painter->restore();
}

QString PrintOverlay::GetColumnText(const QString& column, const Entry* entry, const MasterDataRegistry& master, const PartnerInventoryRegistry& partner) const
{
    if (column == QStringLiteral("internal_sku"))
        return master.InventoryPath(entry->rhs_node);

    if (column == QStringLiteral("external_sku"))
        return partner.ExternalSku(node_o_->partner_id, entry->rhs_node);

    if (column == QStringLiteral("description"))
        return entry->description;

    const auto* d_entry { static_cast<const EntryO*>(entry) };

    if (column == QStringLiteral("count"))
        return QString::number(d_entry->count, 'f', section_config_->quantity_decimal);

    if (column == QStringLiteral("measure"))
        return QString::number(d_entry->measure, 'f', section_config_->quantity_decimal);

    if (column == QStringLiteral("unit_price"))
        return QString::number(d_entry->unit_price, 'f', section_config_->rate_decimal);

    if (column == QStringLiteral("amount"))
        return QString::number(d_entry->initial, 'f', section_config_->amount_decimal);

    return {};
}

void PrintOverlay::DrawText(QPainter* painter, const QString& field, const QString& text)
{
    const auto& opt_pos { field_position_.value(field) };

    if (!opt_pos.has_value()) {
        qDebug() << "Field not found in config:" << field;

        return;
    }

    const FieldPosition& pos { *opt_pos };

    if (qFuzzyIsNull(pos.x) && qFuzzyIsNull(pos.y)) {
        return;
    }

    const qreal x { CmToPixel(pos.x) };

    const qreal y { CmToPixel(pos.y) };

    painter->drawText(QPointF(x, y), text);
}

void PrintOverlay::ReadFieldPosition(QSettings& settings, const QString& field)
{
    if (!settings.contains(field)) {
        qWarning() << "Position setting not found, field:" << field;

        field_position_[field] = std::nullopt;

        return;
    }

    const QStringList position { settings.value(field).toStringList() };

    if (position.size() != 2) {
        qWarning() << "Invalid position value, field:" << field << "value:" << position;

        field_position_[field] = std::nullopt;

        return;
    }

    bool x_ok {};
    bool y_ok {};

    const qreal x { position.at(0).trimmed().toDouble(&x_ok) };

    const qreal y { position.at(1).trimmed().toDouble(&y_ok) };

    if (x_ok && y_ok) {
        field_position_[field] = FieldPosition { x, y };
    } else {
        qWarning() << "Invalid position coordinates, field:" << field << "value:" << position;

        field_position_[field] = std::nullopt;
    }
}
