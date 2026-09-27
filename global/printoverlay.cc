#include "printoverlay.h"

#include <QtGui/qpainter.h>

#include "component/constantstring.h"
#include "printhub.h"
#include "utils/nodeutils.h"

bool PrintOverlay::LoadTemplate(QSettings& settings)
{
    field_position_.clear();
    column_widths_.clear();

    font_size_ = 12;
    row_height_ = 30;

    settings.beginGroup(QStringLiteral("page"));
    font_size_ = settings.value(QStringLiteral("font_size"), 12).toInt();
    settings.endGroup();

    settings.beginGroup(QStringLiteral("overlay"));

    static const QList<QString> fields { QStringLiteral("header_partner"), QStringLiteral("header_issued_time"), QStringLiteral("header_code"),

        QStringLiteral("table_left_top"), QStringLiteral("table_rows_columns"),

        QStringLiteral("footer_employee"), QStringLiteral("footer_unit"), QStringLiteral("footer_initial_total"), QStringLiteral("footer_initial_total_upper"),
        QStringLiteral("footer_page_info") };

    for (const auto& field : fields)
        ReadFieldPosition(settings, field);

    row_height_ = settings.value(QStringLiteral("table_row_height"), 30).toInt();

    const auto widths { settings.value(QStringLiteral("table_column_widths")).toStringList() };

    column_widths_.reserve(widths.size());

    std::ranges::transform(widths, std::back_inserter(column_widths_), [](const QString& value) { return value.toInt(); });

    settings.endGroup();

    const int columns { GetFieldY(QStringLiteral("table_rows_columns")) };

    if (columns <= 0 || column_widths_.size() != columns) {
        qWarning() << "Overlay column count does not match column widths:" << columns << column_widths_.size();
        return false;
    }

    return true;
}

void PrintOverlay::Render(QPrinter* printer, const NodeO* node_o, const QList<Entry*>& entry_list, const CSectionConfig* section_config)
{
    node_o_ = node_o;
    entry_list_ = &entry_list;
    section_config_ = section_config;

    RenderAllPages(printer);
}

void PrintOverlay::RenderAllPages(QPrinter* printer)
{
    // Fetch configuration values for rows and columns
    const int rows { GetFieldX(QStringLiteral("table_rows_columns")) };
    if (rows <= 0)
        return;

    // Calculate total pages required based on the total rows and rows per page
    const long long total_pages { (entry_list_->size() + rows - 1) / rows }; // Ceiling division to determine total pages

    QPainter painter(printer);
    painter.setPen(QPen(Qt::black, 0));

    QFont font { painter.font() };
    font.setPointSize(font_size_);
    painter.setFont(font);

    // Start rendering each page
    for (long long page_num = 0; page_num != total_pages; ++page_num) {
        // Begin a new page
        if (page_num != 0) {
            printer->newPage();
        }

        // Draw header (e.g., title, date, etc.)
        DrawHeader(&painter);

        // Draw content on the page
        const long long start_index { page_num * rows };
        const long long end_index { qMin((page_num + 1) * rows, entry_list_->size()) };
        DrawTable(&painter, start_index, end_index);

        // Draw footer (e.g., page number, etc.)
        DrawFooter(&painter, page_num + 1, total_pages);
    }
}

void PrintOverlay::DrawHeader(QPainter* painter)
{
    painter->save();

    DrawText(painter, QStringLiteral("header_partner"), MasterDataRegistry::Instance().PartnerName(node_o_->partner_id));
    DrawText(painter, QStringLiteral("header_issued_time"), node_o_->issued_time.toString(datetime_format::kDashedDate));
    DrawText(painter, QStringLiteral("header_code"), node_o_->code);

    painter->restore();
}

