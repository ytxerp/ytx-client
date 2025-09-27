#include "printmanager.h"

#include <QFile>
#include <QFont>
#include <QPainter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinterInfo>
#include <QVariant>

PrintManager::PrintManager(CLocalConfig& local_config, TreeModel* item, TreeModel* partner)
    : local_config_ { local_config }
    , item_ { item }
    , partner_ { partner }
{
}

void PrintManager::SetData(const PrintData& data, const QList<EntryShadow*>& entry_shadow_list)
{
    data_ = data;
    entry_shadow_list_ = entry_shadow_list;
}

void PrintManager::Preview()
{
    QPrinter printer { QPrinter::ScreenResolution };
    ApplyConfig(&printer);

    printer.setPrinterName(local_config_.printer);

    QPrintPreviewDialog preview(&printer);

    QObject::connect(&preview, &QPrintPreviewDialog::paintRequested, &preview, [this](QPrinter* printer) { this->RenderAllPages(printer); });
    preview.exec();
}

void PrintManager::Print()
{
    QPrinter printer(QPrinter::ScreenResolution);
    ApplyConfig(&printer);

    const auto available_printers { QPrinterInfo::availablePrinterNames() };
    const QString& printer_name { local_config_.printer };

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

    const QList<QString> header_settings { "party", "issued_time" };
    const QList<QString> content_settings { "left_top", "heigh_width", "rows_columns" };
    const QList<QString> footer_settings { "employee", "unit", "initial_total", "page_info" };

    // Read fields for header section
    for (const QString& setting : header_settings) {
        ReadFieldPosition(settings, "header", setting);
    }

    // Read fields for content section
    for (const QString& setting : content_settings) {
        ReadFieldPosition(settings, "table", setting);
    }

    // Read fields for footer section
    for (const QString& setting : footer_settings) {
        ReadFieldPosition(settings, "footer", setting);
    }

    return true;
}

void PrintManager::RenderAllPages(QPrinter* printer)
{
    // Fetch configuration values for rows and columns
    int rows { field_settings_.value("rows_columns").x };

    // Calculate total pages required based on the total rows and rows per page
    const long long total_pages { (entry_shadow_list_.size() + rows - 1) / rows }; // Ceiling division to determine total pages

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
        const long long start_index = page_num * rows;
        const long long end_index = qMin((page_num + 1) * rows, entry_shadow_list_.size());
        DrawTable(&painter, start_index, end_index);

        // Draw footer (e.g., page number, etc.)
        DrawFooter(&painter, page_num + 1, total_pages);
    }
}

void PrintManager::DrawHeader(QPainter* painter)
{
    // Example: Draw a header at the specified position

    if (field_settings_.contains("party")) {
        const auto& party { field_settings_.value("party") };
        painter->drawText(party.x, party.y, data_.party);
    }

    const auto& issued_time { field_settings_.value("issued_time") };
    painter->drawText(issued_time.x, issued_time.y, data_.issued_time);
}

void PrintManager::DrawTable(QPainter* painter, long long start_index, long long end_index)
{
    int columns { field_settings_.value("rows_columns").y };
    int left { field_settings_.value("left_top").x };
    int top { field_settings_.value("left_top").y };
    int heigh { field_settings_.value("heigh_width").x };
    int width { field_settings_.value("heigh_width").y };

    for (int row = 0; row != end_index - start_index; ++row) {
        const auto* entry_shadow { entry_shadow_list_.at(start_index + row) };

        for (int col = 0; col != columns; ++col) {
            QRect cellRect(left + col * width, top + row * heigh, width, heigh);
            // painter->drawRect(cellRect);
            painter->drawText(cellRect, Qt::AlignCenter, GetColumnText(col, entry_shadow));
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

    const auto& page_info { field_settings_.value("page_info") };
    painter->drawText(page_info.x, page_info.y, QString::asprintf("%d/%d", page_num, total_pages));
}

QString PrintManager::GetColumnText(int col, const EntryShadow* entry_shadow)
{
    auto* o_entry_shadow { static_cast<const EntryShadowO*>(entry_shadow) };

    switch (col) {
    case 0:
        return item_->Path(*entry_shadow->rhs_node);
    case 1:
        return item_->Path(*o_entry_shadow->external_item);
    case 2:
        return *entry_shadow->description;
    case 3:
        return QString::number(*o_entry_shadow->count);
    case 4:
        return QString::number(*o_entry_shadow->measure);
    case 5:
        return QString::number(*o_entry_shadow->unit_price);
    case 6:
        return QString::number(*o_entry_shadow->initial);
    default:
        return QString();
    }
}

void PrintManager::ApplyConfig(QPrinter* printer)
{
    QPageLayout layout = printer->pageLayout();

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
