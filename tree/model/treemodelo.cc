#include "treemodelo.h"

#include <QJsonArray>

#include "global/nodepool.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TreeModelO::TreeModelO(CSectionInfo& info, CString& separator, int default_unit, QObject* parent)
    : TreeModel(info, separator, default_unit, parent)
{
}

void TreeModelO::RNodeStatus(const QUuid& node_id, NodeStatus value)
{
    auto* d_node { DerivedPtr<NodeO>(node_hash_.value(node_id)) };
    assert(d_node);

    const int coefficient { value == NodeStatus::kReleased ? 1 : -1 };

    const auto& affected_ids { UpdateAncestorTotalOrder(d_node, coefficient * d_node->initial_total, coefficient * d_node->final_total,
        coefficient * d_node->count_total, coefficient * d_node->measure_total, coefficient * d_node->discount_total) };

    if (d_node->unit == std::to_underlying(UnitO::kMonthly))
        emit SUpdateAmount(d_node->partner, coefficient * d_node->initial_total, coefficient * d_node->final_total);

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
        if (node->kind == std::to_underlying(NodeKind::kLeaf)) {
            node->parent->children.emplaceBack(node);
        }
    }

    if (node_hash_.size() >= 2) {
        HandleNode();
    }

    sort(std::to_underlying(NodeEnumT::kName), Qt::AscendingOrder);
    endResetModel();
}

void TreeModelO::SyncNodeName(const QUuid& node_id, const QString& name, const QJsonObject& meta)
{
    Q_ASSERT_X(meta.contains(kUpdatedBy), "TreeModel::SyncName", "Missing 'updated_by' in data");
    Q_ASSERT_X(meta.contains(kUpdatedTime), "TreeModel::SyncName", "Missing 'updated_time' in data");

    auto* node = GetNode(node_id);
    if (!node)
        return;

    node->name = name;
    node->updated_by = QUuid(meta[kUpdatedBy].toString());
    node->updated_time = QDateTime::fromString(meta[kUpdatedTime].toString(), Qt::ISODate);

    emit SResizeColumnToContents(std::to_underlying(NodeEnum::kName));

    auto index { GetIndex(node_id) };
    if (index.isValid()) {
        emit dataChanged(index.siblingAtColumn(std::to_underlying(NodeEnum::kName)), index.siblingAtColumn(std::to_underlying(NodeEnum::kName)));
    }
}

bool TreeModelO::InsertNode(int row, const QModelIndex& parent, Node* node)
{
    if (row < 0 || row > rowCount(parent)) {
        qCritical() << "InsertNode: row out of range";
    }
    assert(row >= 0 && row <= rowCount(parent));

    auto* parent_node { GetNodeByIndex(parent) };
    InsertImpl(parent_node, row, node);
    return true;
}

void TreeModelO::SyncNodeDelta(const QUuid& node_id, const QJsonObject& data)
{
    if (data.isEmpty()) {
        qInfo().noquote() << "SyncNodeDelta ignored: empty data for node" << node_id.toString(QUuid::WithoutBraces);
        return;
    }

    auto* node { GetNode(node_id) };
    if (!node) {
        qInfo().noquote() << "SyncNodeDelta ignored: node not found in local node_hash_, id =" << node_id.toString(QUuid::WithoutBraces);
        return;
    }

    auto* node_o { static_cast<NodeO*>(node) };
    if (!node_o) {
        return;
    }

    const double initial_delta { data.value(kInitialDelta).toString().toDouble() };
    const double final_delta { data.value(kFinalDelta).toString().toDouble() };
    const double count_delta { data.value(kCountDelta).toString().toDouble() };
    const double measure_delta { data.value(kMeasureDelta).toString().toDouble() };
    const double discount_delta { data.value(kDiscountDelta).toString().toDouble() };

    node_o->initial_total += initial_delta;
    node_o->final_total += final_delta;
    node_o->count_total += count_delta;
    node_o->measure_total += measure_delta;
    node_o->discount_total += discount_delta;

    UpdateMeta(node_o, data);

    auto index { GetIndex(node_id) };
    if (index.isValid()) {
        const auto [start, end] = NodeUtils::NumericColumnRange(section_);
        if (end != -1)
            emit dataChanged(index.siblingAtColumn(start), index.siblingAtColumn(end));
    }
}

void TreeModelO::UpdateName(const QUuid& node_id, CString& new_name)
{
    auto* node { node_hash_.value(node_id) };
    if (!node) {
        qCritical() << "UpdateName: node_id not found in node_hash_, node_id =" << node_id;
    }
    assert(node);

    node->name = new_name;

    const auto message { JsonGen::NodeName(section_, node_id, new_name) };
    WebSocket::Instance()->SendMessage(kNodeName, message);

    emit SResizeColumnToContents(std::to_underlying(NodeEnum::kName));
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
        if (d_node->status == std::to_underlying(NodeStatus::kReleased)) {
            UpdateAncestorTotalOrder(node, -d_node->initial_total, -d_node->final_total, -d_node->count_total, -d_node->measure_total, -d_node->discount_total);

            if (node->unit == std::to_underlying(UnitO::kMonthly))
                emit SUpdateAmount(d_node->partner, -node->initial_total, -node->final_total);
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

    const int kUnit { node->unit };

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

        if (d_node->kind == std::to_underlying(NodeKind::kLeaf) && d_node->status == std::to_underlying(NodeStatus::kReleased))
            UpdateAncestorTotalOrder(node, d_node->initial_total, d_node->final_total, d_node->count_total, d_node->measure_total, d_node->discount_total);
    }
}

