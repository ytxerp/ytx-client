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

qreal PrintFull::MeasureHeaderHeight(const QFont& base_font) const
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

qreal PrintFull::MeasureRemarkHeight(const QFont& base_font, qreal page_width) const
{
    if (!remark_config_.enabled || remark_config_.text.trimmed().isEmpty())
        return 0.0;

    const qreal width { page_width - layout_config_.margin_left - layout_config_.margin_right };
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

qreal PrintFull::DrawFooter(QPainter* painter, qreal y, qreal page_width, int page_num, int total_pages)
{
    painter->save();

    const qreal left { static_cast<qreal>(layout_config_.margin_left) };
    const qreal width { page_width - layout_config_.margin_left - layout_config_.margin_right };

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

void PrintFull::RenderAllPages(QPrinter* printer)
{
    if (!node_o_)
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

    const qreal footer_height { MeasureFooterHeight(font) };
    const qreal content_bottom_limit { page_height - bottom_margin - footer_height };

    const auto widths { CalculateColumnWidths(page_width - layout_config_.margin_left - layout_config_.margin_right) };
    if (!table_config_.columns.isEmpty() && widths.size() != table_config_.columns.size()) {
        qWarning() << "Print aborted: column width configuration is invalid.";
        return;
    }

    const qreal row_h { static_cast<qreal>(table_config_.row_height) };
    const qreal table_header_h { table_config_.show_header ? row_h : 0.0 };
    const qsizetype total_rows { entry_list_->size() };

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

QString PrintFull::GetColumnText(const QString& column, const Entry* entry, const MasterDataRegistry& master, const PartnerInventoryRegistry& partner) const
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

    if (table_config_.columns.size() != table_config_.column_widths.size()) {
        qWarning() << "Print template column count does not match column width count:" << table_config_.columns.size() << table_config_.column_widths.size();

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
