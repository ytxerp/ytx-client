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

#include "component/constantstring.h"
#include "utils/nodeutils.h"

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

    if (template_name == current_template_)
        return true;

    QSettings settings(template_name, QSettings::IniFormat);

    if (settings.status() != QSettings::NoError) {
        qWarning() << "Failed to load print template:" << template_name;
        return false;
    }

    ReadPageConfig(settings);
    ReadCompanyConfig(settings);
    ReadHeaderConfig(settings);
    ReadTableConfig(settings);
    ReadTotalConfig(settings);
    ReadRemarkConfig(settings);
    ReadFooterConfig(settings);

    if (table_config_.columns.size() != table_config_.column_widths.size()) {
        qWarning() << "Print template column count does not match column width count:" << table_config_.columns.size() << table_config_.column_widths.size();
        return false;
    }

    current_template_ = template_name;
    return true;
}

void PrintHub::ReadPageConfig(QSettings& settings)
{
    settings.beginGroup(QStringLiteral("page"));

    page_config_.page_size = settings.value(QStringLiteral("page_size"), QStringLiteral("A5")).toString();

    page_config_.orientation = settings.value(QStringLiteral("orientation"), QStringLiteral("landscape")).toString();

    page_config_.font_size = settings.value(QStringLiteral("font_size"), 12).toInt();

    page_config_.margin_left = settings.value(QStringLiteral("margin_left"), 20).toInt();

    page_config_.margin_right = settings.value(QStringLiteral("margin_right"), 20).toInt();

    page_config_.margin_top = settings.value(QStringLiteral("margin_top"), 15).toInt();

    page_config_.margin_bottom = settings.value(QStringLiteral("margin_bottom"), 15).toInt();

    settings.endGroup();
}

void PrintHub::ReadCompanyConfig(QSettings& settings)
{
    settings.beginGroup(QStringLiteral("company"));

    company_config_.show_logo = settings.value(QStringLiteral("show_logo"), true).toBool();

    company_config_.logo = settings.value(QStringLiteral("logo")).toString();

    company_config_.show_name = settings.value(QStringLiteral("show_name"), true).toBool();

    company_config_.name = settings.value(QStringLiteral("name")).toString();

    company_config_.show_address = settings.value(QStringLiteral("show_address"), true).toBool();

    company_config_.address = settings.value(QStringLiteral("address")).toString();

    company_config_.show_phone = settings.value(QStringLiteral("show_phone"), true).toBool();

    company_config_.phone = settings.value(QStringLiteral("phone")).toString();

    settings.endGroup();
}

void PrintHub::ReadHeaderConfig(QSettings& settings)
{
    settings.beginGroup(QStringLiteral("header"));

    header_config_.show_title = settings.value(QStringLiteral("show_title"), true).toBool();

    header_config_.title = settings.value(QStringLiteral("title")).toString();

    header_config_.show_partner = settings.value(QStringLiteral("show_partner"), true).toBool();

    header_config_.show_code = settings.value(QStringLiteral("show_code"), true).toBool();

    header_config_.show_issued_time = settings.value(QStringLiteral("show_issued_time"), true).toBool();

    header_config_.show_settlement = settings.value(QStringLiteral("show_settlement"), true).toBool();

    settings.endGroup();
}

void PrintHub::ReadTableConfig(QSettings& settings)
{
    settings.beginGroup(QStringLiteral("table"));

    table_config_.show_border = settings.value(QStringLiteral("show_border"), true).toBool();

    table_config_.show_header = settings.value(QStringLiteral("show_header"), true).toBool();

    table_config_.row_height = settings.value(QStringLiteral("row_height"), 30).toInt();

    table_config_.columns = settings.value(QStringLiteral("columns")).toStringList();

    const auto widths = settings.value(QStringLiteral("column_widths")).toStringList();

    table_config_.column_widths.clear();
    table_config_.column_widths.reserve(widths.size());

    std::ranges::transform(widths, std::back_inserter(table_config_.column_widths), [](const QString& value) { return value.toInt(); });

    settings.endGroup();

    settings.beginGroup(QStringLiteral("column_titles"));

    table_config_.column_titles.clear();

    for (const auto& column : std::as_const(table_config_.columns)) {
        table_config_.column_titles.insert(column, settings.value(column, column).toString());
    }

    settings.endGroup();
}

