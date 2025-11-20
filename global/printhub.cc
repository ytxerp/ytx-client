#include "printhub.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QPainter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinterInfo>
#include <QVariant>

void PrintHub::SetValue(const NodeO* node_o, const QList<Entry*>& entry_list)
{
    node_o_ = node_o;
    entry_list_ = entry_list;
}

void PrintHub::Preview()
{
    QPrinter printer { QPrinter::ScreenResolution };
    ApplyConfig(&printer);

    printer.setPrinterName(app_config_->printer);

    QPrintPreviewDialog preview(&printer);

    QObject::connect(&preview, &QPrintPreviewDialog::paintRequested, &preview, [this](QPrinter* printer) { RenderAllPages(printer); });
    preview.exec();
}

void PrintHub::Print()
{
    QPrinter printer(QPrinter::ScreenResolution);
    ApplyConfig(&printer);

    const auto available_printers { QPrinterInfo::availablePrinterNames() };
    const QString& printer_name { app_config_->printer };

    if (printer_name.isEmpty() || !available_printers.contains(printer_name)) {
        QPrintDialog dialog(&printer);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }
    } else {
        printer.setPrinterName(printer_name);
    }

    RenderAllPages(&printer);
}

void PrintHub::ScanTemplate()
{
#ifdef Q_OS_MAC
    constexpr auto folder_name { "../Resources/print_template" };
#elif defined(Q_OS_WIN32)
    constexpr auto folder_name { "print_template" };
#else
    return;
#endif

    const QString folder_path { QCoreApplication::applicationDirPath() + QDir::separator() + QString::fromUtf8(folder_name) };
    QDir dir(folder_path);

    if (!dir.exists()) {
        return;
    }

    const QStringList name_filters { "*.ini" };
    const QDir::Filters entry_filters { QDir::Files | QDir::NoSymLinks };
    const QFileInfoList file_list { dir.entryInfoList(name_filters, entry_filters) };

    for (const auto& fileInfo : file_list) {
        template_map_.insert(fileInfo.baseName(), fileInfo.absoluteFilePath());
    }
}

bool PrintHub::LoadTemplate(const QString& template_name)
{
    if (template_name == current_template_) {
        return true;
    }

    page_values_.clear();
    field_position_.clear();

    // Page settings
    QSettings settings(template_name, QSettings::IniFormat);

    {
        settings.beginGroup(QStringLiteral("page"));

        page_values_[QStringLiteral("page_size")] = settings.value(QStringLiteral("page_size"), QStringLiteral("A5"));
        page_values_[QStringLiteral("orientation")] = settings.value(QStringLiteral("orientation"), QStringLiteral("Portrait"));
        page_values_[QStringLiteral("font_size")] = settings.value(QStringLiteral("font_size"), 12);

        settings.endGroup();
    }

    static const QList<QString> header_fields { QStringLiteral("partner"), QStringLiteral("issued_time") };
    static const QList<QString> table_fields { QStringLiteral("left_top"), QStringLiteral("rows_columns") };
    static const QList<QString> footer_fields { QStringLiteral("employee"), QStringLiteral("unit"), QStringLiteral("initial_total"),
        QStringLiteral("initial_total_upper"), QStringLiteral("page_info") };

    // Read fields for header section
    for (const QString& field : header_fields) {
        ReadFieldPosition(settings, QStringLiteral("header"), field);
    }

    // Read fields for table section
    for (const QString& field : table_fields) {
        ReadFieldPosition(settings, QStringLiteral("table"), field);
    }

    {
        settings.beginGroup(QStringLiteral("table"));
        row_height_ = settings.value(QStringLiteral("row_height")).toInt();

        const auto col_string { settings.value(QStringLiteral("column_widths")).toStringList() };
        column_widths_.reserve(col_string.size());

        for (const auto& s : col_string) {
            column_widths_.push_back(s.toInt());
        }
        settings.endGroup();
    }

    // Read fields for footer section
    for (const QString& field : footer_fields) {
        ReadFieldPosition(settings, QStringLiteral("footer"), field);
    }

    current_template_ = template_name;
    return true;
}

