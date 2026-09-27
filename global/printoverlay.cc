#include "printoverlay.h"

#include <QtGui/qpainter.h>

#include "component/constantstring.h"
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
                const int best_size { FindBestFontSize(painter, text, available_width, max_font_size) };

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
            align |= IsNumber(text) ? Qt::AlignRight : Qt::AlignLeft;

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
    DrawText(painter, QStringLiteral("footer_initial_total_upper"), QStringLiteral("大写：") + NumberToChineseUpper(amount_str.toDouble()));
    DrawText(painter, QStringLiteral("footer_page_info"), QString::asprintf("%d/%d", page_num, total_pages));

    painter->restore();
}

int PrintOverlay::FindBestFontSize(QPainter* painter, const QString& text, int max_width, int max_font, int min_font)
{
    // Binary search boundaries
    int low { min_font };
    int high { max_font };
    int best { min_font }; // Best font size found so far

    // Copy current painter font
    QFont font { painter->font() };

    // Perform binary search to find the largest font size that fits
    while (low <= high) {
        const int mid { (low + high) / 2 }; // Middle font size to test
        font.setPointSize(mid);

        const QFontMetrics fm { font };
        const int text_width { fm.horizontalAdvance(text) }; // Width of text in current font

        if (text_width <= max_width) {
            // Current font fits, try a larger size
            best = mid;
            low = mid + 1;
        } else {
            // Too wide, try a smaller size
            high = mid - 1;
        }
    }

    return best; // Return the largest font size that fits
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

QString PrintOverlay::NumberToChineseUpper(double value)
{
    // Handle negative values
    if (value < 0) {
        return "负" + NumberToChineseUpper(-value);
    }

    // Check if amount is too large
    if (value >= 1e15) {
        return "金额过大";
    }

    // Static constants initialized once
    static const QStringList digits { "零", "壹", "贰", "叁", "肆", "伍", "陆", "柒", "捌", "玖" };
    static const QStringList big_units { "", "万", "亿", "兆" };
    static const QRegularExpression multi_zero("零{2,}");
    static const QRegularExpression zero_before_unit("零([万亿兆])");
    static const QRegularExpression trailing_zero("零+$");

    // Separate integer and decimal parts
    const qint64 integer { static_cast<qint64>(value) };
    const int fraction { qRound((value - static_cast<double>(integer)) * 100) };

    QString result {};
    result.reserve(64);

    // Convert integer part
    if (integer == 0) {
        result = "零元";
    } else {
        QString temp {};
        temp.reserve(48);

        qint64 remaining { integer };
        int section_idx { 0 };
        bool need_zero { false }; // Flag to indicate if zero should be prepended

        while (remaining > 0) {
            const int section { static_cast<int>(remaining % 10000) };
            remaining /= 10000;

            if (section > 0) {
                QString section_str { ConvertSection(section, digits) };

                // Prepend zero if previous sections were empty
                if (need_zero) {
                    section_str = "零" + section_str;
                }

                section_str += big_units[section_idx];
                temp = section_str + temp;
                need_zero = false;
            } else if (!temp.isEmpty()) {
                // Current section is zero but has following content
                need_zero = true;
            }

            section_idx++;
        }

        // Clean up redundant zeros
        temp.replace(multi_zero, "零");
        temp.replace(zero_before_unit, "\\1");
        temp.remove(trailing_zero);

        result = temp + "元";
    }

    // Convert decimal part (jiao and fen)
    if (fraction == 0) {
        result += "整";
    } else {
        const int jiao { fraction / 10 };
        const int fen { fraction % 10 };

        if (jiao > 0) {
            result += digits[jiao] + "角";
            if (fen > 0) {
                result += digits[fen] + "分";
            }
        } else {
            // Zero jiao but non-zero fen requires explicit zero
            result += "零" + digits[fen] + "分";
        }
    }

    return result;
}

QString PrintOverlay::ConvertSection(int section, const QStringList& digits)
{
    if (section == 0 || section > 9999) {
        return QString();
    }

    QString result {};
    result.reserve(16);

    // Extract individual digits
    const int qian { section / 1000 }; // Thousands digit
    const int bai { (section / 100) % 10 }; // Hundreds digit
    const int shi { (section / 10) % 10 }; // Tens digit
    const int ge { section % 10 }; // Ones digit

    // Process thousands place
    if (qian > 0) {
        result += digits[qian] + "仟";
    }

    // Process hundreds place
    if (bai > 0) {
        result += digits[bai] + "佰";
    } else if (qian > 0 && (shi > 0 || ge > 0)) {
        // Zero in hundreds but has higher and lower non-zero digits
        result += "零";
    }

    // Process tens place
    if (shi > 0) {
        result += digits[shi] + "拾";
    } else if (bai > 0 && ge > 0) {
        // Zero in tens but has higher and lower non-zero digits
        result += "零";
    }

    // Process ones place
    if (ge > 0) {
        result += digits[ge];
    }

    return result;
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
