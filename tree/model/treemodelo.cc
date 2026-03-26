#include "treemodelo.h"

#include <QJsonArray>

#include "global/nodepool.h"
#include "websocket/jsongen.h"

TreeModelO::TreeModelO(CSectionInfo& info, CString& separator, QObject* parent)
    : TreeModel(info, separator, parent)
{
}

void TreeModelO::RNodeStatus(const QUuid& node_id, NodeStatus value)
{
    auto* d_node { DerivedPtr<NodeO>(node_hash_.value(node_id)) };
    if (!d_node)
        return;

    const int coefficient { value == NodeStatus::kReleased ? 1 : -1 };

    const auto affected_ids { UpdateAncestorTotal(d_node, coefficient * d_node->initial_total, coefficient * d_node->final_total,
        coefficient * d_node->count_total, coefficient * d_node->measure_total, coefficient * d_node->discount_total) };

    RefreshAffectedTotal(affected_ids);
}

void TreeModelO::AckTree(const QJsonObject& obj)
{
    const QJsonArray node_array { obj.value(kNodeArray).toArray() };
    const QJsonArray path_array { obj.value(kPathArray).toArray() };

    beginResetModel();

    {
        NodePool::Instance().Recycle(node_hash_, section_);
        root_->children.clear();
    }

    for (const QJsonValue& val : node_array) {
        const QJsonObject obj { val.toObject() };
        Node* node { NodePool::Instance().Allocate(section_) };
        node->ReadJson(obj);
        node_hash_.insert(node->id, node);
    }

    BuildHierarchy(path_array);
    HandleNode();

    sort(std::to_underlying(NodeEnumO::kName), Qt::AscendingOrder);
    endResetModel();
}

void TreeModelO::AckNode(const QJsonObject& leaf_obj, const QUuid& ancestor_id)
{
    auto* node = NodePool::Instance().Allocate(section_);
    node->ReadJson(leaf_obj);

    Node* ancestor { ancestor_id.isNull() ? root_ : node_hash_.value(ancestor_id) };

    Q_ASSERT(ancestor);

    const long long row { ancestor->children.size() };
    const auto parent { GetIndex(ancestor_id) };

    beginInsertRows(parent, row, row);
    ancestor->children.insert(row, node);
    node->parent = ancestor;
    endInsertRows();

    {
        auto* d_node { DerivedPtr<NodeO>(node) };

        if (d_node->kind == NodeKind::kLeaf && d_node->status == NodeStatus::kReleased) {
            auto ids { UpdateAncestorTotal(
                node, d_node->initial_total, d_node->final_total, d_node->count_total, d_node->measure_total, d_node->discount_total) };

            ids.remove(node->id);
            RefreshAffectedTotal(ids);
        }
    }

    node_hash_.insert(node->id, node);
}

void TreeModelO::UpdateName(const QUuid& node_id, const QString& name)
{
    auto* node = GetNode(node_id);
    if (!node)
        return;

    const auto index { GetIndex(node_id) };
    if (!index.isValid())
        return;

    node->name = name;

    const int column { std::to_underlying(NodeEnumO::kName) };
    const int row { index.row() };

    EmitDataChanged(row, row, column, column, index.parent());
}

void TreeModelO::InsertSettlement(const QSet<QUuid>& settled_set, const QUuid& settlement_id)
{
    if (settled_set.isEmpty() || settlement_id.isNull())
        return;

    for (auto it = node_hash_.constBegin(); it != node_hash_.constEnd(); ++it) {
        auto* node = it.value();
        Q_ASSERT(node != nullptr);

        if (!settled_set.contains(node->id))
            continue;

        auto* d_node = static_cast<NodeO*>(node);
        Q_ASSERT(d_node != nullptr);

        d_node->is_settled = true;
        d_node->settlement_id = settlement_id;
        d_node->final_total = d_node->initial_total - d_node->discount_total;
        d_node->version += 1;

        const auto index { GetIndex(node->id) };
        if (!index.isValid())
            continue;

        const int column { std::to_underlying(NodeEnumO::kFinalTotal) };
        const int row { index.row() };

        EmitDataChanged(row, row, column, column, index.parent());
    }
}