void PrintHub::RenderAllPages(QPrinter* printer)
{
    // Fetch configuration values for rows and columns
    const int rows { GetFieldX(QStringLiteral("rows_columns"), 7) };

    // Calculate total pages required based on the total rows and rows per page
    const long long total_pages { (entry_list_.size() + rows - 1) / rows }; // Ceiling division to determine total pages

    QPainter painter(printer);

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
        const long long end_index { qMin((page_num + 1) * rows, entry_list_.size()) };
        DrawTable(&painter, start_index, end_index);

        // Draw footer (e.g., page number, etc.)
        DrawFooter(&painter, page_num + 1, total_pages);
    }
}

void PrintHub::DrawHeader(QPainter* painter)
{
    // Example: Draw a header at the specified position

    DrawText(painter, QStringLiteral("partner"), partner_->Name(node_o_->partner));
    DrawText(painter, QStringLiteral("issued_time"), node_o_->issued_time.toLocalTime().toString(kDateTimeFST));
}

void PrintHub::DrawTable(QPainter* painter, long long start_index, long long end_index)
{
    const int columns { GetFieldY(QStringLiteral("rows_columns"), 0) };
    const int left { GetFieldX(QStringLiteral("left_top"), 0) };
    const int top { GetFieldY(QStringLiteral("left_top"), 0) };

    for (int row = 0; row != end_index - start_index; ++row) {
        const auto* entry { entry_list_.at(start_index + row) };

        int x { left };
        for (int col = 0; col != columns; ++col) {
            const int col_width { column_widths_.at(col) };

            if (col_width == 0) {
                continue;
            }

            const QRect cellRect(x, top + row * row_height_, col_width, row_height_);

            painter->drawText(cellRect, Qt::AlignCenter, GetColumnText(col, entry));
            x += col_width;
        }
    }
}

void PrintHub::DrawFooter(QPainter* painter, int page_num, int total_pages)
{
    DrawText(painter, QStringLiteral("employee"), partner_->Name(node_o_->employee));

    // unit
    {
        QString unit {};
        switch (UnitO(node_o_->unit)) {
        case UnitO::kMonthly:
            unit = QObject::tr("MS");
            break;
        case UnitO::kImmediate:
            unit = QObject::tr("IS");
            break;
        case UnitO::kPending:
            unit = QObject::tr("PEND");
            break;
        default:
            break;
        }

        DrawText(painter, QStringLiteral("unit"), unit);
    }

    const double rounded_value { QString::number(node_o_->initial_total, 'f', section_config_->amount_decimal).toDouble() };

    DrawText(painter, QStringLiteral("initial_total"), QString::number(rounded_value));
    DrawText(painter, QStringLiteral("initial_total_upper"), NumberToChineseUpper(rounded_value));
    DrawText(painter, QStringLiteral("page_info"), QString::asprintf("%d/%d", page_num, total_pages));
}

QString PrintHub::GetColumnText(int col, const Entry* entry)
{
    auto* d_entry { static_cast<const EntryO*>(entry) };
    assert(section_config_);

    switch (col) {
    case 0:
        return inventory_->Path(entry->rhs_node);
    case 1:
        return inventory_->Name(d_entry->external_sku);
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
        return QString();
    }
}

