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
#include <QString>

#include "component/config.h"
#include "table/entryshadow.h"
#include "tree/model/treemodel.h"

struct FieldSettings {
    int x {};
    int y {};
};

struct PrintData {
    QString partner {};
    QString issued_time {};
    QString employee {};
    QString unit {};
    double initial_total {};
};

class PrintManager {
public:
    PrintManager(CAppConfig& app_config, TreeModel* inventory, TreeModel* partner);

    bool LoadIni(const QString& file_path);
    void SetData(const PrintData& print_data, const QList<EntryShadow*>& entry_shadow_list);
    void Preview();
    void Print();

private:
    void RenderAllPages(QPrinter* printer);

    void ApplyConfig(QPrinter* printer);
    void ReadFieldPosition(QSettings& settings, const QString& section, const QString& prefix);

    void DrawHeader(QPainter* painter);
    void DrawTable(QPainter* painter, long long start_index, long long end_index);
    void DrawFooter(QPainter* painter, int page_num, int total_pages);

    QString GetColumnText(int col, const EntryShadow* entry_shadow);

private:
    QHash<QString, QVariant> page_settings_ {};
    QHash<QString, FieldSettings> field_settings_ {};
    CAppConfig& app_config_ {};

    QList<EntryShadow*> entry_shadow_list_ {};
    PrintData data_ {};

    TreeModel* inventory_ {};
    TreeModel* partner_ {};
};

#endif // PRINTMANAGER_H
