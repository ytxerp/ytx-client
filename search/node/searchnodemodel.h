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

#ifndef SEARCHNODEMODEL_H
#define SEARCHNODEMODEL_H

#include <QAbstractItemModel>

#include "component/info.h"
#include "component/using.h"
#include "tree/model/treemodel.h"
#include "utils/castutils.h"

using CastUtils::DerivedPtr;

class SearchNodeModel : public QAbstractItemModel {
    Q_OBJECT
public:
    virtual ~SearchNodeModel() = default;

public slots:
    virtual void RNodeSearch(const QJsonObject& obj) { Q_UNUSED(obj) }

protected:
    SearchNodeModel(CSectionInfo& info, CTreeModel* tree_model, QObject* parent = nullptr);

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    virtual void Search(CString& text);

protected:
    virtual void ResetData() { node_list_.clear(); }

protected:
    CSectionInfo& info_;
    CTreeModel* tree_model_ {};
    QList<Node*> node_list_ {};
    const Section section_ {};
};

#endif // SEARCHNODEMODEL_H