void PrintHub::ReadTotalConfig(QSettings& settings)
{
    settings.beginGroup(QStringLiteral("total"));

    total_config_.enabled = settings.value(QStringLiteral("enabled"), true).toBool();

    total_config_.show_title = settings.value(QStringLiteral("show_title"), true).toBool();

    total_config_.title = settings.value(QStringLiteral("title")).toString();

    total_config_.show_upper = settings.value(QStringLiteral("show_upper"), true).toBool();

    total_config_.show_amount = settings.value(QStringLiteral("show_amount"), true).toBool();

    settings.endGroup();
}

void PrintHub::ReadRemarkConfig(QSettings& settings)
{
    settings.beginGroup(QStringLiteral("remark"));

    remark_config_.enabled = settings.value(QStringLiteral("enabled"), true).toBool();

    remark_config_.show_title = settings.value(QStringLiteral("show_title"), true).toBool();

    remark_config_.title = settings.value(QStringLiteral("title")).toString();

    remark_config_.text = settings.value(QStringLiteral("text")).toString();

    remark_config_.padding = settings.value(QStringLiteral("padding"), 6).toInt();

    remark_config_.show_border = settings.value(QStringLiteral("show_border"), true).toBool();

    settings.endGroup();
}

void PrintHub::ReadFooterConfig(QSettings& settings)
{
    settings.beginGroup(QStringLiteral("footer"));

    footer_config_.show_employee = settings.value(QStringLiteral("show_employee"), true).toBool();

    footer_config_.employee_title = settings.value(QStringLiteral("employee_title")).toString();

    footer_config_.show_page_info = settings.value(QStringLiteral("show_page_info"), true).toBool();

    settings.endGroup();
}

qreal PrintHub::DrawCompany(QPainter* painter, qreal y, qreal page_width)
{
    painter->save();

    const qreal left { static_cast<qreal>(page_config_.margin_left) };
    const qreal width { page_width - page_config_.margin_left - page_config_.margin_right };

    const QFont original_font { painter->font() };
    const qreal line_height { QFontMetricsF(original_font).height() };

    if (company_config_.show_logo && !company_config_.logo.trimmed().isEmpty()) {
        // Logo drawing can be added here.
        // y += logoHeight + spacing;
    }

    if (company_config_.show_name && !company_config_.name.trimmed().isEmpty()) {
        QFont font { original_font };
        font.setBold(true);
        font.setPointSize(original_font.pointSize() + 2);
        painter->setFont(font);

        const QFontMetricsF fm(font);
        const qreal height { fm.height() };

        painter->drawText(QRectF(left, y, width, height), Qt::AlignCenter, company_config_.name);

        y += height + 2;
        painter->setFont(original_font);
    }

    if (company_config_.show_address && !company_config_.address.trimmed().isEmpty()) {
        painter->drawText(QRectF(left, y, width, line_height), Qt::AlignCenter, company_config_.address);

        y += line_height;
    }

    if (company_config_.show_phone && !company_config_.phone.trimmed().isEmpty()) {
        painter->drawText(QRectF(left, y, width, line_height), Qt::AlignCenter, company_config_.phone);

        y += line_height;
    }

    painter->restore();

    return y;
}

