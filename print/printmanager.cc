#include "printmanager.h"

#include <QFile>
#include <QFont>
#include <QPainter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinterInfo>
#include <QVariant>

PrintManager::PrintManager(CAppConfig& app_config, TreeModel* inventory, TreeModel* partner)
    : app_config_ { app_config }
    , inventory_ { inventory }
    , partner_ { partner }
{
}

void PrintManager::SetData(const PrintData& data, const QList<Entry*>& entry_list)
{
    data_ = data;
    entry_list_ = entry_list;
}

void PrintManager::Preview()
{
    QPrinter printer { QPrinter::ScreenResolution };
    ApplyConfig(&printer);

    printer.setPrinterName(app_config_.printer);

    QPrintPreviewDialog preview(&printer);

    QObject::connect(&preview, &QPrintPreviewDialog::paintRequested, &preview, [this](QPrinter* printer) { this->RenderAllPages(printer); });
    preview.exec();
}

void PrintManager::Print()
{
    QPrinter printer(QPrinter::ScreenResolution);
    ApplyConfig(&printer);

    const auto available_printers { QPrinterInfo::availablePrinterNames() };
    const QString& printer_name { app_config_.printer };

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

bool PrintManager::LoadIni(const QString& file_path)
{
    // Page settings
    QSettings settings(file_path, QSettings::IniFormat);

    const QList<QString> page_settings { "paper_size", "orientation", "font_size" };

    for (const QString& setting : page_settings) {
        page_settings_[setting] = settings.value("page_settings/" + setting);
    }

    const QList<QString> header_settings { "partner", "issued_time" };
    const QList<QString> content_settings { "left_top", "rows_columns" };
    const QList<QString> footer_settings { "employee", "unit", "initial_total", "initial_total_upper", "page_info" };

    // Read fields for header section
    for (const QString& setting : header_settings) {
        ReadFieldPosition(settings, "header", setting);
    }

    // Read fields for content section
    for (const QString& setting : content_settings) {
        ReadFieldPosition(settings, "table", setting);
    }

    settings.beginGroup("table");
    row_height_ = settings.value("row_height").toInt();

    const auto col_string { settings.value("column_widths").toStringList() };
    column_widths_.reserve(col_string.size());

    for (const auto& s : col_string) {
        column_widths_.push_back(s.toInt());
    }
    settings.endGroup();

    // Read fields for footer section
    for (const QString& setting : footer_settings) {
        ReadFieldPosition(settings, "footer", setting);
    }

    return true;
}

void PrintManager::RenderAllPages(QPrinter* printer)
{
    // Fetch configuration values for rows and columns
    const int rows { field_settings_.value("rows_columns").x };

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

void PrintManager::DrawHeader(QPainter* painter)
{
    // Example: Draw a header at the specified position

    if (field_settings_.contains("partner")) {
        const auto& partner { field_settings_.value("partner") };
        painter->drawText(partner.x, partner.y, data_.partner);
    }

    const auto& issued_time { field_settings_.value("issued_time") };
    painter->drawText(issued_time.x, issued_time.y, data_.issued_time);
}

void PrintManager::DrawTable(QPainter* painter, long long start_index, long long end_index)
{
    int columns { field_settings_.value("rows_columns").y };
    int left { field_settings_.value("left_top").x };
    int top { field_settings_.value("left_top").y };

    for (int row = 0; row != end_index - start_index; ++row) {
        const auto* entry { entry_list_.at(start_index + row) };

        int x = left;
        for (int col = 0; col != columns; ++col) {
            const int col_width { column_widths_.at(col) };

            QRect cellRect(x, top + row * row_height_, col_width, row_height_);

            painter->drawText(cellRect, Qt::AlignCenter, GetColumnText(col, entry));
            x += col_width;
        }
    }
}

void PrintManager::DrawFooter(QPainter* painter, int page_num, int total_pages)
{
    // Example: Draw footer with page number at the bottom of the page

    if (field_settings_.contains("employee")) {
        const auto& employee { field_settings_.value("employee") };
        painter->drawText(employee.x, employee.y, data_.employee);
    }

    const auto& unit { field_settings_.value("unit") };
    painter->drawText(unit.x, unit.y, data_.unit);

    const auto& initial_total { field_settings_.value("initial_total") };
    painter->drawText(initial_total.x, initial_total.y, QString::number(data_.initial_total));

    const auto& initial_total_upper { field_settings_.value("initial_total_upper") };
    painter->drawText(initial_total_upper.x, initial_total_upper.y, NumberToChineseUpper(data_.initial_total));

    const auto& page_info { field_settings_.value("page_info") };
    painter->drawText(page_info.x, page_info.y, QString::asprintf("%d/%d", page_num, total_pages));
}

QString PrintManager::GetColumnText(int col, const Entry* entry)
{
    auto* d_entry { static_cast<const EntryO*>(entry) };

    switch (col) {
    case 0:
        return inventory_->Path(entry->rhs_node);
    case 1:
        return inventory_->Name(d_entry->external_sku);
    case 2:
        return entry->description;
    case 3:
        return QString::number(d_entry->count);
    case 4:
        return QString::number(d_entry->measure);
    case 5:
        return QString::number(d_entry->unit_price);
    case 6:
        return QString::number(d_entry->initial);
    default:
        return QString();
    }
}

QString PrintManager::NumberToChineseUpper(double value)
{
    if (value < 0) {
        return QObject::tr("-") + NumberToChineseUpper(-value);
    }

    if (value >= 1e15) {
        return QObject::tr("Amount Too Large");
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
QString PrintManager::ConvertSection(int section, const QStringList& digits, const QStringList& units)
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

void PrintManager::ApplyConfig(QPrinter* printer)
{
    QPageLayout layout { printer->pageLayout() };

    const QString orientation { page_settings_.value("orientation").toString().toLower() };
    const QString paper_size { page_settings_.value("paper_size").toString().toLower() };

    layout.setOrientation(orientation == "landscape" ? QPageLayout::Landscape : QPageLayout::Portrait);
    layout.setPageSize(QPageSize(paper_size == "a4" ? QPageSize::A4 : QPageSize::A5));

    printer->setPageLayout(layout);
}

void PrintManager::ReadFieldPosition(QSettings& settings, const QString& section, const QString& prefix)
{
    settings.beginGroup(section);

    // Check if the key exists in settings
    if (!settings.contains(prefix)) {
        qWarning() << "Position setting not found for section:" << section << "field:" << prefix;
        return;
    }

    // Read the position value from the settings
    const auto position { settings.value(prefix).value<QVariantList>() };
    if (position.size() != 2) {
        qWarning() << "Non valid position value for section:" << section << "field:" << prefix;
        return;
    }

    // Try to convert the position parts to integers
    bool x_ok {};
    bool y_ok {};
    const int x { position[0].toInt(&x_ok) };
    const int y { position[1].toInt(&y_ok) };

    // Only store if both x and y are valid integers
    if (x_ok && y_ok) {
        field_settings_[prefix] = FieldSettings(x, y);
    } else {
        qWarning() << "Invalid position coordinates for section:" << section << "field:" << prefix << "value:" << position;
    }

    settings.endGroup();
}
