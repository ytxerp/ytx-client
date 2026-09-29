#include "printfull.h"

#include <QtGui/qpainter.h>

#include "component/constantstring.h"
#include "printhub.h"
#include "utils/nodeutils.h"

void PrintFull::ReadCompanyConfig(QSettings& settings)
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

void PrintFull::ReadHeaderConfig(QSettings& settings)
{
    settings.beginGroup(QStringLiteral("header"));

    header_config_.show_title = settings.value(QStringLiteral("show_title"), true).toBool();

    header_config_.title = settings.value(QStringLiteral("title")).toString();

    header_config_.return_title = settings.value(QStringLiteral("return_title")).toString();

    header_config_.show_partner = settings.value(QStringLiteral("show_partner"), true).toBool();

    header_config_.show_code = settings.value(QStringLiteral("show_code"), true).toBool();

    header_config_.show_issued_time = settings.value(QStringLiteral("show_issued_time"), true).toBool();

    header_config_.show_settlement = settings.value(QStringLiteral("show_settlement"), true).toBool();

    settings.endGroup();
}

void PrintFull::ReadTableConfig(QSettings& settings)
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

void PrintFull::ReadTotalConfig(QSettings& settings)
{
    settings.beginGroup(QStringLiteral("total"));

    total_config_.enabled = settings.value(QStringLiteral("enabled"), true).toBool();

    total_config_.show_title = settings.value(QStringLiteral("show_title"), true).toBool();

    total_config_.title = settings.value(QStringLiteral("title")).toString();

    total_config_.show_upper = settings.value(QStringLiteral("show_upper"), true).toBool();

    total_config_.show_amount = settings.value(QStringLiteral("show_amount"), true).toBool();

    settings.endGroup();
}

void PrintFull::ReadRemarkConfig(QSettings& settings)
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

void PrintFull::ReadFooterConfig(QSettings& settings)
{
    settings.beginGroup(QStringLiteral("footer"));

    footer_config_.show_employee = settings.value(QStringLiteral("show_employee"), true).toBool();

    footer_config_.employee_title = settings.value(QStringLiteral("employee_title")).toString();

    footer_config_.show_page_info = settings.value(QStringLiteral("show_page_info"), true).toBool();

    settings.endGroup();
}

void PrintFull::ReadLayoutConfig(QSettings& settings)
{
    settings.beginGroup(QStringLiteral("page"));

    layout_config_.font_size = settings.value(QStringLiteral("font_size"), 12).toInt();

    layout_config_.margin_left = settings.value(QStringLiteral("margin_left"), 20).toInt();

    layout_config_.margin_right = settings.value(QStringLiteral("margin_right"), 20).toInt();

    layout_config_.margin_top = settings.value(QStringLiteral("margin_top"), 15).toInt();

    layout_config_.margin_bottom = settings.value(QStringLiteral("margin_bottom"), 15).toInt();

    settings.endGroup();
}

qreal PrintFull::DrawCompany(QPainter* painter, qreal y, qreal page_width)
{
    painter->save();

    const qreal left { static_cast<qreal>(layout_config_.margin_left) };
    const qreal width { page_width - layout_config_.margin_left - layout_config_.margin_right };

    const QFont original_font { painter->font() };

    if (company_config_.show_logo && !company_config_.logo.trimmed().isEmpty()) {
        // Logo drawing can be added here.
        // y += logoHeight + spacing;
    }

    if (company_config_.show_name && !company_config_.name.trimmed().isEmpty()) {
        QFont font { original_font };
        font.setBold(true);
        font.setPointSize(original_font.pointSize() + 8);

        painter->setFont(font);

        const QFontMetricsF fm { font };
        const qreal height { fm.height() };

        painter->drawText(QRectF(left, y, width, height), Qt::AlignCenter, company_config_.name);

        y += height + 2;
    }

    QFont info_font { original_font };
    info_font.setPointSize(qMax(1, original_font.pointSize() - 2));

    painter->setFont(info_font);

    const QFontMetricsF info_fm { info_font };
    const qreal info_height { info_fm.height() };

    if (company_config_.show_address && !company_config_.address.trimmed().isEmpty()) {
        painter->drawText(QRectF(left, y, width, info_height), Qt::AlignCenter, company_config_.address);

        y += info_height;
    }

    if (company_config_.show_phone && !company_config_.phone.trimmed().isEmpty()) {
        painter->drawText(QRectF(left, y, width, info_height), Qt::AlignCenter, company_config_.phone);

        y += info_height;
    }

    painter->restore();

    return y;
}