qreal PrintHub::DrawHeader(QPainter* painter, qreal y, qreal page_width)
{
    painter->save();

    const qreal left { static_cast<qreal>(page_config_.margin_left) };
    const qreal width { page_width - page_config_.margin_left - page_config_.margin_right };

    const qreal line_height { QFontMetricsF(painter->font()).height() + 4 };
    const qreal column_width { width / 3.0 };

    qreal left_y { y };
    qreal right_y { y };
    qreal title_bottom { y };

    if (header_config_.show_partner) {
        const QString text { QObject::tr("Customer: ") + MasterDataRegistry::Instance().PartnerName(node_o_->partner_id) };
        painter->drawText(QRectF(left, left_y, column_width, line_height), Qt::AlignLeft | Qt::AlignVCenter, text);
        left_y += line_height;
    }

    if (header_config_.show_title && !header_config_.title.trimmed().isEmpty()) {
        QFont font { painter->font() };
        font.setBold(true);
        font.setPointSize(font.pointSize() + 2);

        painter->save();
        painter->setFont(font);

        const qreal title_height { line_height * 2 };
        painter->drawText(QRectF(left + column_width, y, column_width, title_height), Qt::AlignCenter, header_config_.title);

        painter->restore();

        title_bottom = y + title_height;
    }

    if (header_config_.show_code) {
        painter->drawText(
            QRectF(left + column_width * 2, right_y, column_width, line_height), Qt::AlignLeft | Qt::AlignVCenter, QObject::tr("Code: ") + node_o_->code);
        right_y += line_height;
    }

    if (header_config_.show_issued_time) {
        painter->drawText(QRectF(left + column_width * 2, right_y, column_width, line_height), Qt::AlignLeft | Qt::AlignVCenter,
            QObject::tr("Date: ") + node_o_->issued_time.toString(datetime_format::kDashedDate));
        right_y += line_height;
    }

    if (header_config_.show_settlement) {
        const QString settlement { node::UnitString(NodeUnit(node_o_->unit)) };
        painter->drawText(
            QRectF(left + column_width * 2, right_y, column_width, line_height), Qt::AlignLeft | Qt::AlignVCenter, QObject::tr("Settlement: ") + settlement);
        right_y += line_height;
    }

    painter->restore();

    return std::max({ left_y, right_y, title_bottom }) + 6;
}

qreal PrintHub::DrawTable(QPainter* painter, qreal y, qreal page_width, qsizetype start_index, qsizetype end_index)
{
    painter->save();

    const qreal left { static_cast<qreal>(page_config_.margin_left) };
    const qreal available_width { page_width - page_config_.margin_left - page_config_.margin_right };

    const auto widths { CalculateColumnWidths(available_width) };

    if (widths.size() != table_config_.columns.size()) {
        painter->restore();
        return y;
    }

    const QFont original_font { painter->font() };
    constexpr qreal padding { 4.0 };

    auto draw_cell = [&](const QRectF& rect, const QString& text, Qt::Alignment alignment, bool bold = false) {
        painter->save();

        if (table_config_.show_border)
            painter->drawRect(rect);

        QFont font { original_font };
        font.setBold(bold);
        painter->setFont(font);

        painter->drawText(rect.adjusted(padding, 0, -padding, 0), static_cast<int>(alignment | Qt::AlignVCenter), text);

        painter->restore();
    };

    // Column header
    if (table_config_.show_header) {
        qreal x { left };

        for (qsizetype col = 0; col < table_config_.columns.size(); ++col) {
            const auto& column { table_config_.columns.at(col) };

            const QRectF rect { x, y, widths.at(col), static_cast<qreal>(table_config_.row_height) };

            draw_cell(rect, table_config_.column_titles.value(column, column), Qt::AlignCenter, true);

            x += widths.at(col);
        }

        y += table_config_.row_height;
    }

    const auto& master { MasterDataRegistry::Instance() };
    const auto& partner { PartnerInventoryRegistry::Instance() };

    for (qsizetype row = start_index; row < end_index; ++row) {
        const auto* entry { entry_list_.at(row) };

        qreal x { left };

        for (qsizetype col = 0; col < table_config_.columns.size(); ++col) {
            const auto& column { table_config_.columns.at(col) };

            const QString text { GetColumnText(column, entry, master, partner) };

            const QRectF rect { x, y, widths.at(col), static_cast<qreal>(table_config_.row_height) };

            Qt::Alignment alignment { Qt::AlignLeft };

            if (IsNumber(text))
                alignment = Qt::AlignRight;

            draw_cell(rect, text, alignment);

            x += widths.at(col);
        }

        y += table_config_.row_height;
    }

    painter->restore();

    return y;
}

