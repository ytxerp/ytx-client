#include "treemodelo.h"

#include <QJsonArray>

#include "global/nodepool.h"

TreeModelO::TreeModelO(CSectionInfo& info, CString& separator, int default_unit, QObject* parent)
    : TreeModel(info, separator, default_unit, parent)
{
}

TreeModelO::~TreeModelO() { NodePool::Instance().Recycle(node_cache_, section_); }

void TreeModelO::RSyncDelta(const QUuid& node_id, double initial_delta, double final_delta, double first_delta, double second_delta, double discount_delta)
{
    auto* node { DerivedPtr<NodeO>(node_hash_.value(node_id)) };
    assert(node && node->kind == kLeaf);

    if (first_delta == 0.0 && second_delta == 0.0 && initial_delta == 0.0 && discount_delta == 0.0 && final_delta == 0.0)
        return;

    // dbhub_o_->UpdateLeafValue(node);

    auto index { GetIndex(node->id) };
    emit dataChanged(index.siblingAtColumn(std::to_underlying(NodeEnumO::kFirstTotal)), index.siblingAtColumn(std::to_underlying(NodeEnumO::kFinalTotal)));

    if (node->is_finished) {
        UpdateAncestorValue(node, initial_delta, final_delta, first_delta, second_delta, discount_delta);
    }
}

void TreeModelO::RSyncFinished(const QUuid& node_id, bool value)
{
    auto* node { DerivedPtr<NodeO>(node_hash_.value(node_id)) };
    assert(node);

    int coefficient = value ? 1 : -1;
    UpdateAncestorValue(node, coefficient * node->initial_total, coefficient * node->final_total, coefficient * node->first_total,
        coefficient * node->second_total, coefficient * node->discount_total);

    if (node->unit == std::to_underlying(UnitO::kMS))
        emit SUpdateAmount(node->party, coefficient * node->initial_total, coefficient * node->final_total);
}

void TreeModelO::AckTree(const QJsonObject& obj)
{
    const QJsonArray node_array { obj.value(kNode).toArray() };
    const QJsonArray path_array { obj.value(kPath).toArray() };

    beginResetModel();
    Clear();
    TransferNode(node_hash_, node_cache_);

    for (const QJsonValue& val : node_array) {
        const QJsonObject obj = val.toObject();

        const QUuid id { QUuid(obj.value(kId).toString()) };
        Node* node {};

        auto it = node_cache_.find(id);
        if (it != node_cache_.end()) {
            node = it.value();
        } else {
            node = NodePool::Instance().Allocate(section_);
            node->ReadJson(obj);
        }

        node_hash_.insert(id, node);
    }

    if (!node_array.isEmpty())
        BuildHierarchy(path_array);

    endResetModel();

    if (!node_array.isEmpty())
        HandleNode();
}

QString TreeModelO::Path(const QUuid& node_id) const
{
    if (auto it = node_hash_.constFind(node_id); it != node_hash_.constEnd())
        return it.value()->name;

    return {};
}

Node* TreeModelO::GetNode(const QUuid& node_id) const { return node_hash_.value(node_id); }

void TreeModelO::RemovePath(Node* node, Node* parent_node)
{
    auto* d_node { DerivedPtr<NodeO>(node) };

    switch (d_node->kind) {
    case kBranch:
        for (auto* child : std::as_const(node->children)) {
            child->parent = parent_node;
            parent_node->children.emplace_back(child);
        }
        break;
    case kLeaf:
        if (d_node->is_finished) {
            UpdateAncestorValue(node, -d_node->initial_total, -d_node->final_total, -d_node->first_total, -d_node->second_total, -d_node->discount_total);

            if (node->unit == std::to_underlying(UnitO::kMS))
                emit SUpdateAmount(d_node->party, -node->initial_total, -node->final_total);
        }
        break;
    default:
        break;
    }
}

bool TreeModelO::UpdateAncestorValue(Node* node, double initial_delta, double final_delta, double first_delta, double second_delta, double discount_delta)
{
    assert(node && node != root_ && node->parent);

    if (node->parent == root_)
        return false;

    if (initial_delta == 0.0 && final_delta == 0.0 && first_delta == 0.0 && second_delta == 0.0 && discount_delta == 0.0)
        return false;

    const int kUnit { node->unit };
    const int kColumnBegin { std::to_underlying(NodeEnumO::kFirstTotal) };
    int column_end { std::to_underlying(NodeEnumO::kFinalTotal) };

    // 确定需要更新的列范围
    if (initial_delta == 0.0 && final_delta == 0.0 && second_delta == 0.0 && discount_delta == 0.0)
        column_end = kColumnBegin;

    QModelIndexList ancestor {};
    for (Node* current = node->parent; current && current != root_; current = current->parent) {
        if (current->unit != kUnit)
            continue;

        auto* d_node { DerivedPtr<NodeO>(current) };

        d_node->first_total += first_delta;
        d_node->second_total += second_delta;
        d_node->discount_total += discount_delta;
        d_node->initial_total += initial_delta;
        d_node->final_total += final_delta;

        ancestor.emplaceBack(GetIndex(current->id));
    }

    if (!ancestor.isEmpty())
        emit dataChanged(index(ancestor.first().row(), kColumnBegin), index(ancestor.last().row(), column_end), { Qt::DisplayRole });

    return true;
}

