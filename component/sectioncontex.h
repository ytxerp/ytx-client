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

#include "component/config.h"
#include "entryhub/entryhub.h"
#include "tag/tag.h"
#include "tree/model/treemodel.h"
#include "tree/widget/treewidget.h"
#include "widgetcontex.h"

struct SectionContext {
    QPointer<TreeWidget> tree_widget {};
    QPointer<EntryHub> entry_hub {};
    QPointer<TreeModel> tree_model {};
    QPointer<QTreeView> tree_view {};
    QPointer<QTabWidget> tab_widget {};

    SectionConfig section_config {};
    SectionInfo info {};
    SharedConfig shared_config {};

    QHash<QUuid, WidgetContext> widget_hash {};

    QHash<QUuid, Tag*> tag_hash {};
    QHash<QUuid, TagIcons> tag_icons_hash {};
};

#endif // SECTIONCONTEX_H
