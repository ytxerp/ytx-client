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

#ifndef SEARCHDIALOGS_H
#define SEARCHDIALOGS_H

#include "searchdialog.h"

class SearchDialogS final : public SearchDialog {
    Q_OBJECT

public:
    SearchDialogS(CTreeModel* node, SearchNodeModel* search_node, SearchEntryModel* search_entry, CTreeModel* item_node, CSectionConfig& config,
        CSectionInfo& info, QWidget* parent = nullptr);

private slots:
    void REntryDoubleClicked(const QModelIndex& index) override;

private:
    void TreeViewDelegate(QTableView* view) override;
    void TableViewDelegate(QTableView* view) override;

private:
    CTreeModel* item_node_ {};
};

#endif // SEARCHDIALOGS_H
