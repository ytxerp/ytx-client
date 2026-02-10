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

#ifndef SEARCHENTRYMODELP_H
#define SEARCHENTRYMODELP_H

#include "entryhub/entryhubp.h"
#include "searchentrymodel.h"

class SearchEntryModelP final : public SearchEntryModel {
    Q_OBJECT
public:
    SearchEntryModelP(EntryHub* entry_hub, CSectionInfo& info, const QHash<QUuid, Tag*>& tag_hash, QObject* parent = nullptr);

public:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order) override;

    void Search(const QString& text) override;

private:
    EntryHubP* entry_hub_p_ {};
};

#endif // SEARCHENTRYMODELP_H
