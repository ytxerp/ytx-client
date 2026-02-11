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

#ifndef SEARCHENTRYMODEL_H
#define SEARCHENTRYMODEL_H

#include <QAbstractItemModel>

#include "component/info.h"
#include "table/entry.h"
#include "tag/tag.h"
#include "utils/castutils.h"

using Utils::DerivedPtr;

class SearchEntryModel : public QAbstractItemModel {
    Q_OBJECT
public:
    ~SearchEntryModel() override { };

protected:
    explicit SearchEntryModel(CSectionInfo& info, const QHash<QUuid, Tag*>& tag_hash, QObject* parent = nullptr);

public slots:
    void RSearchEntry(const EntryList& entry_list);

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public:
    virtual void Search(const QString& text);

protected:
    void ClearModel();

protected:
    EntryList entry_list_ {};
    CSectionInfo& info_;
    const QHash<QUuid, Tag*>& tag_hash_ {};
};

#endif // SEARCHENTRYMODEL_H
