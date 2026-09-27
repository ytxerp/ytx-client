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
    constexpr auto folder_name { "print_template" };
#endif

    const QString folder_path { QCoreApplication::applicationDirPath() + QDir::separator() + QString::fromUtf8(folder_name) };
    QDir dir(folder_path);

    if (!dir.exists()) {
        return;
    }

    const QStringList name_filters { "*.ini" };
    const QDir::Filters entry_filters { QDir::Files | QDir::NoSymLinks };
    const QFileInfoList file_list { dir.entryInfoList(name_filters, entry_filters) };

    for (const auto& file_info : file_list) {
        template_map_.insert(file_info.baseName(), file_info.absoluteFilePath());
    }
}

bool PrintHub::LoadTemplate(const QString& template_name)
{
    if (template_name.isEmpty())
        return false;

    QSettings settings(template_name, QSettings::IniFormat);

    if (settings.status() != QSettings::NoError) {
        qWarning() << "Failed to load print template:" << template_name;
        return false;
    }

    ReadPageConfig(settings);

    if (!full_.LoadTemplate(settings))
        return false;

    return true;
}

void PrintHub::ReadPageConfig(QSettings& settings)
{
    settings.beginGroup(QStringLiteral("page"));

    page_config_.page_size = settings.value(QStringLiteral("page_size"), QStringLiteral("A5")).toString();

    page_config_.orientation = settings.value(QStringLiteral("orientation"), QStringLiteral("landscape")).toString();

    settings.endGroup();
}

void PrintHub::ApplyConfig(QPrinter* printer)
{
    QPageLayout layout { printer->pageLayout() };

    layout.setOrientation(
        page_config_.orientation.compare(QStringLiteral("landscape"), Qt::CaseInsensitive) == 0 ? QPageLayout::Landscape : QPageLayout::Portrait);

    layout.setPageSize(QPageSize(page_config_.page_size.compare(QStringLiteral("A4"), Qt::CaseInsensitive) == 0 ? QPageSize::A4 : QPageSize::A5));

    printer->setPageLayout(layout);
}
