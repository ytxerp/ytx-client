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

#ifndef NODEOPARGO_H
#define NODEOPARGO_H

#include "component/config.h"
#include "entryhub/entryhub.h"
#include "table/model/tablemodel.h"
#include "tree/model/treemodel.h"
#include "tree/node.h"

// Order Node Operation Argument
struct NodeOpArgO {
    Node* node {};
    EntryHub* entry_hub {};
    TableModel* table_model {};
    TreeModel* tree_model_partner {};
    TreeModel* tree_model_inventory;
    CAppConfig& app_config {};
    CSectionConfig& section_config {};
    Section section {};
    bool is_new {};
    const QMap<QString, QString>& print_template {};
};

using CNodeOpArgO = const NodeOpArgO;

#endif // NODEOPARGO_H
