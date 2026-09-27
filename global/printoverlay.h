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

struct FieldPosition {
    int x {};
    int y {};
};

class PrintOverlay {
public:
    PrintOverlay() = default;
    ~PrintOverlay() = default;
    PrintOverlay(const PrintOverlay&) = delete;
    PrintOverlay& operator=(const PrintOverlay&) = delete;

    bool LoadTemplate(QSettings& settings);
    void Render(QPrinter* printer, const NodeO* node_o, const QList<Entry*>& entry_list, const CSectionConfig* section_config);

private:
    void RenderAllPages(QPrinter* printer);
    void ReadFieldPosition(QSettings& settings, const QString& field);

    void DrawHeader(QPainter* painter);
    void DrawTable(QPainter* painter, long long start_index, long long end_index);
    void DrawFooter(QPainter* painter, int page_num, int total_pages);

    QString GetColumnText(int col, const Entry* entry, const MasterDataRegistry& master, const PartnerInventoryRegistry& partner) const;

    void DrawText(QPainter* painter, const QString& field, const QString& text);

    int GetFieldX(const QString& field, int default_x = 0) const
    {
        if (auto it = field_position_.constFind(field); it != field_position_.constEnd() && it.value().has_value())
            return it.value()->x;
        return default_x;
    }

    int GetFieldY(const QString& field, int default_y = 0) const
    {
        if (auto it = field_position_.constFind(field); it != field_position_.constEnd() && it.value().has_value())
            return it.value()->y;
        return default_y;
    }

private:
    int font_size_ { 12 };
    int row_height_ { 30 };

    QHash<QString, std::optional<FieldPosition>> field_position_ {};
    QList<int> column_widths_ {};

    const NodeO* node_o_ {};
    const QList<Entry*>* entry_list_ {};
    const CSectionConfig* section_config_ {};
};
