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

#include "component/constant.h"
#include "component/info.h"
#include "utils/castutils.h"
#include "utils/nodeutils.h"

using CastUtils::DerivedPtr;
using NodeUtils::SortIterative;

class TreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    virtual ~TreeModel();

protected:
    explicit TreeModel(CSectionInfo& info, CString& separator, int default_unit, QObject* parent = nullptr);

signals:
    // send to LeafSStation
    void SDirectionRule(const QUuid& node_id, bool value);

    // send to its view
    void SResizeColumnToContents(int column);

    // send to Mainwindow
    void SUpdateName(const QUuid& node_id, const QString& name, bool branch);
    void SFreeWidget(Section section, const QUuid& node_id);

    // send to NodeWidget
    void STotalsUpdated();

public slots:
    // receive from RemoveDialog
    void RRemoveNode(const QUuid& node_id);

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

    virtual void AckTree(const QJsonObject& obj) { Q_UNUSED(obj) }
    virtual void UpdateName(const QUuid& node_id, const QString& name);
    virtual void AckNode(const QJsonObject& leaf_obj, const QUuid& ancestor_id)
    {
        Q_UNUSED(leaf_obj)
        Q_UNUSED(ancestor_id)
    }

    void InsertNode(const QUuid& ancestor, const QJsonObject& data);
    void InsertMeta(const QUuid& node_id, const QJsonObject& meta);

    void SyncNode(const QUuid& node_id, const QJsonObject& update);
    void UpdateMeta(const QUuid& node_id, const QJsonObject& meta);

    void ReplaceLeaf(const QUuid& old_node_id, const QUuid& new_node_id);
    void DragNode(const QUuid& ancestor, const QUuid& descendant);

    void SyncDirectionRule(const QUuid& node_id, bool direction_rule);
    void SyncTotalArray(const QJsonArray& total_array);

    // Ytx's
    // Default implementations
    double InitialTotal(QUuid node_id) const { return NodeUtils::Value(node_hash_, node_id, &Node::initial_total); }
    double FinalTotal(QUuid node_id) const { return NodeUtils::Value(node_hash_, node_id, &Node::final_total); }
    int Unit(QUuid node_id) const { return NodeUtils::Value(node_hash_, node_id, &Node::unit); }
    bool Rule(QUuid node_id) const { return NodeUtils::Value(node_hash_, node_id, &Node::direction_rule); }
    QString Name(QUuid node_id) const { return NodeUtils::Value(node_hash_, node_id, &Node::name); }
    QString Path(const QUuid& node_id) const;

    inline ItemModel* LeafModel() const { return leaf_path_model_; }
    inline CUuidString& LeafPath() const { return leaf_path_; }

    void LeafPathBranchPathModel(ItemModel* model) const;

    void UpdateSeparator(CString& old_separator, CString& new_separator);
    void UpdateDefaultUnit(int default_unit);

    void AckNode(const QUuid& node_id) const;
    void SearchNode(QList<Node*>& node_list, CString& name) const;

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
    QSortFilterProxyModel* ExcludeOneModel(const QUuid& node_id, QObject* parent);

    // virtual functions
    virtual void ResetColor(const QModelIndex& index) { Q_UNUSED(index); };

    virtual int Status(QUuid node_id) const
    {
        Q_UNUSED(node_id);
        return false;
    }

    virtual QSortFilterProxyModel* IncludeUnitModel(int unit, QObject* parent)
    {
        Q_UNUSED(unit)
        Q_UNUSED(parent)
        return nullptr;
    }

    virtual QSortFilterProxyModel* ExcludeMultipleModel(const QUuid& node_id, int unit, QObject* parent)
    {
        Q_UNUSED(unit)
        Q_UNUSED(node_id)
        Q_UNUSED(parent)
        return nullptr;
    }

protected:
    void SortModel();

    void InitRoot(Node*& root, int default_unit);

    void BuildHierarchy(const QJsonArray& path_array);

    void RestartTimer(const QUuid& id);
    void FlushCaches();
    void EmitRowChanged(const QUuid& node_id, int start_column, int end_column);

    void UpdateDirectionRule(Node* node, bool value);
    void DirectionRuleImpl(Node* node, bool value);

    void UpdateMeta(Node* node, const QJsonObject& meta);
    void InsertMeta(Node* node, const QJsonObject& meta);

    void RefreshAffectedTotal(const QSet<QUuid>& affected_ids);

    QSet<QUuid> SyncDeltaImpl(const QUuid& node_id, double initial_delta, double final_delta);
    QSet<QUuid> UpdateTotal(const QUuid& node_id, double initial_total, double final_total);
    virtual QSet<QUuid> UpdateAncestorTotal(Node* node, double initial_delta, double final_delta);

    virtual void RegisterPath(Node* node);
    virtual void RemovePath(Node* node, Node* parent_node);

    virtual void ResetBranch(Node* node) { Q_UNUSED(node) };
    virtual void ClearModel() { }
    virtual void UpdateStatus(Node* node, int value)
    {
        Q_UNUSED(node)
        Q_UNUSED(value)
    }
    virtual const QSet<QUuid>* UnitSet(int unit) const
    {
        Q_UNUSED(unit)
        return nullptr;
    }

    virtual void RemoveUnitSet(const QUuid& node_id, int unit)
    {
        Q_UNUSED(node_id)
        Q_UNUSED(unit)
    }

    virtual void InsertUnitSet(const QUuid& node_id, int unit)
    {
        Q_UNUSED(node_id)
        Q_UNUSED(unit)
    }

    virtual void HandleNode();

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