qreal PrintHub::MeasureCompanyHeight(const QFont& base_font) const
{
    qreal height { 0.0 };
    const qreal line_height { QFontMetricsF(base_font).height() };

    if (company_config_.show_name && !company_config_.name.trimmed().isEmpty()) {
        QFont font { base_font };
        font.setBold(true);
        font.setPointSize(base_font.pointSize() + 2);
        height += QFontMetricsF(font).height() + 2;
    }

    if (company_config_.show_address && !company_config_.address.trimmed().isEmpty())
        height += line_height;

    if (company_config_.show_phone && !company_config_.phone.trimmed().isEmpty())
        height += line_height;

    return height;
}

qreal PrintHub::MeasureHeaderHeight(const QFont& base_font) const
{
    const qreal line_height { QFontMetricsF(base_font).height() + 4 };

    qreal left_h { 0.0 };
    qreal right_h { 0.0 };
    qreal title_h { 0.0 };

    if (header_config_.show_partner)
        left_h += line_height;

    if (header_config_.show_code)
        right_h += line_height;
    if (header_config_.show_issued_time)
        right_h += line_height;
    if (header_config_.show_settlement)
        right_h += line_height;

    if (header_config_.show_title && !header_config_.title.trimmed().isEmpty())
        title_h = line_height * 2;

    return std::max({ left_h, right_h, title_h }) + 6;
}

qreal PrintHub::MeasureRemarkHeight(const QFont& base_font, qreal page_width) const
{
    if (!remark_config_.enabled || remark_config_.text.trimmed().isEmpty())
        return 0.0;

    const qreal width { page_width - page_config_.margin_left - page_config_.margin_right };
    const qreal text_width { width - remark_config_.padding * 2 };

    QString text {};
    if (remark_config_.show_title && !remark_config_.title.trimmed().isEmpty()) {
        text += remark_config_.title;
        text += '\n';
    }
    text += remark_config_.text;

    const QFontMetricsF fm(base_font);
    const QRectF bounds { fm.boundingRect(QRectF(0, 0, text_width, 10000), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text) };

    return bounds.height() + remark_config_.padding * 2;
}

qreal PrintHub::MeasureFooterHeight(const QFont& base_font) const
{
    if (!footer_config_.show_employee && !footer_config_.show_page_info)
        return 0.0;

    return QFontMetricsF(base_font).height() + 6;
}

qreal PrintHub::DrawTotal(QPainter* painter, qreal y, qreal page_width)
{
    if (!total_config_.enabled)
        return y;

    painter->save();

    const qreal left { static_cast<qreal>(page_config_.margin_left) };
    const qreal width { page_width - page_config_.margin_left - page_config_.margin_right };

    const qreal height { static_cast<qreal>(table_config_.row_height) };

    const QString amount { QString::number(node_o_->initial_total, 'f', section_config_->amount_decimal) };

    QStringList parts {};

    if (total_config_.show_title && !total_config_.title.trimmed().isEmpty()) {
        parts.append(total_config_.title);
    }

    if (total_config_.show_upper) {
        parts.append(QStringLiteral("大写：") + NumberToChineseUpper(node_o_->initial_total));
    }

    if (total_config_.show_amount)
        parts.append(QStringLiteral("¥") + amount);

    if (!parts.isEmpty()) {
        const QRectF rect { left, y, width, height };

        if (table_config_.show_border)
            painter->drawRect(rect);

        QFont font { painter->font() };
        font.setBold(true);
        painter->setFont(font);

        painter->drawText(rect.adjusted(4, 0, -4, 0), Qt::AlignRight | Qt::AlignVCenter, parts.join(QStringLiteral("    ")));

        y += height;
    }

    painter->restore();

    return y;
}