QString PrintHub::NumberToChineseUpper(double value)
{
    if (value < 0) {
        return "负" + NumberToChineseUpper(-value);
    }

    if (value >= 1e15) {
        return "金额过大";
    }

    static const QStringList digits = { "零", "壹", "贰", "叁", "肆", "伍", "陆", "柒", "捌", "玖" };
    static const QStringList units = { "", "拾", "佰", "仟" };
    static const QStringList big_units = { "", "万", "亿", "兆" };

    qint64 integer { static_cast<qint64>(value) };
    const int fraction { qRound((value - integer) * 100) };

    QString result {};
    result.reserve(64);

    if (integer == 0) {
        result = "零元";
    } else {
        QString temp {};
        temp.reserve(48);

        int section_idx { 0 };
        bool has_prev_section { false };

        while (integer > 0) {
            const int section { static_cast<int>(integer % 10000) };
            integer /= 10000;

            if (section > 0) {
                QString section_str { ConvertSection(section, digits, units) };
                section_str += big_units[section_idx];

                if (has_prev_section && !temp.isEmpty()) {
                    temp = section_str + temp;
                } else {
                    temp = section_str + temp;
                }
                has_prev_section = true;
            } else if (has_prev_section) {
                temp = "零" + temp;
            }

            section_idx++;
        }

        static const QRegularExpression multi_zero("零{2,}");
        static const QRegularExpression zero_before_unit("零([万亿兆])");
        static const QRegularExpression trailing_zero("零+$");

        temp.replace(multi_zero, "零");
        temp.replace(zero_before_unit, "\\1");
        temp.remove(trailing_zero);

        result = temp + "元";
    }

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
            result += "零" + digits[fen] + "分";
        }
    }

    return result;
}
QString PrintHub::ConvertSection(int section, const QStringList& digits, const QStringList& units)
{
    if (section == 0)
        return QString();

    static constexpr int divisors[] = { 1000, 100, 10, 1 };

    QString result {};
    result.reserve(8);
    bool need_zero { false };

    for (int i = 0; i != 4; ++i) {
        const int digit { section / divisors[i] };
        section %= divisors[i];

        if (digit > 0) {
            if (need_zero) {
                result += "零";
            }
            result += digits[digit] + units[3 - i];
            need_zero = false;
        } else if (!result.isEmpty()) {
            need_zero = true;
        }
    }

    return result;
}

void PrintHub::DrawText(QPainter* painter, const QString& field, const QString& text)
{
    const auto& opt_pos { field_position_.value(field) };
    if (!opt_pos.has_value())
        return;

    const FieldPosition& pos { *opt_pos };
    painter->drawText(pos.x, pos.y, text);
}

void PrintHub::ApplyConfig(QPrinter* printer)
{
    QPageLayout layout { printer->pageLayout() };

    const QString orientation { page_values_.value(QStringLiteral("orientation")).toString().toLower() };
    const QString page_size { page_values_.value(QStringLiteral("page_size")).toString().toLower() };

    layout.setOrientation(orientation == QStringLiteral("landscape") ? QPageLayout::Landscape : QPageLayout::Portrait);
    layout.setPageSize(QPageSize(page_size == QStringLiteral("a4") ? QPageSize::A4 : QPageSize::A5));

    printer->setPageLayout(layout);
}

void PrintHub::ReadFieldPosition(QSettings& settings, const QString& group, const QString& field)
{
    settings.beginGroup(group);

    // Check if the key exists in settings
    if (!settings.contains(field)) {
        qWarning() << "Position setting not found, group:" << group << "field:" << field;
        field_position_[field] = std::nullopt;
        settings.endGroup();
        return;
    }

    // Read the position value from the settings
    const auto position { settings.value(field).value<QVariantList>() };
    if (position.size() != 2) {
        qWarning() << "Non valid position value for field_group:" << group << "field:" << field;
        field_position_[field] = std::nullopt;
        settings.endGroup();
        return;
    }

    // Try to convert the position parts to integers
    bool x_ok {};
    bool y_ok {};
    const int x { position[0].toInt(&x_ok) };
    const int y { position[1].toInt(&y_ok) };

    // Only store if both x and y are valid integers
    if (x_ok && y_ok) {
        field_position_[field] = FieldPosition(x, y);
    } else {
        qWarning() << "Invalid position coordinates, group:" << group << "field:" << field << "value:" << position;
        field_position_[field] = std::nullopt;
    }

    settings.endGroup();
}