void TreeModelO::RecallSettlement(const QUuid& settlement_id)
{
    if (settlement_id.isNull())
        return;

    for (auto it = node_hash_.constBegin(); it != node_hash_.constEnd(); ++it) {
        auto* node = it.value();
        Q_ASSERT(node != nullptr);

        auto* d_node = static_cast<NodeO*>(node);
        Q_ASSERT(d_node != nullptr);

        if (d_node->settlement_id != settlement_id)
            continue;

        d_node->is_settled = false;
        d_node->settlement_id = QUuid();
        d_node->final_total = {};
        d_node->version += 1;

        const auto index { GetIndex(d_node->id) };
        if (!index.isValid())
            continue;

        const int column { std::to_underlying(NodeEnumO::kFinalTotal) };
        const int row { index.row() };

        EmitDataChanged(row, row, column, column, index.parent());
    }
}

void TreeModelO::DeletePath(Node* node, Node* parent_node)
{
    auto* d_node { DerivedPtr<NodeO>(node) };
    const NodeKind kind { d_node->kind };

    switch (kind) {
    case NodeKind::kBranch:
        for (auto* child : std::as_const(node->children)) {
            child->parent = parent_node;
            parent_node->children.emplace_back(child);
        }
        break;
    case NodeKind::kLeaf:
        if (d_node->status == NodeStatus::kReleased) {
            UpdateAncestorTotal(node, -d_node->initial_total, -d_node->final_total, -d_node->count_total, -d_node->measure_total, -d_node->discount_total);
        }
        break;
    default:
        break;
    }
}

QSet<QUuid> TreeModelO::UpdateAncestorTotal(
    Node* node, double initial_delta, double final_delta, double count_delta, double measure_delta, double discount_delta) const
{
    QSet<QUuid> affected_ids {};

    if (!node || node == root_)
        return affected_ids;

    affected_ids.insert(node->id);

    if (!node->parent || node->parent == root_)
        return affected_ids;

    if (FloatEqual(initial_delta, 0.0) && FloatEqual(final_delta, 0.0) && FloatEqual(count_delta, 0.0) && FloatEqual(measure_delta, 0.0)
        && FloatEqual(discount_delta, 0.0))
        return affected_ids;

    const auto unit { node->unit };

    for (Node* current = node->parent; current && current != root_; current = current->parent) {
        if (current->unit != unit)
            continue;

        auto* d_node { DerivedPtr<NodeO>(current) };

        d_node->count_total += count_delta;
        d_node->measure_total += measure_delta;
        d_node->discount_total += discount_delta;
        d_node->initial_total += initial_delta;
        d_node->final_total += final_delta;

        affected_ids.insert(current->id);
    }

    return affected_ids;
}

void TreeModelO::InitAncestorTotal(Node* node, double initial_delta, double final_delta, double count_delta, double measure_delta, double discount_delta) const
{
    if (!node || node == root_ || !node->parent || node->parent == root_)
        return;

    if (FloatEqual(initial_delta, 0.0) && FloatEqual(final_delta, 0.0) && FloatEqual(count_delta, 0.0) && FloatEqual(measure_delta, 0.0)
        && FloatEqual(discount_delta, 0.0))
        return;

    const auto unit { node->unit };

    for (Node* current = node->parent; current && current != root_; current = current->parent) {
        if (current->unit != unit)
            continue;

        auto* d_node { DerivedPtr<NodeO>(current) };

        d_node->count_total += count_delta;
        d_node->measure_total += measure_delta;
        d_node->discount_total += discount_delta;
        d_node->initial_total += initial_delta;
        d_node->final_total += final_delta;
    }
}

void TreeModelO::HandleNode()
{
    for (auto* node : std::as_const(node_hash_)) {
        auto* d_node { DerivedPtr<NodeO>(node) };

        if (d_node->kind == NodeKind::kLeaf && d_node->status == NodeStatus::kReleased)
            InitAncestorTotal(node, d_node->initial_total, d_node->final_total, d_node->count_total, d_node->measure_total, d_node->discount_total);
    }
}