/*!
 * \brief Draw table rows with auto-fit text (shrink to fit cell width)
 *
 * Table columns (left to right):
 * 0: Internal Sku    - Internal product SKU/code
 * 1: External Sku    - Customer/external product SKU/code
 * 2: Description     - Product description
 * 3: Count           - Quantity count
 * 4: Measure         - Unit of measure
 * 5: Unit Price      - Price per unit
 * 6: Amount          - Total amount (Count × Unit Price)
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

    const int columns { GetFieldY(QStringLiteral("table_rows_columns")) };
    const int left { GetFieldX(QStringLiteral("table_left_top")) };
    const int top { GetFieldY(QStringLiteral("table_left_top")) };

    const QFont original_font { painter->font() }; // Save original font
    const QFontMetrics fm(original_font);
    const int max_font_size { original_font.pointSize() };
    const int padding { 4 }; // Cell padding

    const auto& master { MasterDataRegistry::Instance() };
    const auto& partner { PartnerInventoryRegistry::Instance() };

    for (int row = 0; row != end_index - start_index; ++row) {
        const auto* entry { entry_list_->at(start_index + row) };
        int x { left };

        for (int col = 0; col != columns; ++col) {
            const int col_width { column_widths_.at(col) };
            if (col_width <= 0) {
                continue;
            }

            const QRect cell_rect(x, top + row * row_height_, col_width, row_height_);
            const QString text { GetColumnText(col, entry, master, partner) };
            const int text_width { fm.horizontalAdvance(text) };
            const int available_width { col_width - padding * 2 };

            // Find and apply best font size
            if (text_width > available_width) {
                // Reset to original font for each cell
                QFont font { original_font };
                const int best_size { PrintHub::FindBestFontSize(painter, text, available_width, max_font_size) };

                font.setPointSize(best_size);
                painter->setFont(font);

                qDebug() << "Shrink font:"
                         << "Text=" << text << "ColWidth=" << col_width << "TextWidth=" << text_width << "BestSize=" << best_size;
            } else {
                painter->setFont(original_font);

                qDebug() << "Use original font:"
                         << "Text=" << text << "ColWidth=" << col_width << "TextWidth=" << text_width;
            }

            // Determine alignment
            Qt::Alignment align { Qt::AlignVCenter };
            align |= PrintHub::IsNumber(text) ? Qt::AlignRight : Qt::AlignLeft;

            painter->drawText(cell_rect, static_cast<int>(align), text);

            x += col_width;
        }
    }

    // Restore original painter
    painter->restore();
}

void PrintOverlay::DrawFooter(QPainter* painter, int page_num, int total_pages)
{
    painter->save();

    DrawText(painter, QStringLiteral("footer_employee"), MasterDataRegistry::Instance().PartnerName(node_o_->employee_id));

    // unit, direction_rule
    {
        const QString text { node::UnitString(NodeUnit(node_o_->unit)) + "/" + node::DirectionRuleString(node_o_->direction_rule) };
        DrawText(painter, QStringLiteral("footer_unit"), text);
    }

    const QString amount_str { QString::number(node_o_->initial_total, 'f', section_config_->amount_decimal) };

    DrawText(painter, QStringLiteral("footer_initial_total"), amount_str);
    DrawText(painter, QStringLiteral("footer_initial_total_upper"), QStringLiteral("大写：") + PrintHub::NumberToChineseUpper(amount_str.toDouble()));
    DrawText(painter, QStringLiteral("footer_page_info"), QString::asprintf("%d/%d", page_num, total_pages));

    painter->restore();
}

QString PrintOverlay::GetColumnText(int col, const Entry* entry, const MasterDataRegistry& master, const PartnerInventoryRegistry& partner) const
{
    const auto* d_entry { static_cast<const EntryO*>(entry) };

    switch (col) {
    case 0:
        return master.InventoryPath(entry->rhs_node);
    case 1:
        return partner.ExternalSku(node_o_->partner_id, entry->rhs_node);
    case 2:
        return entry->description;
    case 3:
        return QString::number(d_entry->count, 'f', section_config_->quantity_decimal);
    case 4:
        return QString::number(d_entry->measure, 'f', section_config_->quantity_decimal);
    case 5:
        return QString::number(d_entry->unit_price, 'f', section_config_->rate_decimal);
    case 6:
        return QString::number(d_entry->initial, 'f', section_config_->amount_decimal);
    default:
        return {};
    }
}

void PrintOverlay::DrawText(QPainter* painter, const QString& field, const QString& text)
{
    const auto& opt_pos { field_position_.value(field) };
    if (!opt_pos.has_value()) {
        qDebug() << "Field not found in config:" << field;
        return;
    }

    const FieldPosition& pos { *opt_pos };
    if (pos.x == 0 && pos.y == 0) {
        return;
    }

    qDebug() << "Drawing field:" << field << "at position:" << pos.x << "," << pos.y << "text:" << text;
    painter->drawText(pos.x, pos.y, text);
}

void PrintOverlay::ReadFieldPosition(QSettings& settings, const QString& field)
{
    if (!settings.contains(field)) {
        qWarning() << "Position setting not found, field:" << field;
        field_position_[field] = std::nullopt;
        return;
    }

    const auto position { settings.value(field).value<QVariantList>() };

    if (position.size() != 2) {
        qWarning() << "Invalid position value, field:" << field;
        field_position_[field] = std::nullopt;
        return;
    }

    bool x_ok {};
    bool y_ok {};

    const int x { position[0].toInt(&x_ok) };
    const int y { position[1].toInt(&y_ok) };

    if (x_ok && y_ok) {
        field_position_[field] = FieldPosition { x, y };
    } else {
        qWarning() << "Invalid position coordinates, field:" << field << "value:" << position;

        field_position_[field] = std::nullopt;
    }
}
