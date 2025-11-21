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

#ifndef SECTIONCONTEX_H
#define SECTIONCONTEX_H

#include <QPointer>

#include "component/config.h"
#include "entryhub/entryhub.h"
#include "report/widget/reportwidget.h"
#include "table/widget/tablewidget.h"
#include "tree/model/treemodel.h"
#include "tree/widget/treewidget.h"
#include "utils/templateutils.h"

struct SectionContext {
    QPointer<TreeWidget> tree_widget {};
    QPointer<EntryHub> entry_hub {};
    QPointer<TreeModel> tree_model {};
    QPointer<QTreeView> tree_view {};

    QList<QPointer<QDialog>> dialog_list {};

    SectionConfig section_config {};
    SectionInfo info {};
    SharedConfig shared_config {};

    QHash<QUuid, QPointer<TableWidget>> table_wgt_hash {};
    QHash<QUuid, QPointer<ReportWidget>> rpt_wgt_hash {};

    void Clear();
};

inline void SectionContext::Clear()
{
    TemplateUtils::SafeDelete(tree_widget);
    TemplateUtils::SafeDelete(entry_hub);
    TemplateUtils::SafeDelete(info.rule_model);
    TemplateUtils::SafeDelete(info.unit_model);
    TemplateUtils::SafeDelete(tree_model);

    section_config = SectionConfig {};
    shared_config = SharedConfig {};

    TemplateUtils::ClearWidgets(dialog_list);
    TemplateUtils::ClearWidgets(table_wgt_hash);
    TemplateUtils::ClearWidgets(rpt_wgt_hash);
}

#endif // SECTIONCONTEX_H
