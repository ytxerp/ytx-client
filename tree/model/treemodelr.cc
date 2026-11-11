#include "treemodelr.h"

#include "global/nodepool.h"
#include "utils/nodeutils.h"
#include "utils/pathutils.h"

TreeModelR::TreeModelR(Section section, const QStringList& header, CString& separator, QObject* parent)
    : TreeModel(section, header, separator, parent)
{
    leaf_model_ = new ItemModel(this);
}

QVariant TreeModelR::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    const NodeEnumR column { index.column() };
    auto* d_node { static_cast<NodeR*>(index.internalPointer()) };

    switch (column) {
    case NodeEnumR::kName:
        return d_node->name;
    case NodeEnumR::kIssuedTime:
        return d_node->issued_time;
    case NodeEnumR::kStatus:
        return std::to_underlying(d_node->status);
    case NodeEnumR::kCode:
        return d_node->code;
    case NodeEnumR::kDescription:
        return d_node->description;
    case NodeEnumR::kTag:
        return d_node->tag;
    case NodeEnumR::kColor:
        return d_node->color;
    case NodeEnumR::kDocument:
        return d_node->document;
    case NodeEnumR::kInitialTotal:
        return d_node->initial_total;
    }

    return QVariant();
}

bool TreeModelR::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    auto* node { static_cast<Node*>(index.internalPointer()) };

    const QUuid id { node->id };
    auto& update { pending_updates_[id] };
    update.node = node;
    auto& changes { update.changes };

    const NodeEnumR column { index.column() };

    switch (column) {
    case NodeEnumR::kTag:
        node::UpdateStringList(changes, node, kTag, value.toStringList(), &Node::tag, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumR::kColor:
        node::UpdateField(changes, node, kColor, value.toString(), &Node::color, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumR::kDocument:
        node::UpdateStringList(changes, node, kDocument, value.toStringList(), &Node::document, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumR::kDescription:
        node::UpdateField(changes, node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumR::kName:
    case NodeEnumR::kIssuedTime:
    case NodeEnumR::kStatus:
    case NodeEnumR::kCode:
    case NodeEnumR::kInitialTotal:
        return false;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    return true;
}

void TreeModelR::sort(int column, Qt::SortOrder order)
{
    const NodeEnumR e_column { column };

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeR>(lhs);
        auto* d_rhs = DerivedPtr<NodeR>(rhs);

        switch (e_column) {
        case NodeEnumR::kName:
            return utils::CompareString(lhs->name, rhs->name, order);
        case NodeEnumR::kCode:
            return utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumR::kDescription:
            return utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumR::kIssuedTime:
            return utils::CompareMember(d_lhs, d_rhs, &NodeR::issued_time, order);
        case NodeEnumR::kStatus:
            return utils::CompareMember(d_lhs, d_rhs, &NodeR::reimbursement_status, order);
        case NodeEnumR::kInitialTotal:
            return utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumR::kTag:
            return utils::CompareMember(lhs, rhs, &Node::tag, order);
        case NodeEnumR::kColor:
            return utils::CompareMember(lhs, rhs, &Node::color, order);
        case NodeEnumR::kDocument:
            return utils::CompareMember(lhs, rhs, &Node::document, order);
        }
    };

    emit layoutAboutToBeChanged();
    node::SortSubtree(root_, Compare);
    emit layoutChanged();
}

Qt::ItemFlags TreeModelR::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };

    const NodeEnumR column { index.column() };
    switch (column) {
    case NodeEnumR::kName:
        flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        break;
    case NodeEnumR::kDescription:
        flags |= Qt::ItemIsEditable;
        break;
    case NodeEnumR::kCode:
    case NodeEnumR::kIssuedTime:
    case NodeEnumR::kStatus:
    case NodeEnumR::kTag:
    case NodeEnumR::kColor:
    case NodeEnumR::kDocument:
    case NodeEnumR::kInitialTotal:
        flags &= ~Qt::ItemIsEditable;
        break;
    }

    return flags;
}

bool TreeModelR::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild)
{
    if (sourceParent == destinationParent) {
        qWarning() << "moveRows: same parent move is not supported";
        return false;
    }

    auto* source_parent { GetNodeByIndex(sourceParent) };
    auto* destination_parent { GetNodeByIndex(destinationParent) };

    Q_ASSERT_X(source_parent, "TreeModel::moveRows", "Source parent is null");
    Q_ASSERT_X(destination_parent, "TreeModel::moveRows", "Destination parent is null");
    Q_ASSERT_X(count == 1, "TreeModel::moveRows", "Only single-row move is supported");
    Q_ASSERT_X(sourceRow >= 0 && sourceRow < source_parent->children.size(), "TreeModel::moveRows", "Source row is out of bounds");
    Q_ASSERT_X(destinationChild >= 0 && destinationChild <= destination_parent->children.size(), "TreeModel::moveRows", "Destination child is out of bounds");

    QSet<QUuid> ids_source {};
    QSet<QUuid> ids_destination {};

    if (!beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent, destinationChild)) {
        qWarning() << "moveRows: beginMoveRows failed - invalid move operation";
        return false;
    }

    auto* node { DerivedPtr<NodeR>(source_parent->children.takeAt(sourceRow)) };
    Q_ASSERT(node);

    const bool update_ancestor { node->kind == NodeKind::kBranch || node->reimbursement_status != ReimbursementStatus::kDraft };

    if (update_ancestor) {
        const node::Delta delta {
            .initial = -node->initial_total,
        };

        ids_source = UpdateAncestorTotal(node, delta);
    }

    destination_parent->children.insert(destinationChild, node);
    node->parent = destination_parent;

    if (update_ancestor) {
        const node::Delta delta {
            .initial = node->initial_total,
        };

        ids_destination = UpdateAncestorTotal(node, delta);
    }

    endMoveRows();

    EmitNumericChanged(ids_destination.unite(ids_source));

    return true;
}

