#include "treemodelo.h"

#include <QJsonArray>

#include "global/nodepool.h"
#include "utils/compareutils.h"
#include "websocket/jsongen.h"

TreeModelO::TreeModelO(CSectionInfo& info, CString& separator, QObject* parent)
    : TreeModel(info, separator, parent)
{
}

void TreeModelO::RNodeStatus(const QUuid& node_id, NodeStatus value)
{
    auto* d_node { DerivedPtr<NodeO>(node_hash_.value(node_id)) };
    assert(d_node);

    const int coefficient { value == NodeStatus::kReleased ? 1 : -1 };

    const auto& affected_ids { UpdateAncestorTotalOrder(d_node, coefficient * d_node->initial_total, coefficient * d_node->final_total,
        coefficient * d_node->count_total, coefficient * d_node->measure_total, coefficient * d_node->discount_total) };

    if (d_node->unit == NodeUnit::OMonthly && FloatChanged(d_node->initial_total, 0.0))
        emit SUpdateAmount(d_node->partner_id, coefficient * d_node->initial_total);

    RefreshAffectedTotal(affected_ids);
}

void TreeModelO::AckTree(const QJsonObject& obj)
{
    const QJsonArray node_array { obj.value(kNodeArray).toArray() };
    const QJsonArray path_array { obj.value(kPathArray).toArray() };

    beginResetModel();
    ClearModel();

    for (const QJsonValue& val : node_array) {
        const QJsonObject obj { val.toObject() };

        const QUuid id { QUuid(obj.value(kId).toString()) };

        assert(!node_hash_.contains(id));

        Node* node { NodePool::Instance().Allocate(section_) };
        node->ReadJson(obj);
        node_hash_.insert(id, node);
    }

    for (const QJsonValue& val : path_array) {
        const QJsonObject obj { val.toObject() };

        const QUuid ancestor_id { QUuid(obj.value(kAncestor).toString()) };
        const QUuid descendant_id { QUuid(obj.value(kDescendant).toString()) };

        Node* ancestor { node_hash_.value(ancestor_id) };
        Node* descendant { node_hash_.value(descendant_id) };

        assert((ancestor) && "Ancestor not found in node_model_");
        assert((descendant) && "Descendant not found in node_model_");

        descendant->parent = ancestor;
    }

    for (auto* node : std::as_const(node_hash_)) {
        if (node != root_) {
            node->parent->children.emplaceBack(node);
        }
    }

    if (node_hash_.size() >= 2) {
        HandleNode();
    }

    sort(std::to_underlying(NodeEnumO::kName), Qt::AscendingOrder);
    endResetModel();
}

void TreeModelO::AckNode(const QJsonObject& leaf_obj, const QUuid& ancestor_id)
{
    if (!node_hash_.contains(ancestor_id)) {
        qCritical() << "AckNode: ancestor_id not found in node_hash_:" << ancestor_id;
    }
    assert(node_hash_.contains(ancestor_id));

    auto* node = NodePool::Instance().Allocate(section_);
    node->ReadJson(leaf_obj);

    Node* ancestor { node_hash_.value(ancestor_id) };

    const long long row { ancestor->children.size() };
    const auto parent { GetIndex(ancestor_id) };

    beginInsertRows(parent, row, row);
    ancestor->children.insert(row, node);
    node->parent = ancestor;
    endInsertRows();

    node_hash_.insert(node->id, node);
}

void TreeModelO::UpdateName(const QUuid& node_id, const QString& name)
{
    auto* node = GetNode(node_id);
    if (!node)
        return;

    node->name = name;

    const int name_column { std::to_underlying(NodeEnumO::kName) };

    EmitRowChanged(node_id, name_column, name_column);
    emit SResizeColumnToContents(name_column);
}

void TreeModelO::InsertSettlement(const QUuid& node_id, const QUuid& settlement_id)
{
    auto* node = GetNode(node_id);
    if (node) {
        auto* d_node { static_cast<NodeO*>(node) };
        d_node->is_settled = true;
        d_node->settlement_id = settlement_id;
    }
}

void TreeModelO::RecallSettlement(const QUuid& settlement_id)
{
    for (auto it = node_hash_.begin(); it != node_hash_.end(); ++it) {
        auto* node = it.value();
        if (!node)
            continue;

        auto* d_node = static_cast<NodeO*>(node);
        if (d_node->settlement_id == settlement_id) {
            d_node->is_settled = false;
        }
    }
}

void TreeModelO::DeleteSettlement(const QUuid& settlement_id)
{
    for (auto it = node_hash_.begin(); it != node_hash_.end(); ++it) {
        auto* node = it.value();
        if (!node)
            continue;

        auto* d_node = static_cast<NodeO*>(node);
        if (d_node->settlement_id == settlement_id) {
            d_node->is_settled = false;
            d_node->settlement_id = QUuid();
        }
    }
}

void TreeModelO::RemovePath(Node* node, Node* parent_node)
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
            UpdateAncestorTotalOrder(node, -d_node->initial_total, -d_node->final_total, -d_node->count_total, -d_node->measure_total, -d_node->discount_total);

            if (node->unit == NodeUnit::OMonthly && FloatChanged(-node->initial_total, 0.0))
                emit SUpdateAmount(d_node->partner_id, -node->initial_total);
        }
        break;
    default:
        break;
    }
}