qreal PrintFull::DrawHeader(QPainter* painter, qreal y, qreal page_width)
{
    painter->save();

    const qreal left { static_cast<qreal>(layout_config_.margin_left) };
    const qreal width { page_width - layout_config_.margin_left - layout_config_.margin_right };

    const qreal line_height { QFontMetricsF(painter->font()).height() + 4 };
    const qreal column_width { width / 3.0 };

    qreal left_y { y };
    qreal right_y { y };
    qreal title_bottom { y };

    // Partner
    if (header_config_.show_partner) {
        const auto& master { MasterDataRegistry::Instance() };
        const auto partner_unit { master.PartnerUnit(node_o_->partner_id) };
        const QString partner_name { master.PartnerName(node_o_->partner_id) };

        const QString partner_title { partner_unit == NodeUnit::PCustomer ? QObject::tr("Customer: ", "Print") : QObject::tr("Supplier: ", "Print") };

        const QString text { partner_title + partner_name };

        painter->drawText(QRectF(left, left_y, column_width, line_height), Qt::AlignLeft | Qt::AlignVCenter, text);

        left_y += line_height;
    }

    // Settlement
    if (header_config_.show_settlement) {
        const QString settlement { node::UnitString(NodeUnit(node_o_->unit)) };

        painter->drawText(QRectF(left, left_y, column_width, line_height), Qt::AlignLeft | Qt::AlignVCenter, QObject::tr("Settlement: ") + settlement);

        left_y += line_height;
    }

    // Title
    const QString& title { node_o_->direction_rule == direction_rule::kRO ? header_config_.return_title : header_config_.title };

    if (header_config_.show_title && !title.trimmed().isEmpty()) {
        QFont font { painter->font() };
        font.setBold(true);
        font.setPointSize(font.pointSize() + 6);

        painter->save();
        painter->setFont(font);

        const qreal title_height { line_height * 2 };

        painter->drawText(QRectF(left + column_width, y, column_width, title_height), Qt::AlignHCenter | Qt::AlignTop, title);

        painter->restore();

        title_bottom = y + title_height;
    }

    // Date
    if (header_config_.show_issued_time) {
        painter->drawText(QRectF(left + column_width * 2, right_y, column_width, line_height), Qt::AlignRight | Qt::AlignVCenter,
            QObject::tr("Date: ") + node_o_->issued_time.toString(datetime_format::kDashedDate));

        right_y += line_height;
    }

    // Code
    if (header_config_.show_code) {
        painter->drawText(
            QRectF(left + column_width * 2, right_y, column_width, line_height), Qt::AlignRight | Qt::AlignVCenter, QObject::tr("Code: ") + node_o_->code);

        right_y += line_height;
    }

    painter->restore();

    return std::max({ left_y, right_y, title_bottom }) + 6;
}

qreal PrintFull::DrawTable(QPainter* painter, qreal y, qreal page_width, qsizetype start_index, qsizetype end_index)
{
    painter->save();

    const qreal left { static_cast<qreal>(layout_config_.margin_left) };
    const qreal available_width { page_width - layout_config_.margin_left - layout_config_.margin_right };

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

        const qreal available_width { qMax<qreal>(1.0, rect.width() - padding * 2) };

        const QFontMetricsF fm { font, painter->device() };

        if (fm.horizontalAdvance(text) > available_width) {
            const int best_size { PrintHub::FindBestFontSize(font, painter->device(), text, static_cast<int>(available_width), font.pointSize()) };

            font.setPointSize(best_size);
        }

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
        const auto* entry { entry_list_->at(row) };

        qreal x { left };

        for (qsizetype col = 0; col < table_config_.columns.size(); ++col) {
            const auto& column { table_config_.columns.at(col) };

            const QString text { GetColumnText(column, entry, master, partner) };

            const QRectF rect { x, y, widths.at(col), static_cast<qreal>(table_config_.row_height) };

            Qt::Alignment alignment { Qt::AlignLeft };

            if (PrintHub::IsNumber(text))
                alignment = Qt::AlignRight;

            draw_cell(rect, text, alignment);

            x += widths.at(col);
        }

        y += table_config_.row_height;
    }

    painter->restore();

    return y;
}

qreal PrintFull::MeasureCompanyHeight(const QFont& base_font) const
{
    qreal height { 0.0 };

    if (company_config_.show_name && !company_config_.name.trimmed().isEmpty()) {
        QFont font { base_font };
        font.setBold(true);
        font.setPointSize(base_font.pointSize() + 8);

        height += QFontMetricsF(font).height() + 2;
    }

    QFont info_font { base_font };
    info_font.setPointSize(qMax(1, base_font.pointSize() - 2));

    const qreal info_height { QFontMetricsF(info_font).height() };

    if (company_config_.show_address && !company_config_.address.trimmed().isEmpty())
        height += info_height;

    if (company_config_.show_phone && !company_config_.phone.trimmed().isEmpty())
        height += info_height;

    return height;
}