void TreeModelO::HandleNode()
{
    for (auto* node : std::as_const(node_hash_)) {
        auto* d_node { DerivedPtr<NodeO>(node) };

        if (d_node->kind == kLeaf && d_node->is_finished)
            UpdateAncestorValue(node, d_node->initial_total, d_node->final_total, d_node->first_total, d_node->second_total, d_node->discount_total);
    }
}

void TreeModelO::ResetBranch(Node* node)
{
    if (node->kind == kBranch) {
        node->children.clear();

        auto* d_node { DerivedPtr<NodeO>(node) };
        d_node->first_total = 0.0;
        d_node->second_total = 0.0;
        d_node->initial_total = 0.0;
        d_node->discount_total = 0.0;
        d_node->final_total = 0.0;
    }
}

void TreeModelO::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column < node_header_.size());

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeO>(lhs);
        auto* d_rhs = DerivedPtr<NodeO>(rhs);

        const NodeEnumO kColumn { column };
        switch (kColumn) {
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
        case NodeEnumO::kParty:
            return (order == Qt::AscendingOrder) ? (d_lhs->party < d_rhs->party) : (d_lhs->party > d_rhs->party);
        case NodeEnumO::kEmployee:
            return (order == Qt::AscendingOrder) ? (d_lhs->employee < d_rhs->employee) : (d_lhs->employee > d_rhs->employee);
        case NodeEnumO::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (d_lhs->issued_time < d_rhs->issued_time) : (d_lhs->issued_time > d_rhs->issued_time);
        case NodeEnumO::kFirstTotal:
            return (order == Qt::AscendingOrder) ? (d_lhs->first_total < d_rhs->first_total) : (d_lhs->first_total > d_rhs->first_total);
        case NodeEnumO::kSecondTotal:
            return (order == Qt::AscendingOrder) ? (d_lhs->second_total < d_rhs->second_total) : (d_lhs->second_total > d_rhs->second_total);
        case NodeEnumO::kDiscountTotal:
            return (order == Qt::AscendingOrder) ? (d_lhs->discount_total < d_rhs->discount_total) : (d_lhs->discount_total > d_rhs->discount_total);
        case NodeEnumO::kIsFinished:
            return (order == Qt::AscendingOrder) ? (d_lhs->is_finished < d_rhs->is_finished) : (d_lhs->is_finished > d_rhs->is_finished);
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
    if (d_node == root_)
        return QVariant();

    const NodeEnumO kColumn { index.column() };
    bool branch { d_node->kind == kBranch };

    switch (kColumn) {
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
    case NodeEnumO::kParty:
        return d_node->party.isNull() ? QVariant() : d_node->party;
    case NodeEnumO::kEmployee:
        return d_node->employee.isNull() ? QVariant() : d_node->employee;
    case NodeEnumO::kIssuedTime:
        return branch || !d_node->issued_time.isValid() ? QVariant() : d_node->issued_time;
    case NodeEnumO::kFirstTotal:
        return d_node->first_total == 0 ? QVariant() : d_node->first_total;
    case NodeEnumO::kSecondTotal:
        return d_node->second_total == 0 ? QVariant() : d_node->second_total;
    case NodeEnumO::kDiscountTotal:
        return d_node->discount_total == 0 ? QVariant() : d_node->discount_total;
    case NodeEnumO::kIsFinished:
        return !branch && d_node->is_finished ? d_node->is_finished : QVariant();
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

    const NodeEnumO kColumn { index.column() };
    switch (kColumn) {
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

    bool update_ancestor { node->kind == kBranch || node->is_finished };

    if (update_ancestor) {
        UpdateAncestorValue(node, -node->initial_total, -node->final_total, -node->first_total, -node->second_total, -node->discount_total);
    }

    destination_parent->children.insert(destinationChild, node);
    node->parent = destination_parent;

    if (update_ancestor) {
        UpdateAncestorValue(node, node->initial_total, node->final_total, node->first_total, node->second_total, node->discount_total);
    }

    endMoveRows();

    emit SResizeColumnToContents(std::to_underlying(NodeEnum::kName));

    return true;
}
