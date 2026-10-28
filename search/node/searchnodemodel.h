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

#include "component/using.h"
#include "tag/tagrow.h"
#include "tree/model/treemodel.h"

using utils::DerivedPtr;

namespace search {

class NodeModel : public QAbstractItemModel {
    Q_OBJECT
public:
    ~NodeModel() override { };

protected:
    NodeModel(Section section, const QStringList& header, CTreeModel* tree_model, const QHash<QUuid, tag::Row>& tag_hash, QObject* parent = nullptr);

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& /*index*/) const override { return QModelIndex(); }
    int rowCount(const QModelIndex& parent = QModelIndex()) const override { return parent.isValid() ? 0 : list_.size(); }
    int columnCount(const QModelIndex& parent = QModelIndex()) const override { return parent.isValid() ? 0 : header_.size(); }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    virtual void Search(QUuid widget_id, CString& text);
    virtual void Rebuild(const QJsonArray& array) { Q_UNUSED(array) }

protected:
    const Section section_ {};
    const QStringList header_ {};
    CTreeModel* tree_model_ {};
    const QHash<QUuid, tag::Row>& tag_hash_ {};

    QList<Node*> list_ {};
};
}