void TreeModelR::SyncTree(const QJsonObject& data)
{
    QHash<QUuid, Node*> new_hash {};
    QHash<QUuid, QString> new_leaf_path {};
    QHash<QUuid, QString> new_branch_path {};

    BuildTreeData(data, new_hash, new_leaf_path, new_branch_path);

    beginResetModel();

    ClearTree();

    node_hash_ = std::move(new_hash);
    branch_path_ = std::move(new_branch_path);

    path::AttachRootNodes(node_hash_, root_);
    node::SortSubtree(root_, [](const Node* lhs, const Node* rhs) { return utils::CompareString(lhs->name, rhs->name, Qt::AscendingOrder); });

    endResetModel();
}

void TreeModelR::ApplyName(QUuid node_id, const QString& name, int version)
{
    const auto index { GetIndex(node_id) };
    if (!index.isValid())
        return;

    auto* node { static_cast<Node*>(index.internalPointer()) };
    if (!node)
        return;

    if (node->kind != NodeKind::kBranch)
        return;

    node->name = name;
    node->version = version;

    UpdateSubtreePath(node);

    const int column { std::to_underlying(NodeEnumO::kName) };
    const int row { index.row() };

    EmitDataChanged(row, row, column, column, index.parent());
}

void TreeModelR::HandleStatusChanged(QUuid node_id, ReimbursementStatus value)
{
    auto* d_node { DerivedPtr<NodeR>(node_hash_.value(node_id)) };
    if (!d_node)
        return;

    const int coefficient { value == ReimbursementStatus::kDraft ? -1 : 1 };

    const node::Delta delta {
        .initial = coefficient * d_node->initial_total,
    };

    const auto ids { UpdateAncestorTotal(d_node, delta) };

    EmitNumericChanged(ids);
}

void TreeModelR::RegisterNode(Node* node)
{
    // NOTE: In this section, only branch nodes have a
    // meaningful hierarchical path. Leaf nodes are displayed by
    // their user name rather than a path, so there is no leaf_path_
    // entry to register here — unlike TreeModel::RegisterPath, which
    // handles both branch and leaf kinds.

    const NodeKind kind { node->kind };

    switch (kind) {
    case NodeKind::kBranch:
        branch_path_.insert(node->id, path::Build(node, root_, separator_));
        break;

    case NodeKind::kLeaf: {
        auto* d_node { DerivedPtr<NodeR>(node) };

        if (d_node->reimbursement_status != ReimbursementStatus::kDraft) {
            const node::Delta delta {
                .initial = d_node->initial_total,
            };

            const auto affected_ids { UpdateAncestorTotal(node, delta) };
            EmitNumericChanged(affected_ids);
        }

        break;
    }
    }
}

void TreeModelR::UnregisterNode(Node* node, Node* parent_node)
{
    const auto node_id { node->id };
    auto* d_node { DerivedPtr<NodeR>(node) };
    const NodeKind kind { d_node->kind };

    switch (kind) {
    case NodeKind::kBranch:
        for (auto* child : std::as_const(node->children)) {
            child->parent = parent_node;
            parent_node->children.emplace_back(child);
        }

        UpdateSubtreePath(node);
        branch_path_.remove(node_id);
        break;
    case NodeKind::kLeaf:
        emit SFreeWidget(section_, node_id);
        break;
    }
}

void TreeModelR::UpdateSubtreePath(const Node* node)
{
    QQueue<const Node*> queue {};
    queue.enqueue(node);

    while (!queue.isEmpty()) {
        const auto* current { queue.dequeue() };
        const NodeKind kind { current->kind };

        if (kind == NodeKind::kLeaf)
            continue;

        for (const auto* child : current->children)
            queue.enqueue(child);

        const auto path { path::Build(current, root_, separator_) };
        branch_path_.insert(current->id, path);
    }
}

void TreeModelR::InitTreeData(const QHash<QUuid, Node*>& node_hash, QHash<QUuid, QString>& /*leaf_path*/, QHash<QUuid, QString>& branch_path)
{
    for (auto* node : node_hash) {
        switch (node->kind) {
        case NodeKind::kBranch:
            branch_path.insert(node->id, path::Build(node, separator_));
            break;
        case NodeKind::kLeaf: {
            auto* d_node { DerivedPtr<NodeR>(node) };
            if (d_node->reimbursement_status != ReimbursementStatus::kDraft) {
                const node::Delta delta {
                    .initial = d_node->initial_total,
                };

                InitAncestorTotal(node, delta);
            }

            break;
        }
        }
    }
}

void TreeModelR::ClearTree()
{
    NodePool::Instance().Recycle(node_hash_, section_);

    root_->children.clear();
    branch_path_.clear();
}

QSet<QUuid> TreeModelR::UpdateAncestorTotal(Node* node, const node::Delta& delta) const
{
    QSet<QUuid> affected_ids {};

    if (!node || node == root_)
        return affected_ids;

    if (!node->parent || node->parent == root_)
        return affected_ids;

    if (delta.IsNull())
        return affected_ids;

    for (Node* current = node->parent; current && current != root_; current = current->parent) {
        auto* d_node { DerivedPtr<NodeR>(current) };

        d_node->initial_total += delta.initial;
        affected_ids.insert(current->id);
    }

    return affected_ids;
}

void TreeModelR::InitAncestorTotal(Node* node, const node::Delta& delta) const
{
    if (!node || !node->parent)
        return;

    if (delta.IsNull())
        return;

    for (Node* current = node->parent; current; current = current->parent) {
        auto* d_node { DerivedPtr<NodeR>(current) };

        d_node->initial_total += delta.initial;
    }
}
