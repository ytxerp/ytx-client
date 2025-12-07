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

#ifndef STATEMENTNODEMODEL_H
#define STATEMENTNODEMODEL_H

#include <QAbstractItemModel>

#include "component/info.h"
#include "statement.h"

class StatementNodeModel final : public QAbstractItemModel {
    Q_OBJECT
public:
    StatementNodeModel(CSectionInfo& info, const QUuid& partner_id, QObject* parent = nullptr);
    ~StatementNodeModel();

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order) override;
    void ResetModel(const QJsonArray& entry_array);

private:
    CSectionInfo& info_;
    const QUuid partner_id_ {};

    QList<StatementNode*> list_ {};
};

#endif // STATEMENTNODEMODEL_H
