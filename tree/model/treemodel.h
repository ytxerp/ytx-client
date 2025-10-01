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
#include "component/enumclass.h"
#include "component/info.h"
#include "utils/castutils.h"
#include "utils/nodeutils.h"

using CastUtils::DerivedPtr;
using NodeUtils::SortIterative;

class TreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    virtual ~TreeModel() = default;

protected:
    explicit TreeModel(CSectionInfo& info, CString& separator, int default_unit, QObject* parent = nullptr);

signals:
    // send to LeafSStation
    void SSyncRule(const QUuid& node_id, bool rule);

    // send to its view
    void SResizeColumnToContents(int column);

    // send to Mainwindow
    void SUpdateName(const QUuid& node_id, const QString& name, bool branch);
    void SFreeWidget(const QUuid& node_id);

    // send to NodeWidget
    void SSyncStatusValue();

    // send to FilterModel
    // Inserting or removing a node will trigger it automatically; manually trigger it only when changing the unit.
    void SSyncFilterModel();

public slots:
    // receive from RemoveDialog
    void RRemoveNode(const QUuid& node_id);

    // receive from EntryModel
    virtual void RSyncDelta(
        const QUuid& node_id, double initial_delta, double final_delta, double first_delta = 0.0, double second_delta = 0.0, double discount_delta = 0.0);

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
    void AckOneNode(const QJsonObject& leaf_obj, const QUuid& ancestor_id);

    void ApplyNodeInsert(const QUuid& ancestor, const QJsonObject& data);
    void ApplyMetaInsert(const QUuid& node_id, const QJsonObject& data);

    void ApplyNodeUpdate(const QUuid& node_id, const QJsonObject& data);
    void ApplyMetaUpdate(const QUuid& node_id, const QJsonObject& data);

    void ApplyLeafReplace(const QUuid& old_node_id, const QUuid& new_node_id);
    void ApplyNodeDrag(const QUuid& ancestor, const QUuid& descendant, const QJsonObject& data);

    void ApplyDirectionRule(const QUuid& node_id, bool direction_rule, const QJsonObject& meta);
    void ApplyName(const QUuid& node_id, const QJsonObject& data);
    void ApplyDelta(const QJsonObject& data);

    // Ytx's
    // Default implementations
    double InitialTotal(QUuid node_id) const { return NodeUtils::Value(node_hash_, node_id, &Node::initial_total); }
    double FinalTotal(QUuid node_id) const { return NodeUtils::Value(node_hash_, node_id, &Node::final_total); }
    int Kind(QUuid node_id) { return NodeUtils::Value(node_hash_, node_id, &Node::kind); }
    int Unit(QUuid node_id) const { return NodeUtils::Value(node_hash_, node_id, &Node::unit); }
    bool Rule(QUuid node_id) const { return NodeUtils::Value(node_hash_, node_id, &Node::direction_rule); }
    QString Name(QUuid node_id) const { return NodeUtils::Value(node_hash_, node_id, &Node::name); }

    QStringList ChildrenName(const QUuid& parent_id) const;
    QSet<QUuid> ChildrenId(const QUuid& parent_id) const;

    inline ItemModel* LeafModel() const { return leaf_model_; }
    inline CUuidString& LeafPath() const { return leaf_path_; }

    void LeafPathBranchPathModel(ItemModel* model) const;
    void SearchNode(QList<const Node*>& node_list, CString& name) const;

    void UpdateSeparator(CString& old_separator, CString& new_separator);
    void UpdateDefaultUnit(int default_unit);

    bool InsertNode(int row, const QModelIndex& parent, Node* node);
    void FetchOneNode(const QUuid& node_id);

    inline bool Contains(const QUuid& node_id) const { return node_hash_.contains(node_id); }
    inline void SetParent(Node* node, const QUuid& parent_id) const
    {
        assert(node);
        assert(node_hash_.contains(parent_id));

        node->parent = node_hash_.value(parent_id);
    }

    QModelIndex GetIndex(const QUuid& node_id) const;
    Node* GetNodeByIndex(const QModelIndex& index) const;
    QSortFilterProxyModel* ExcludeOneModel(const QUuid& node_id);

    // virtual functions
    virtual void UpdateName(const QUuid& node_id, CString& new_name);

    virtual bool Finished(QUuid node_id) const
    {
        Q_UNUSED(node_id);
        return false;
    }

    virtual void ResetColor(const QModelIndex& index) { Q_UNUSED(index); };

    virtual QString Path(const QUuid& node_id) const;

    virtual QSortFilterProxyModel* IncludeUnitModel(int unit)
    {
        Q_UNUSED(unit)
        return nullptr;
    }

    virtual QSortFilterProxyModel* ExcludeMultipleModel(const QUuid& node_id, int unit)
    {
        Q_UNUSED(unit)
        Q_UNUSED(node_id)
        return nullptr;
    }

protected:
    void SortModel();

    void InitRoot(Node*& root, int default_unit);

    void BuildHierarchy(const QJsonArray& path_array);

    void ResetModel();

    void RestartTimer(const QUuid& id);
    void EmitRowChanged(const QUuid& node_id, int start_column, int end_column);

    void UpdateDirectionRule(Node* node, bool value);
    void DirectionRuleImpl(Node* node, bool value);

    void UpdateMeta(Node* node, const QJsonObject& meta);
    void InsertMeta(Node* node, const QJsonObject& meta);

    void InsertImpl(Node* parent, int row, Node* node);

    virtual void InsertPath(Node* node);
    virtual void RemovePath(Node* node, Node* parent_node);
    virtual void RegisterNode(Node* node) { node_hash_.insert(node->id, node); }

    virtual void ResetBranch(Node* node) { Q_UNUSED(node) };

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

    virtual Node* GetNode(const QUuid& node_id) const
    {
        auto* node = node_hash_.value(node_id);
        assert(node);
        return node;
    }

    virtual void HandleNode();
    virtual bool UpdateAncestorValue(
        Node* node, double initial_delta, double final_delta, double first_delta = 0.0, double second_delta = 0.0, double discount_delta = 0.0);

    // Returns the range of numeric columns used for totals
    virtual std::pair<int, int> TotalColumnRange() const
    {
        const int col_count = columnCount();

        if (col_count < 2)
            return { 0, 0 };

        return { col_count - 2, col_count - 1 };
    }

    // Returns the range of cache field columns
    // Note: the positions of these "code", "descriptioon" and "note" are fixed / hard-coded
    virtual std::pair<int, int> CacheColumnRange() const { return { 0, 0 }; }

protected:
    Node* root_ {};
    NodeHash node_hash_ {};

    UuidString leaf_path_ {};
    UuidString branch_path_ {};

    ItemModel* leaf_model_ {};
    CString& separator_;

    const Section section_ {};
    const QString& section_str_ {};
    const QStringList& node_header_ {};

    QHash<QUuid, QTimer*> timers_ {};
    QHash<QUuid, QJsonObject> caches_ {};
};

using CTreeModel = const TreeModel;

#endif // TREEMODEL_H