qreal PrintFull::MeasureHeaderHeight(const QFont& base_font) const
{
    const qreal line_height { QFontMetricsF(base_font).height() + 4 };

    qreal left_h { 0.0 };
    qreal right_h { 0.0 };
    qreal title_h { 0.0 };

    if (header_config_.show_partner)
        left_h += line_height;

    if (header_config_.show_settlement)
        left_h += line_height;

    if (header_config_.show_issued_time)
        right_h += line_height;

    if (header_config_.show_code)
        right_h += line_height;

    const QString& title { node_o_->direction_rule == direction_rule::kRO ? header_config_.return_title : header_config_.title };

    if (header_config_.show_title && !title.trimmed().isEmpty())
        title_h = line_height * 2;

    return std::max({ left_h, right_h, title_h }) + 6;
}

qreal PrintFull::MeasureRemarkHeight(const QFont& base_font, qreal page_width) const
{
    if (!remark_config_.enabled || remark_config_.text.trimmed().isEmpty())
        return 0.0;

    QFont font { base_font };
    font.setPointSize(qMax(1, font.pointSize() - 2));

    const qreal width { page_width - layout_config_.margin_left - layout_config_.margin_right };

    const qreal text_width { width - remark_config_.padding * 2 };

    QString text {};

    if (remark_config_.show_title && !remark_config_.title.trimmed().isEmpty()) {
        text += remark_config_.title;
        text += '\n';
    }

    text += remark_config_.text;

    const QFontMetricsF fm { font };

    const QRectF bounds { fm.boundingRect(QRectF(0, 0, text_width, 10000), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text) };

    return bounds.height() + remark_config_.padding * 2;
}

qreal PrintFull::MeasureFooterHeight(const QFont& base_font) const
{
    if (!footer_config_.show_employee && !footer_config_.show_page_info)
        return 0.0;

    return QFontMetricsF(base_font).height() + 6;
}

