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

#ifndef PRINTHUB_H
#define PRINTHUB_H

#include <QPrinter>
#include <QSettings>

#include "component/config.h"
#include "table/entry.h"
#include "tree/model/treemodel.h"

struct FieldPosition {
    int x {};
    int y {};
};

class PrintHub {
public:
    static PrintHub& Instance()
    {
        static PrintHub instance {};
        return instance;
    }

    void ScanTemplate();
    const QMap<QString, QString>& TemplateMap() { return template_map_; }

    void SetAppConfig(CAppConfig* config) { app_config_ = config; }
    void SetSectionConfig(CSectionConfig* config) { section_config_ = config; }
    void SetInventoryModel(TreeModel* inventory) { inventory_ = inventory; }
    void SetPartnerModel(TreeModel* partner) { partner_ = partner; }

    bool LoadTemplate(const QString& template_name);
    void SetValue(const NodeO* node_o, const QList<Entry*>& entry_list);
    void Preview();
    void Print();

private:
    PrintHub() = default;
    ~PrintHub() = default;
    PrintHub(const PrintHub&) = delete;
    PrintHub& operator=(const PrintHub&) = delete;

    void RenderAllPages(QPrinter* printer);
    void ApplyConfig(QPrinter* printer);
    void ReadFieldPosition(QSettings& settings, const QString& group, const QString& field);

    void DrawHeader(QPainter* painter);
    void DrawTable(QPainter* painter, long long start_index, long long end_index);
    void DrawFooter(QPainter* painter, int page_num, int total_pages);

    QString GetColumnText(int col, const Entry* entry);

    QString NumberToChineseUpper(double value);
    QString ConvertSection(int section, const QStringList& digits, const QStringList& units);

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
    QHash<QString, QVariant> page_values_ {};
    QHash<QString, std::optional<FieldPosition>> field_position_ {};

    QMap<QString, QString> template_map_ {};

    CAppConfig* app_config_ {};
    CSectionConfig* section_config_ {};
    QList<Entry*> entry_list_ {};
    const NodeO* node_o_ {};
    QString current_template_ {};

    TreeModel* inventory_ {};
    TreeModel* partner_ {};

    int row_height_ {};
    QList<int> column_widths_ {};
};

#endif // PRINTHUB_H
