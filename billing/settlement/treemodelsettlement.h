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

#ifndef TREEMODELSETTLEMENT_H
#define TREEMODELSETTLEMENT_H

#include <QAbstractItemModel>
#include <QJsonObject>

#include "component/info.h"
#include "settlement.h"

class TreeModelSettlement final : public QAbstractItemModel {
    Q_OBJECT
public:
    TreeModelSettlement(CSectionInfo& info, QObject* parent = nullptr);
    ~TreeModelSettlement();

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order) override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    bool InsertSucceeded(Settlement* settlement, const QJsonObject& meta);
    void RecallSucceeded(const QUuid& settlement_id, const QJsonObject& meta);
    void UpdateSucceeded(const QUuid& settlement_id, double amount, const QJsonObject& meta);
    void ResetModel(const QJsonArray& array);

    Settlement* FindSettlement(const QUuid& settlement_id) const
    {
        for (auto* s : std::as_const(list_)) {
            if (s && s->id == settlement_id) {
                return s;
            }
        }

        return nullptr;
    }

private:
    CSectionInfo& info_;
    QList<Settlement*> list_ {};
};

#endif // TREEMODELSETTLEMENT_H