qreal PrintFull::DrawTotal(QPainter* painter, qreal y, qreal page_width)
{
    if (!total_config_.enabled)
        return y;

    painter->save();

    const qreal left { static_cast<qreal>(layout_config_.margin_left) };
    const qreal width { page_width - layout_config_.margin_left - layout_config_.margin_right };

    const qreal height { static_cast<qreal>(table_config_.row_height) };

    const QString amount { QString::number(node_o_->initial_total, 'f', section_config_->amount_decimal) };

    QStringList parts {};

    if (total_config_.show_title && !total_config_.title.trimmed().isEmpty()) {
        parts.append(total_config_.title);
    }

    if (total_config_.show_upper) {
        parts.append(QObject::tr("Uppercase: ") + PrintHub::NumberToChineseUpper(node_o_->initial_total));
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

qreal PrintFull::DrawRemark(QPainter* painter, qreal y, qreal page_width)
{
    if (!remark_config_.enabled || remark_config_.text.trimmed().isEmpty()) {
        return y;
    }

    painter->save();

    QFont font { painter->font() };
    font.setPointSize(qMax(1, font.pointSize() - 2));
    painter->setFont(font);

    const qreal left { static_cast<qreal>(layout_config_.margin_left) };
    const qreal width { page_width - layout_config_.margin_left - layout_config_.margin_right };

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

qreal PrintFull::DrawFooter(QPainter* painter, qreal y, qreal page_width, int page_num, int total_pages, bool is_last_page)
{
    painter->save();

    const qreal left { static_cast<qreal>(layout_config_.margin_left) };
    const qreal width { page_width - layout_config_.margin_left - layout_config_.margin_right };

    const qreal line_height { QFontMetricsF(painter->font()).height() + 6 };

    // Employee only appears on the last page
    if (is_last_page && footer_config_.show_employee) {
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

void PrintFull::RenderAllPages(QPrinter* printer)
{
    if (!printer || !node_o_ || !section_config_)
        return;

    QPainter painter(printer);
    painter.setPen(QPen(Qt::black, 0));

    QFont font { painter.font() };
    font.setPointSize(layout_config_.font_size);
    painter.setFont(font);

    const QRectF page_rect { printer->pageLayout().paintRectPixels(printer->resolution()) };

    const qreal page_width { page_rect.width() };

    const qreal page_height { page_rect.height() };

    const qreal top_margin { static_cast<qreal>(layout_config_.margin_top) };

    const qreal bottom_margin { static_cast<qreal>(layout_config_.margin_bottom) };

    const auto widths { CalculateColumnWidths(page_width - layout_config_.margin_left - layout_config_.margin_right) };

    if (!table_config_.columns.isEmpty() && widths.size() != table_config_.columns.size()) {
        qWarning() << "Print aborted: column width configuration is invalid.";
        return;
    }

    const qreal row_h { static_cast<qreal>(table_config_.row_height) };

    const qreal table_header_h { table_config_.show_header ? row_h : 0.0 };

    const qsizetype total_rows { entry_list_->size() };

    // ========================================================
    // Layout measurement
    // ========================================================

    const qreal company_h { MeasureCompanyHeight(font) };

    const qreal header_h { MeasureHeaderHeight(font) };

    const qreal footer_h { MeasureFooterHeight(font) };

    const qreal remark_h { MeasureRemarkHeight(font, page_width) };

    // Total + Remark
    // Footer is reserved separately on every page.
    const qreal summary_h { (total_config_.enabled ? row_h : 0.0) + remark_h };

    const qreal table_top { top_margin + company_h + header_h };

    const qreal page_bottom { page_height - bottom_margin };

    // Footer is fixed at the bottom of every page.
    const qreal footer_y { page_bottom - footer_h };

    // ========================================================
    // Row capacity
    // ========================================================

    // Normal pages:
    // reserve space for footer/page info.
    const qreal normal_available { footer_y - table_top - table_header_h };

    const qsizetype normal_capacity { std::max<qsizetype>(static_cast<qsizetype>(normal_available / row_h), 1) };

    // Final page:
    // reserve space for Total + Remark + Footer.
    const qreal final_available { footer_y - table_top - table_header_h - summary_h };

    const qsizetype final_capacity { std::max<qsizetype>(static_cast<qsizetype>(final_available / row_h), 0) };

    // ========================================================
    // Calculate total pages
    // ========================================================

    int total_pages { 1 };

    if (total_rows > final_capacity) {
        const qsizetype rows_before_final { total_rows - final_capacity };

        const int normal_pages { static_cast<int>((rows_before_final + normal_capacity - 1) / normal_capacity) };

        total_pages = normal_pages + 1;
    }

    // ========================================================
    // Render
    // ========================================================

    qsizetype start_index { 0 };

    for (int page_num = 1; page_num <= total_pages; ++page_num) {
        if (page_num != 1)
            printer->newPage();

        qreal y { top_margin };

        y = DrawCompany(&painter, y, page_width);

        y = DrawHeader(&painter, y, page_width);

        const bool is_last_page { page_num == total_pages };

        qsizetype rows_this_page {};

        if (is_last_page) {
            // All remaining rows belong to the final page.
            rows_this_page = total_rows - start_index;
        } else {
            const qsizetype remaining { total_rows - start_index };

            // Keep enough rows for the final page so that
            // Total + Remark do not occupy a page alone.
            const qsizetype must_keep_for_final { final_capacity };

            rows_this_page = std::min(normal_capacity, std::max<qsizetype>(remaining - must_keep_for_final, 0));
        }

        const qsizetype end_index { start_index + rows_this_page };

        qreal cur_y { DrawTable(&painter, y, page_width, start_index, end_index) };

        // Total and Remark only appear on the final page.
        if (is_last_page) {
            cur_y = DrawTotal(&painter, cur_y, page_width);

            cur_y = DrawRemark(&painter, cur_y, page_width);
        }

        // Footer appears on every page.
        // Employee is controlled by is_last_page inside DrawFooter().
        DrawFooter(&painter, footer_y, page_width, page_num, total_pages, is_last_page);

        start_index = end_index;
    }
}

QString PrintFull::GetColumnText(const QString& column, const Entry* entry, const MasterDataRegistry& master, const PartnerInventoryRegistry& partner) const
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

QList<qreal> PrintFull::CalculateColumnWidths(qreal available_width) const
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

bool PrintFull::LoadTemplate(QSettings& settings)
{
    ReadLayoutConfig(settings);
    ReadCompanyConfig(settings);
    ReadHeaderConfig(settings);
    ReadTableConfig(settings);
    ReadTotalConfig(settings);
    ReadRemarkConfig(settings);
    ReadFooterConfig(settings);

    if (layout_config_.font_size <= 0 || table_config_.row_height <= 0) {
        qWarning() << "Invalid full print layout configuration.";
        return false;
    }

    if (table_config_.columns.isEmpty() || table_config_.columns.size() != table_config_.column_widths.size()) {
        qWarning() << "Print template column configuration is invalid.";
        return false;
    }

    if (std::ranges::any_of(table_config_.column_widths, [](int width) { return width <= 0; })) {
        qWarning() << "Print template column width must be greater than 0.";
        return false;
    }

    return true;
}

void PrintFull::Render(QPrinter* printer, const NodeO* node_o, const QList<Entry*>& entry_list, const CSectionConfig* section_config)
{
    node_o_ = node_o;
    entry_list_ = &entry_list;
    section_config_ = section_config;

    RenderAllPages(printer);
}
