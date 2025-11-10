#include "printmanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QPainter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinterInfo>
#include <QVariant>

void PrintManager::SetValue(const NodeO* node_o, const QList<Entry*>& entry_list)
{
    node_o_ = node_o;
    entry_list_ = entry_list;
}

void PrintManager::Preview()
{
    QPrinter printer { QPrinter::ScreenResolution };
    ApplyConfig(&printer);

    printer.setPrinterName(app_config_->printer);

    QPrintPreviewDialog preview(&printer);

    QObject::connect(&preview, &QPrintPreviewDialog::paintRequested, &preview, [this](QPrinter* printer) { RenderAllPages(printer); });
    preview.exec();
}

void PrintManager::Print()
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

void PrintManager::ScanTemplate()
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

bool PrintManager::LoadTemplate(const QString& template_name)
{
    if (template_name == current_template_) {
        return true;
    }

    page_values_.clear();
    field_position_.clear();

    // Page settings
    QSettings settings(template_name, QSettings::IniFormat);

    static const QList<QString> page_fields { "page_size", "orientation", "font_size" };

    for (const QString& field : page_fields) {
        page_values_[field] = settings.value("page/" + field);
    }

    static const QList<QString> header_fields { "partner", "issued_time" };
    static const QList<QString> content_fields { "left_top", "rows_columns" };
    static const QList<QString> footer_fields { "employee", "unit", "initial_total", "initial_total_upper", "page_info" };

    // Read fields for header section
    for (const QString& field : header_fields) {
        ReadFieldPosition(settings, "header", field);
    }

    // Read fields for content section
    for (const QString& field : content_fields) {
        ReadFieldPosition(settings, "table", field);
    }

    {
        settings.beginGroup("table");
        row_height_ = settings.value("row_height").toInt();

        const auto col_string { settings.value("column_widths").toStringList() };
        column_widths_.reserve(col_string.size());

        for (const auto& s : col_string) {
            column_widths_.push_back(s.toInt());
        }
        settings.endGroup();
    }

    // Read fields for footer section
    for (const QString& field : footer_fields) {
        ReadFieldPosition(settings, "footer", field);
    }

    current_template_ = template_name;
    return true;
}

void PrintManager::RenderAllPages(QPrinter* printer)
{
    // Fetch configuration values for rows and columns
    const int rows { field_position_.value("rows_columns").x };

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

    {
        const auto partner { field_position_.value("partner") };

        const int x { partner.x };
        const int y { partner.y };

        if (x != 0 || y != 0) {
            painter->drawText(x, y, partner_->Name(node_o_->partner));
        }
    }

    const auto issued_time { field_position_.value("issued_time") };
    painter->drawText(issued_time.x, issued_time.y, node_o_->issued_time.toLocalTime().toString(kDateTimeFST));
}

void PrintManager::DrawTable(QPainter* painter, long long start_index, long long end_index)
{
    int columns { field_position_.value("rows_columns").y };
    int left { field_position_.value("left_top").x };
    int top { field_position_.value("left_top").y };

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

void PrintManager::DrawFooter(QPainter* painter, int page_num, int total_pages)
{
    // employee
    {
        const auto employee { field_position_.value("employee") };
        painter->drawText(employee.x, employee.y, partner_->Name(node_o_->employee));
    }

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

        const auto unit_poition { field_position_.value("unit") };
        painter->drawText(unit_poition.x, unit_poition.y, unit);
    }

    // initial_total
    {
        const auto p { field_position_.value("initial_total") };
        const int x { p.x };
        const int y { p.y };
        if (x != 0 || y != 0) {
            painter->drawText(x, y, QString::number(node_o_->initial_total));
        }
    }

    // initial_total_upper
    {
        const auto p { field_position_.value("initial_total_upper") };
        const int x { p.x };
        const int y { p.y };
        if (x != 0 || y != 0) {
            painter->drawText(x, y, NumberToChineseUpper(node_o_->initial_total));
        }
    }

    {
        const auto page_info { field_position_.value("page_info") };
        painter->drawText(page_info.x, page_info.y, QString::asprintf("%d/%d", page_num, total_pages));
    }
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

    const QString orientation { page_values_.value("orientation").toString().toLower() };
    const QString page_size { page_values_.value("page_size").toString().toLower() };

    layout.setOrientation(orientation == "landscape" ? QPageLayout::Landscape : QPageLayout::Portrait);
    layout.setPageSize(QPageSize(page_size == "a4" ? QPageSize::A4 : QPageSize::A5));

    printer->setPageLayout(layout);
}

void PrintManager::ReadFieldPosition(QSettings& settings, const QString& group, const QString& field)
{
    settings.beginGroup(group);

    // Check if the key exists in settings
    if (!settings.contains(field)) {
        qWarning() << "Position setting not found, group:" << group << "field:" << field;
        return;
    }

    // Read the position value from the settings
    const auto position { settings.value(field).value<QVariantList>() };
    if (position.size() != 2) {
        qWarning() << "Non valid position value for field_group:" << group << "field:" << field;
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
    }

    settings.endGroup();
}