qreal PrintHub::DrawRemark(QPainter* painter, qreal y, qreal page_width)
{
    if (!remark_config_.enabled || remark_config_.text.trimmed().isEmpty()) {
        return y;
    }

    painter->save();

    const qreal left { static_cast<qreal>(page_config_.margin_left) };
    const qreal width { page_width - page_config_.margin_left - page_config_.margin_right };

    QString text {};

    if (remark_config_.show_title && !remark_config_.title.trimmed().isEmpty()) {
        text += remark_config_.title;
        text += '\n';
    }

    text += remark_config_.text;

    const qreal text_width { width - remark_config_.padding * 2 };

    const QFontMetricsF fm { painter->font() };

    const QRectF text_bounds { fm.boundingRect(QRectF(0, 0, text_width, 10000), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text) };

    const qreal height { text_bounds.height() + remark_config_.padding * 2 };

    const QRectF rect { left, y, width, height };

    if (remark_config_.show_border)
        painter->drawRect(rect);

    painter->drawText(rect.adjusted(remark_config_.padding, remark_config_.padding, -remark_config_.padding, -remark_config_.padding),
        Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text);

    painter->restore();

    return y + height;
}

qreal PrintHub::DrawFooter(QPainter* painter, qreal y, qreal page_width, int page_num, int total_pages)
{
    painter->save();

    const qreal left { static_cast<qreal>(page_config_.margin_left) };
    const qreal width { page_width - page_config_.margin_left - page_config_.margin_right };

    const qreal line_height { QFontMetricsF(painter->font()).height() + 6 };

    if (footer_config_.show_employee) {
        const QString employee { MasterDataRegistry::Instance().PartnerName(node_o_->employee_id) };

        if (!employee.isEmpty()) {
            painter->drawText(QRectF(left, y, width / 2, line_height), Qt::AlignLeft | Qt::AlignVCenter, footer_config_.employee_title + employee);
        }
    }

    if (footer_config_.show_page_info) {
        painter->drawText(
            QRectF(left + width / 2, y, width / 2, line_height), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("%1/%2").arg(page_num).arg(total_pages));
    }

    painter->restore();

    return y + line_height;
}

