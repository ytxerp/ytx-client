#include "printhub.h"

#include <QtCore/qdir.h>
#include <QtGui/qpainter.h>
#include <QtPrintSupport/qprintdialog.h>
#include <QtPrintSupport/qprinterinfo.h>
#include <QtPrintSupport/qprintpreviewdialog.h>
#include <QtWidgets/qapplication.h>

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

bool PrintHub::IsNumber(const QString& text)
{
    // Match: optional minus, digits with optional thousand separators, optional decimal part
    // Examples: "123", "-456", "1,234", "1,234.56", "-1,234.56"
    static const QRegularExpression re(R"(^-?(\d{1,3}(,\d{3})*|\d+)(\.\d+)?$)");
    return re.match(text).hasMatch();
}

QString PrintHub::NumberToChineseUpper(double value)
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

int PrintHub::FindBestFontSize(const QFont& base_font, const QPaintDevice* device, const QString& text, int max_width, int max_font, int min_font)
{
    int low { min_font };
    int high { max_font };
    int best { min_font };

    QFont font { base_font };

    while (low <= high) {
        const int mid { low + (high - low) / 2 };

        font.setPointSize(mid);

        const QFontMetricsF fm { font, device };

        if (fm.horizontalAdvance(text) <= max_width) {
            best = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return best;
}

QString PrintHub::ConvertSection(int section, const QStringList& digits)
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

void PrintHub::RenderAllPages(QPrinter* printer)
{
    switch (page_config_.print_mode) {
    case PrintMode::kFull:
        full_.Render(printer, node_o_, entry_list_, section_config_);
        break;

    case PrintMode::kOverlay:
        overlay_.Render(printer, node_o_, entry_list_, section_config_);
        break;
    }
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

    switch (page_config_.print_mode) {
    case PrintMode::kFull:
        return full_.LoadTemplate(settings);

    case PrintMode::kOverlay:
        return overlay_.LoadTemplate(settings);
    }

    return true;
}

void PrintHub::ReadPageConfig(QSettings& settings)
{
    settings.beginGroup(QStringLiteral("page"));

    page_config_.page_size = settings.value(QStringLiteral("page_size"), QStringLiteral("A5")).toString();

    page_config_.orientation = settings.value(QStringLiteral("orientation"), QStringLiteral("landscape")).toString();

    const QString mode { settings.value(QStringLiteral("mode"), QStringLiteral("full")).toString() };

    page_config_.print_mode = mode.compare(QStringLiteral("overlay"), Qt::CaseInsensitive) == 0 ? PrintMode::kOverlay : PrintMode::kFull;

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
