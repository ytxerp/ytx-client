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

#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QMimeData>
#include <QSortFilterProxyModel>
#include <QTimer>

#include "component/constant.h"
#include "component/info.h"
#include "utils/castutils.h"
#include "utils/nodeutils.h"

using Utils::DerivedPtr;

class TreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    ~TreeModel() override;

protected:
    explicit TreeModel(CSectionInfo& info, CString& separator, QObject* parent = nullptr);

signals:
    // send to LeafSStation
    void SDirectionRule(const QUuid& node_id, bool value);

    // send to its view
    void SResizeColumnToContents(int column);

    // send to Mainwindow
    void SUpdateName(const QUuid& node_id, const QString& name, bool branch);
    void SFreeWidget(Section section, const QUuid& node_id);

    // send to NodeWidget
    void SSyncValue();
    void SInitStatus();

public slots:
    // receive from DeleteDialog
    void RDeleteNode(const QUuid& node_id);

public:
    // Qt's
    // Default implementations
    QModelIndex parent(const QModelIndex& index) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) override;

    inline Qt::DropActions supportedDropActions() const override { return Qt::CopyAction | Qt::MoveAction; }
    inline QStringList mimeTypes() const override { return QStringList { kYTX }; }

    inline bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex& /*parent*/) const override
    {
        return data && data->hasFormat(kYTX) && action != Qt::IgnoreAction;
    }

    inline int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return node_header_.size();
    }

    inline QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            return node_header_.at(section);
        }

        return QVariant();
    }

    // WebSocket functions
    void ApplyTree(const QJsonObject& data);

    void InsertNode(const QUuid& ancestor, const QJsonObject& data);
    void InsertMeta(const QUuid& node_id, const QJsonObject& meta);

    void SyncNode(const QUuid& node_id, const QJsonObject& update);
    void UpdateMeta(const QUuid& node_id, const QJsonObject& meta);

    void ReplaceLeaf(const QUuid& old_node_id, const QUuid& new_node_id);
    void DragNode(const QUuid& ancestor, const QUuid& descendant);

    void SyncDirectionRule(const QUuid& node_id, bool direction_rule);
    void SyncTotalArray(const QJsonArray& total_array);

    virtual void UpdateName(const QUuid& node_id, const QString& name);

    // Ytx's
    // Default implementations
    double InitialTotal(QUuid node_id) const { return Utils::Value(node_hash_, node_id, &Node::initial_total); }
    double FinalTotal(QUuid node_id) const { return Utils::Value(node_hash_, node_id, &Node::final_total); }
    NodeUnit Unit(QUuid node_id) const { return Utils::Value(node_hash_, node_id, &Node::unit); }
    bool Rule(QUuid node_id) const { return Utils::Value(node_hash_, node_id, &Node::direction_rule); }
    QString Name(QUuid node_id) const { return Utils::Value(node_hash_, node_id, &Node::name); }
    QString Path(const QUuid& node_id) const;

    inline ItemModel* LeafModel() const { return leaf_path_model_; }
    inline CUuidString& LeafPath() const { return leaf_path_; }

    void LeafPathBranchPathModel(ItemModel* model) const;

    void UpdateSeparator(CString& old_separator, CString& new_separator);

    void AckNode(const QUuid& node_id) const;
    void SearchName(QList<Node*>& node_list, CString& name) const;
    void SearchTag(QList<Node*>& node_list, const QSet<QString>& tag_set) const;

    void Reset();
    void FlushCaches();

    inline bool Contains(const QUuid& node_id) const { return node_hash_.contains(node_id); }
    inline Node* GetNode(const QUuid& node_id) const
    {
        auto* node { node_hash_.value(node_id, nullptr) };
        if (!node) {
            qInfo() << "Node not found for section:" << std::to_underlying(section_) << "id:" << node_id;
        }
        return node;
    }

    QModelIndex GetIndex(const QUuid& node_id) const;
    Node* GetNodeByIndex(const QModelIndex& index) const;

    QSortFilterProxyModel* ExcludeId(const QUuid& node_id, QObject* parent);
    QSortFilterProxyModel* IncludeUnit(NodeUnit unit, QObject* parent);
    QSortFilterProxyModel* ExcludeIdUnit(const QUuid& node_id, NodeUnit unit, QObject* parent);
    QSortFilterProxyModel* ReplaceSelf(const QUuid& node_id, NodeUnit unit, QObject* parent);

protected:
    void SortModel();

    void InitRoot(Node*& root);

    void BuildHierarchy(const QJsonArray& path_array);

    void RestartTimer(const QUuid& id);
    void EmitRowChanged(const QUuid& node_id, int start_column, int end_column);

    void UpdateDirectionRule(Node* node, bool value);
    void DirectionRuleImpl(Node* node, bool value);

    void UpdateMeta(Node* node, const QJsonObject& meta);
    void InsertMeta(Node* node, const QJsonObject& meta);

    void RefreshAffectedTotal(const QSet<QUuid>& affected_ids);

    QSet<QUuid> UpdateTotal(const QUuid& node_id, double initial_total, double final_total);

    void RemoveUnitSet(const QUuid& node_id, NodeUnit unit)
    {
        if (auto* set = UnitSet(unit)) {
            set->remove(node_id);
        }
    }

    void InsertUnitSet(const QUuid& node_id, NodeUnit unit)
    {
        if (auto* set = UnitSet(unit)) {
            set->insert(node_id);
        }
    }

    virtual void RegisterPath(Node* node);
    virtual void DeletePath(Node* node, Node* parent_node);
    virtual void HandleNode();

    virtual QSet<QUuid> UpdateAncestorTotal(Node* node, double initial_delta, double final_delta);
    virtual QSet<QUuid>* UnitSet(NodeUnit unit)
    {
        Q_UNUSED(unit)
        return nullptr;
    }

protected:
    Node* root_ {};
    NodeHash node_hash_ {};

    QHash<QUuid, QString> leaf_path_ {};
    QHash<QUuid, QString> branch_path_ {};

    ItemModel* leaf_path_model_ {};
    CString& separator_;

    const Section section_ {};
    const QStringList& node_header_ {};

    QHash<QUuid, QTimer*> pending_timers_ {};
    QHash<QUuid, QJsonObject> pending_updates_ {};
};

using CTreeModel = const TreeModel;

#endif // TREEMODEL_H