void TreeModelO::sort(int column, Qt::SortOrder order)
{
    const NodeEnumO e_column { column };

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeO>(lhs);
        auto* d_rhs = DerivedPtr<NodeO>(rhs);

        switch (e_column) {
        case NodeEnumO::kName:
            return Utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumO::kCode:
            return Utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumO::kDescription:
            return Utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumO::kDirectionRule:
            return Utils::CompareMember(lhs, rhs, &Node::direction_rule, order);
        case NodeEnumO::kKind:
            return Utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumO::kUnit:
            return Utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumO::kPartnerId:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::partner_id, order);
        case NodeEnumO::kEmployeeId:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::employee_id, order);
        case NodeEnumO::kIssuedTime:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::issued_time, order);
        case NodeEnumO::kCountTotal:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::count_total, order);
        case NodeEnumO::kMeasureTotal:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::measure_total, order);
        case NodeEnumO::kDiscountTotal:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::discount_total, order);
        case NodeEnumO::kStatus:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::status, order);
        case NodeEnumO::kInitialTotal:
            return Utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumO::kFinalTotal:
            return Utils::CompareMember(lhs, rhs, &Node::final_total, order);
        case NodeEnumO::kId:
        case NodeEnumO::kUpdateBy:
        case NodeEnumO::kUpdateTime:
        case NodeEnumO::kCreateTime:
        case NodeEnumO::kCreateBy:
        case NodeEnumO::kVersion:
        case NodeEnumO::kUserId:
        case NodeEnumO::kIsSettled:
        case NodeEnumO::kSettlementId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    Utils::SortIterative(root_, Compare);
    emit layoutChanged();
}

QVariant TreeModelO::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* d_node { static_cast<NodeO*>(index.internalPointer()) };
    Q_ASSERT(d_node != nullptr);

    const NodeEnumO column { index.column() };
    const bool is_branch { d_node->kind == NodeKind::kBranch };

    switch (column) {
    case NodeEnumO::kName:
        return d_node->name;
    case NodeEnumO::kId:
        return d_node->id;
    case NodeEnumO::kUserId:
        return d_node->user_id;
    case NodeEnumO::kCreateTime:
        return d_node->created_time;
    case NodeEnumO::kCreateBy:
        return d_node->created_by;
    case NodeEnumO::kUpdateTime:
        return d_node->updated_time;
    case NodeEnumO::kUpdateBy:
        return d_node->updated_by;
    case NodeEnumO::kCode:
        return d_node->code;
    case NodeEnumO::kVersion:
        return d_node->version;
    case NodeEnumO::kDescription:
        return d_node->description;
    case NodeEnumO::kDirectionRule:
        return d_node->direction_rule;
    case NodeEnumO::kKind:
        return std::to_underlying(d_node->kind);
    case NodeEnumO::kUnit:
        return std::to_underlying(d_node->unit);
    case NodeEnumO::kPartnerId:
        return d_node->partner_id;
    case NodeEnumO::kEmployeeId:
        return d_node->employee_id;
    case NodeEnumO::kIssuedTime:
        return is_branch ? QVariant() : d_node->issued_time;
    case NodeEnumO::kCountTotal:
        return d_node->count_total;
    case NodeEnumO::kMeasureTotal:
        return d_node->measure_total;
    case NodeEnumO::kDiscountTotal:
        return d_node->discount_total;
    case NodeEnumO::kStatus:
        return std::to_underlying(d_node->status);
    case NodeEnumO::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumO::kFinalTotal:
        return d_node->final_total;
    case NodeEnumO::kIsSettled:
        return d_node->is_settled;
    case NodeEnumO::kSettlementId:
        return d_node->settlement_id;
    default:
        return QVariant();
    }
}

Qt::ItemFlags TreeModelO::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };

    const NodeEnumO column { index.column() };
    switch (column) {
    case NodeEnumO::kName:
        flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        break;
    default:
        flags &= ~Qt::ItemIsEditable;
        break;
    }

    return flags;
}

bool TreeModelO::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild)
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

    QSet<QUuid> affected_ids_source {};
    QSet<QUuid> affected_ids_destination {};

    if (!beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent, destinationChild)) {
        qWarning() << "moveRows: beginMoveRows failed - invalid move operation";
        return false;
    }

    auto* node { DerivedPtr<NodeO>(source_parent->children.takeAt(sourceRow)) };
    Q_ASSERT(node);

    const bool update_ancestor { node->kind == NodeKind::kBranch || node->status == NodeStatus::kReleased };

    if (update_ancestor) {
        affected_ids_source
            = UpdateAncestorTotal(node, -node->initial_total, -node->final_total, -node->count_total, -node->measure_total, -node->discount_total);
    }

    destination_parent->children.insert(destinationChild, node);
    node->parent = destination_parent;

    if (update_ancestor) {
        affected_ids_destination
            = UpdateAncestorTotal(node, node->initial_total, node->final_total, node->count_total, node->measure_total, node->discount_total);
    }

    endMoveRows();

    RefreshAffectedTotal(affected_ids_destination.unite(affected_ids_source));

    return true;
}
