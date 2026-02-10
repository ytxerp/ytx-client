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

#ifndef SEARCHDIALOGO_H
#define SEARCHDIALOGO_H

#include "searchdialog.h"

class SearchDialogO final : public SearchDialog {
    Q_OBJECT

public:
    SearchDialogO(CTreeModel* tree, SearchNodeModel* search_node, SearchEntryModel* search_entry, CTreeModel* inventory, CTreeModel* partner,
        CSectionConfig& config, CSectionInfo& info, const QHash<QUuid, Tag*>& tag_hash, QWidget* parent = nullptr);

private slots:
    void REntryDoubleClicked(const QModelIndex& index) override;
    void RNodeDoubleClicked(const QModelIndex& index) override;

private:
    void TreeViewDelegate(QTableView* view) override;
    void TableViewDelegate(QTableView* view) override;

private:
    CTreeModel* inventory_ {};
    CTreeModel* partner_ {};
};

#endif // SEARCHDIALOGO_H
