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

#ifndef SEARCHNODEMODELO_H
#define SEARCHNODEMODELO_H

#include "searchnodemodel.h"

class SearchNodeModelO final : public SearchNodeModel {
    Q_OBJECT

public:
    SearchNodeModelO(CSectionInfo& info, CTreeModel* tree_model, const QHash<QUuid, Tag*>& tag_hash, QObject* parent = nullptr);

public slots:
    void RNodeSearch(const QJsonObject& obj) override;

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order) override;

    void Search(CString& text) override;
};

#endif // SEARCHNODEMODELO_H
