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

#ifndef PRINTMANAGER_H
#define PRINTMANAGER_H

#include <QPrinter>
#include <QSettings>

#include "component/config.h"
#include "table/entry.h"
#include "tree/model/treemodel.h"

struct FieldPosition {
    int x {};
    int y {};
};

class PrintManager {
public:
    static PrintManager& Instance()
    {
        static PrintManager instance {};
        return instance;
    }

    void ScanTemplate();
    const QMap<QString, QString>& TemplateMap() { return template_map_; }

    void SetAppConfig(CAppConfig* config) { app_config_ = config; }
    void SetInventoryModel(TreeModel* inventory) { inventory_ = inventory; }
    void SetPartnerModel(TreeModel* partner) { partner_ = partner; }

    bool LoadTemplate(const QString& template_name);
    void SetValue(const NodeO* node_o, const QList<Entry*>& entry_list);
    void Preview();
    void Print();

private:
    PrintManager() = default;
    ~PrintManager() = default;
    PrintManager(const PrintManager&) = delete;
    PrintManager& operator=(const PrintManager&) = delete;

    void RenderAllPages(QPrinter* printer);
    void ApplyConfig(QPrinter* printer);
    void ReadFieldPosition(QSettings& settings, const QString& group, const QString& field);

    void DrawHeader(QPainter* painter);
    void DrawTable(QPainter* painter, long long start_index, long long end_index);
    void DrawFooter(QPainter* painter, int page_num, int total_pages);

    QString GetColumnText(int col, const Entry* entry);

    QString NumberToChineseUpper(double value);
    QString ConvertSection(int section, const QStringList& digits, const QStringList& units);

private:
    QHash<QString, QVariant> page_values_ {};
    QHash<QString, FieldPosition> field_position_ {};

    QMap<QString, QString> template_map_ {};

    CAppConfig* app_config_ {};
    QList<Entry*> entry_list_ {};
    const NodeO* node_o_ {};
    QString current_template_ {};

    TreeModel* inventory_ {};
    TreeModel* partner_ {};

    int row_height_ {};
    QList<int> column_widths_ {};
};

#endif // PRINTMANAGER_H