QSet<QUuid> TreeModelO::UpdateAncestorTotalOrder(
    Node* node, double initial_delta, double final_delta, double count_delta, double measure_delta, double discount_delta)
{
    assert(node && node != root_ && node->parent);
    QSet<QUuid> affected_ids {};

    if (node->parent == root_)
        return affected_ids;

    if (initial_delta == 0.0 && final_delta == 0.0 && count_delta == 0.0 && measure_delta == 0.0 && discount_delta == 0.0)
        return affected_ids;

    const auto kUnit { node->unit };

    for (Node* current = node->parent; current && current != root_; current = current->parent) {
        if (current->unit != kUnit)
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

void TreeModelO::HandleNode()
{
    for (auto* node : std::as_const(node_hash_)) {
        auto* d_node { DerivedPtr<NodeO>(node) };

        if (d_node->kind == NodeKind::kLeaf && d_node->status == NodeStatus::kReleased)
            UpdateAncestorTotalOrder(node, d_node->initial_total, d_node->final_total, d_node->count_total, d_node->measure_total, d_node->discount_total);
    }
}

void TreeModelO::ResetBranch(Node* node)
{
    assert(node->kind == NodeKind::kBranch && "ResetBranch: node must be of kind NodeKind::kBranch");

    auto* d_node { DerivedPtr<NodeO>(node) };
    d_node->count_total = 0.0;
    d_node->measure_total = 0.0;
    d_node->initial_total = 0.0;
    d_node->discount_total = 0.0;
    d_node->final_total = 0.0;
    d_node->children.clear();
}

void TreeModelO::ClearModel()
{
    // Clear non-branch nodes from node_hash_, keep branch nodes and unfinidhws nodes
    for (auto it = node_hash_.begin(); it != node_hash_.end();) {
        auto* node = static_cast<NodeO*>(it.value());

        if (node->kind == NodeKind::kBranch) {
            ResetBranch(node);
            ++it;
            continue;
        }

        if (node->status == NodeStatus::kRecalled) {
            ++it;
            continue;
        }

        if (node->unit == NodeUnit::OPending) {
            ++it;
            continue;
        }

        NodePool::Instance().Recycle(node, section_);
        it = node_hash_.erase(it);
    }
}

void TreeModelO::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column < node_header_.size());

    const NodeEnumO e_column { column };

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeO>(lhs);
        auto* d_rhs = DerivedPtr<NodeO>(rhs);

        switch (e_column) {
        case NodeEnumO::kName:
            return Utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumO::kDescription:
            return Utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumO::kDirectionRule:
            return Utils::CompareMember(lhs, rhs, &Node::direction_rule, order);
        case NodeEnumO::kKind:
            return Utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumO::kUnit:
            return Utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumO::kPartner:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::partner_id, order);
        case NodeEnumO::kEmployee:
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
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_node { DerivedPtr<NodeO>(GetNodeByIndex(index)) };
    if (!d_node)
        return false;

    const NodeEnumO column { index.column() };
    bool branch { d_node->kind == NodeKind::kBranch };

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
    case NodeEnumO::kVersion:
        return d_node->version;
    case NodeEnumO::kDescription:
        return d_node->description;
    case NodeEnumO::kDirectionRule:
        return branch ? QVariant() : d_node->direction_rule;
    case NodeEnumO::kKind:
        return std::to_underlying(d_node->kind);
    case NodeEnumO::kUnit:
        return std::to_underlying(d_node->unit);
    case NodeEnumO::kPartner:
        return d_node->partner_id;
    case NodeEnumO::kEmployee:
        return d_node->employee_id;
    case NodeEnumO::kIssuedTime:
        return d_node->issued_time;
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

bool TreeModelO::moveRows(const QModelIndex& sourceParent, int sourceRow, int /*count*/, const QModelIndex& destinationParent, int destinationChild)
{
    auto* source_parent { GetNodeByIndex(sourceParent) };
    auto* destination_parent { GetNodeByIndex(destinationParent) };

    assert(source_parent);
    assert(destination_parent);
    assert(sourceRow >= 0 && sourceRow < source_parent->children.size());

    beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent, destinationChild);
    auto* node { DerivedPtr<NodeO>(source_parent->children.takeAt(sourceRow)) };
    assert(node);

    bool update_ancestor { node->kind == NodeKind::kBranch || node->status == NodeStatus::kReleased };

    if (update_ancestor) {
        UpdateAncestorTotalOrder(node, -node->initial_total, -node->final_total, -node->count_total, -node->measure_total, -node->discount_total);
    }

    destination_parent->children.insert(destinationChild, node);
    node->parent = destination_parent;

    if (update_ancestor) {
        UpdateAncestorTotalOrder(node, node->initial_total, node->final_total, node->count_total, node->measure_total, node->discount_total);
    }

    endMoveRows();

    emit SResizeColumnToContents(std::to_underlying(NodeEnum::kName));

    return true;
}