void TreeModelO::ResetBranch(Node* node)
{
    assert(node->kind == std::to_underlying(NodeKind::kBranch) && "ResetBranch: node must be of kind NodeKind::kBranch");

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

        if (node->kind == std::to_underlying(NodeKind::kBranch)) {
            ResetBranch(node);
            ++it;
            continue;
        }

        if (node->status == std::to_underlying(NodeStatus::kRecalled)) {
            ++it;
            continue;
        }

        if (node->kind == std::to_underlying(UnitO::kPending)) {
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

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeO>(lhs);
        auto* d_rhs = DerivedPtr<NodeO>(rhs);

        const NodeEnumO e_column { column };
        switch (e_column) {
        case NodeEnumO::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case NodeEnumO::kUserId:
            return (order == Qt::AscendingOrder) ? (lhs->user_id < rhs->user_id) : (lhs->user_id > rhs->user_id);
        case NodeEnumO::kCreateTime:
            return (order == Qt::AscendingOrder) ? (lhs->created_time < rhs->created_time) : (lhs->created_time > rhs->created_time);
        case NodeEnumO::kCreateBy:
            return (order == Qt::AscendingOrder) ? (lhs->created_by < rhs->created_by) : (lhs->created_by > rhs->created_by);
        case NodeEnumO::kUpdateTime:
            return (order == Qt::AscendingOrder) ? (lhs->updated_time < rhs->updated_time) : (lhs->updated_time > rhs->updated_time);
        case NodeEnumO::kUpdateBy:
            return (order == Qt::AscendingOrder) ? (lhs->updated_by < rhs->updated_by) : (lhs->updated_by > rhs->updated_by);
        case NodeEnumO::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case NodeEnumO::kDirectionRule:
            return (order == Qt::AscendingOrder) ? (lhs->direction_rule < rhs->direction_rule) : (lhs->direction_rule > rhs->direction_rule);
        case NodeEnumO::kKind:
            return (order == Qt::AscendingOrder) ? (lhs->kind < rhs->kind) : (lhs->kind > rhs->kind);
        case NodeEnumO::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case NodeEnumO::kPartner:
            return (order == Qt::AscendingOrder) ? (d_lhs->partner < d_rhs->partner) : (d_lhs->partner > d_rhs->partner);
        case NodeEnumO::kEmployee:
            return (order == Qt::AscendingOrder) ? (d_lhs->employee < d_rhs->employee) : (d_lhs->employee > d_rhs->employee);
        case NodeEnumO::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (d_lhs->issued_time < d_rhs->issued_time) : (d_lhs->issued_time > d_rhs->issued_time);
        case NodeEnumO::kCountTotal:
            return (order == Qt::AscendingOrder) ? (d_lhs->count_total < d_rhs->count_total) : (d_lhs->count_total > d_rhs->count_total);
        case NodeEnumO::kMeasureTotal:
            return (order == Qt::AscendingOrder) ? (d_lhs->measure_total < d_rhs->measure_total) : (d_lhs->measure_total > d_rhs->measure_total);
        case NodeEnumO::kDiscountTotal:
            return (order == Qt::AscendingOrder) ? (d_lhs->discount_total < d_rhs->discount_total) : (d_lhs->discount_total > d_rhs->discount_total);
        case NodeEnumO::kStatus:
            return (order == Qt::AscendingOrder) ? (d_lhs->status < d_rhs->status) : (d_lhs->status > d_rhs->status);
        case NodeEnumO::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        case NodeEnumO::kFinalTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    SortIterative(root_, Compare);
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
    bool branch { d_node->kind == std::to_underlying(NodeKind::kBranch) };

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
    case NodeEnumO::kDescription:
        return d_node->description;
    case NodeEnumO::kDirectionRule:
        return branch ? QVariant() : d_node->direction_rule;
    case NodeEnumO::kKind:
        return d_node->kind;
    case NodeEnumO::kUnit:
        return d_node->unit;
    case NodeEnumO::kPartner:
        return d_node->partner;
    case NodeEnumO::kEmployee:
        return d_node->employee;
    case NodeEnumO::kIssuedTime:
        return d_node->issued_time;
    case NodeEnumO::kCountTotal:
        return d_node->count_total;
    case NodeEnumO::kMeasureTotal:
        return d_node->measure_total;
    case NodeEnumO::kDiscountTotal:
        return d_node->discount_total;
    case NodeEnumO::kStatus:
        return d_node->status;
    case NodeEnumO::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumO::kFinalTotal:
        return d_node->final_total;
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

    bool update_ancestor { node->kind == std::to_underlying(NodeKind::kBranch) || node->status == std::to_underlying(NodeStatus::kReleased) };

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