void PrintHub::RenderAllPages(QPrinter* printer)
{
    if (!node_o_)
        return;

    QPainter painter(printer);
    painter.setPen(QPen(Qt::black, 0));

    QFont font { painter.font() };
    font.setPointSize(page_config_.font_size);
    painter.setFont(font);

    const QRectF page_rect { printer->pageLayout().paintRectPixels(printer->resolution()) };
    const qreal page_width { page_rect.width() };
    const qreal page_height { page_rect.height() };

    const qreal top_margin { static_cast<qreal>(page_config_.margin_top) };
    const qreal bottom_margin { static_cast<qreal>(page_config_.margin_bottom) };

    const qreal footer_height { MeasureFooterHeight(font) };
    const qreal content_bottom_limit { page_height - bottom_margin - footer_height };

    const auto widths { CalculateColumnWidths(page_width - page_config_.margin_left - page_config_.margin_right) };
    if (!table_config_.columns.isEmpty() && widths.size() != table_config_.columns.size()) {
        qWarning() << "Print aborted: column width configuration is invalid.";
        return;
    }

    const qreal row_h { static_cast<qreal>(table_config_.row_height) };
    const qreal table_header_h { table_config_.show_header ? row_h : 0.0 };
    const qsizetype total_rows { entry_list_.size() };

    // ---- 第一步：纯计算 ----
    // 公司信息 + 抬头现在每页都重复，所以每页表格起始位置固定不变
    const qreal company_h { MeasureCompanyHeight(font) };
    const qreal header_h { MeasureHeaderHeight(font) };
    const qreal table_top_per_page { top_margin + company_h + header_h };

    auto RowsCapacity = [&](qreal table_top) -> qsizetype {
        const qreal avail { content_bottom_limit - table_top - table_header_h };
        if (avail <= 0 || row_h <= 0)
            return 0;
        return static_cast<qsizetype>(avail / row_h);
    };

    const qsizetype cap { std::max<qsizetype>(RowsCapacity(table_top_per_page), 1) };

    int total_pages { total_rows > 0 ? static_cast<int>((total_rows + cap - 1) / cap) : 1 };

    // 合计+备注能否挤进最后一页表格之后
    const qreal summary_h { (total_config_.enabled ? row_h : 0.0) + MeasureRemarkHeight(font, page_width) };

    qsizetype last_page_rows { total_rows % cap };
    if (total_rows > 0 && last_page_rows == 0)
        last_page_rows = cap; // 整除时最后一页仍是满的

    const qreal last_page_table_bottom { table_top_per_page + table_header_h + last_page_rows * row_h };
    const bool summary_needs_new_page { last_page_table_bottom + summary_h > content_bottom_limit };
    if (summary_needs_new_page)
        total_pages += 1;

    // ---- 第二步：正式绘制，每页都重复公司信息 + 抬头 ----
    qsizetype start_index { 0 };
    int page_num { 1 };

    while (true) {
        qreal y { top_margin };
        y = DrawCompany(&painter, y, page_width);
        y = DrawHeader(&painter, y, page_width);

        const qsizetype end_index { std::min(start_index + cap, total_rows) };

        qreal cur_y { DrawTable(&painter, y, page_width, start_index, end_index) };

        const bool is_last_row_page { end_index >= total_rows };

        if (is_last_row_page && !summary_needs_new_page) {
            cur_y = DrawTotal(&painter, cur_y, page_width);
            DrawRemark(&painter, cur_y, page_width);

            DrawFooter(&painter, content_bottom_limit, page_width, page_num, total_pages);

            break;
        }

        DrawFooter(&painter, content_bottom_limit, page_width, page_num, total_pages);

        if (is_last_row_page && summary_needs_new_page) {
            printer->newPage();
            page_num++;

            qreal y2 { top_margin };
            y2 = DrawCompany(&painter, y2, page_width);
            y2 = DrawHeader(&painter, y2, page_width);
            y2 = DrawTotal(&painter, y2, page_width);

            DrawRemark(&painter, y2, page_width);

            DrawFooter(&painter, content_bottom_limit, page_width, page_num, total_pages);

            break;
        }

        printer->newPage();
        page_num++;
        start_index = end_index;
    }
}

QString PrintHub::GetColumnText(const QString& column, const Entry* entry, const MasterDataRegistry& master, const PartnerInventoryRegistry& partner) const
{
    const auto* d_entry { static_cast<const EntryO*>(entry) };

    if (column == QStringLiteral("internal_sku"))
        return master.InventoryPath(entry->rhs_node);

    if (column == QStringLiteral("external_sku"))
        return partner.ExternalSku(node_o_->partner_id, entry->rhs_node);

    if (column == QStringLiteral("description"))
        return entry->description;

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

void PrintHub::ApplyConfig(QPrinter* printer)
{
    QPageLayout layout { printer->pageLayout() };

    layout.setOrientation(
        page_config_.orientation.compare(QStringLiteral("landscape"), Qt::CaseInsensitive) == 0 ? QPageLayout::Landscape : QPageLayout::Portrait);

    layout.setPageSize(QPageSize(page_config_.page_size.compare(QStringLiteral("A4"), Qt::CaseInsensitive) == 0 ? QPageSize::A4 : QPageSize::A5));

    printer->setPageLayout(layout);
}

QList<qreal> PrintHub::CalculateColumnWidths(qreal available_width) const
{
    QList<qreal> result {};

    if (table_config_.column_widths.isEmpty())
        return result;

    const auto total = std::accumulate(table_config_.column_widths.cbegin(), table_config_.column_widths.cend(), 0.0);

    if (total <= 0.0)
        return result;

    result.reserve(table_config_.column_widths.size());

    for (const auto width : table_config_.column_widths)
        result.append(available_width * width / total);

    return result;
}

bool PrintHub::IsNumber(const QString& text)
{
    bool ok {};
    text.toDouble(&ok);
    return ok;
}
