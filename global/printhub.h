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
#include "printfull.h"
#include "table/entry.h"

class PrintHub {
public:
    static PrintHub& Instance()
    {
        static PrintHub instance {};
        return instance;
    }

    void ScanTemplate();
    const QMap<QString, QString>& TemplateMap() const { return template_map_; }

    inline void SetAppConfig(CAppConfig* app) { app_config_ = app; }
    inline void SetSectionConfig(CSectionConfig* section) { section_config_ = section; }

    bool LoadTemplate(const QString& template_name);
    void SetValue(const NodeO* node_o, const QList<Entry*>& entry_list);

    void Preview();
    void Print();

private:
    enum class PrintMode {
        kFull,
        kOverlay,
    };

    struct PageConfig {
        QString page_size { "A5" };
        QString orientation { "landscape" };

        PrintMode print_mode { PrintMode::kFull };
    };

private:
    PrintHub() = default;
    ~PrintHub() = default;

    PrintHub(const PrintHub&) = delete;
    PrintHub& operator=(const PrintHub&) = delete;

    void RenderAllPages(QPrinter* printer);
    void ApplyConfig(QPrinter* printer);

    void ReadPageConfig(QSettings& settings);

private:
    PageConfig page_config_ {};
    QMap<QString, QString> template_map_ {};

    PrintFull full_ {};

    CAppConfig* app_config_ {};
    CSectionConfig* section_config_ {};

    QList<Entry*> entry_list_ {};
    const NodeO* node_o_ {};
};
