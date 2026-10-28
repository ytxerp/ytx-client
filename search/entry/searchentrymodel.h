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

#include <QAbstractItemModel>

#include "enum/section.h"
#include "table/entry.h"
#include "tag/tagrow.h"
#include "utils/templateutils.h"

using utils::DerivedPtr;

namespace search {

class EntryModel : public QAbstractItemModel {
    Q_OBJECT
public:
    ~EntryModel() override;

protected:
    explicit EntryModel(Section section, const QStringList& header, const QHash<QUuid, tag::Row>& tag_hash, QObject* parent = nullptr);

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& /*index*/) const override { return QModelIndex(); }
    int rowCount(const QModelIndex& parent = QModelIndex()) const override { return parent.isValid() ? 0 : list_.size(); }
    int columnCount(const QModelIndex& parent = QModelIndex()) const override { return parent.isValid() ? 0 : header_.size(); }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public:
    void Search(QUuid widget_id, const QString& text);
    void Rebuild(const QJsonArray& array);

protected:
    void Reset();

protected:
    const Section section_ {};
    const QStringList header_ {};
    const QHash<QUuid, tag::Row>& tag_hash_ {};

    EntryList list_ {};
};
}
