/*
 * Copyright (C) 2023 YTX
 *
 * This file is part of YTX.
 *
 * YTX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * YTX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YTX. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <QPrinter>
#include <QSettings>

#include "component/config.h"
#include "global/masterdataregistry.h"
#include "global/partner_inventory_registry.h"
#include "table/entry.h"

class PrintFull {
public:
    PrintFull() = default;
    ~PrintFull() = default;

    PrintFull(const PrintFull&) = delete;
    PrintFull& operator=(const PrintFull&) = delete;

    bool LoadTemplate(QSettings& settings);
    void Render(QPrinter* printer, const NodeO* node_o, const QList<Entry*>& entry_list, const CSectionConfig* section_config);

private:
    struct LayoutConfig {
        int font_size { 12 };

        int margin_left { 20 };
        int margin_right { 20 };
        int margin_top { 15 };
        int margin_bottom { 15 };
    };

    struct CompanyConfig {
        bool show_logo { true };
        QString logo {};

        bool show_name { true };
        QString name {};

        bool show_address { true };
        QString address {};

        bool show_phone { true };
        QString phone {};
    };

    struct HeaderConfig {
        bool show_title { true };
        QString title {};

        bool show_partner { true };
        bool show_code { true };
        bool show_issued_time { true };
        bool show_settlement { true };
    };

    struct TableConfig {
        bool show_border { true };
        bool show_header { true };

        int row_height { 30 };

        QStringList columns {};
        QList<int> column_widths {};
        QHash<QString, QString> column_titles {};
    };

    struct TotalConfig {
        bool enabled { true };

        bool show_title { true };
        QString title {};

        bool show_upper { true };
        bool show_amount { true };
    };

    struct RemarkConfig {
        bool enabled { true };

        bool show_title { true };
        QString title {};
        QString text {};

        int padding { 6 };
        bool show_border { true };
    };

    struct FooterConfig {
        bool show_employee { true };
        QString employee_title {};

        bool show_page_info { true };
    };

private:
    void RenderAllPages(QPrinter* printer);

    void ReadLayoutConfig(QSettings& settings);
    void ReadCompanyConfig(QSettings& settings);
    void ReadHeaderConfig(QSettings& settings);
    void ReadTableConfig(QSettings& settings);
    void ReadTotalConfig(QSettings& settings);
    void ReadRemarkConfig(QSettings& settings);
    void ReadFooterConfig(QSettings& settings);

    qreal MeasureCompanyHeight(const QFont& base_font) const;
    qreal MeasureHeaderHeight(const QFont& base_font) const;
    qreal MeasureRemarkHeight(const QFont& base_font, qreal page_width) const;
    qreal MeasureFooterHeight(const QFont& base_font) const;

    qreal DrawCompany(QPainter* painter, qreal y, qreal page_width);
    qreal DrawHeader(QPainter* painter, qreal y, qreal page_width);

    qreal DrawTable(QPainter* painter, qreal y, qreal page_width, qsizetype start_index, qsizetype end_index);

    qreal DrawTotal(QPainter* painter, qreal y, qreal page_width);
    qreal DrawRemark(QPainter* painter, qreal y, qreal page_width);

    qreal DrawFooter(QPainter* painter, qreal y, qreal page_width, int page_num, int total_pages);

    QString GetColumnText(const QString& column, const Entry* entry, const MasterDataRegistry& master, const PartnerInventoryRegistry& partner) const;

    QList<qreal> CalculateColumnWidths(qreal available_width) const;

    static bool IsNumber(const QString& text);
    static QString NumberToChineseUpper(double value);
    static QString ConvertSection(int section, const QStringList& digits);

private:
    LayoutConfig layout_config_ {};

    CompanyConfig company_config_ {};
    HeaderConfig header_config_ {};
    TableConfig table_config_ {};
    TotalConfig total_config_ {};
    RemarkConfig remark_config_ {};
    FooterConfig footer_config_ {};

    const NodeO* node_o_ {};
    const QList<Entry*>* entry_list_ {};
    const CSectionConfig* section_config_ {};
};
