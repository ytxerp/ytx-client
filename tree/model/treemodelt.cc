#include "treemodelt.h"

#include <QJsonArray>

#include "global/nodepool.h"

TreeModelT::TreeModelT(CSectionInfo& info, CString& separator, int default_unit, QObject* parent)
    : TreeModel(info, separator, default_unit, parent)
{
    leaf_model_ = new ItemModel(this);
    node_cache_.insert(QUuid(), root_);
}

TreeModelT::~TreeModelT() { NodePool::Instance().Recycle(node_cache_, section_); }

QVariant TreeModelT::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_node { DerivedPtr<NodeT>(GetNodeByIndex(index)) };

    if (d_node == root_)
        return QVariant();

    const NodeEnumT kColumn { index.column() };

    switch (kColumn) {
    case NodeEnumT::kName:
        return d_node->name;
    case NodeEnumT::kId:
        return d_node->id;
    case NodeEnumT::kUserId:
        return d_node->user_id;
    case NodeEnumT::kCreateTime:
        return d_node->created_time;
    case NodeEnumT::kCreateBy:
        return d_node->created_by;
    case NodeEnumT::kUpdateTime:
        return d_node->updated_time;
    case NodeEnumT::kUpdateBy:
        return d_node->updated_by;
    case NodeEnumT::kCode:
        return d_node->code;
    case NodeEnumT::kDescription:
        return d_node->description;
    case NodeEnumT::kNote:
        return d_node->note;
    case NodeEnumT::kDirectionRule:
        return d_node->direction_rule;
    case NodeEnumT::kKind:
        return d_node->kind;
    case NodeEnumT::kUnit:
        return d_node->unit;
    case NodeEnumT::kColor:
        return d_node->color;
    case NodeEnumT::kIssuedTime:
        return d_node->issued_time;
    case NodeEnumT::kStatus:
        return d_node->status ? d_node->status : QVariant();
    case NodeEnumT::kDocument:
        return d_node->document;
    case NodeEnumT::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumT::kFinalTotal:
        return d_node->final_total;
    default:
        return QVariant();
    }
}

bool TreeModelT::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    auto* node { GetNodeByIndex(index) };
    auto* d_node { DerivedPtr<NodeT>(node) };

    if (node == root_)
        return false;

    const NodeEnumT kColumn { index.column() };
    const QUuid id { node->id };
    auto& cache { caches_[id] };

    switch (kColumn) {
    case NodeEnumT::kCode:
        NodeUtils::UpdateField(cache, node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kDescription:
        NodeUtils::UpdateField(cache, node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kNote:
        NodeUtils::UpdateField(cache, node, kNote, value.toString(), &Node::note, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kDirectionRule:
        UpdateDirectionRule(node, value.toBool());
        break;
    case NodeEnumT::kColor:
        NodeUtils::UpdateField(cache, d_node, kColor, value.toString(), &NodeT::color, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kIssuedTime:
        NodeUtils::UpdateIssuedTime(cache, d_node, kIssuedTime, value.toDateTime(), &NodeT::issued_time, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kDocument:
        NodeUtils::UpdateDocument(cache, d_node, kDocument, value.toStringList(), &NodeT::document, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kStatus:
        if (d_node->kind == kLeaf)
            NodeUtils::UpdateField(cache, d_node, kStatus, value.toBool(), &NodeT::status, [id, this]() { RestartTimer(id); });
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());
    return true;
}

void TreeModelT::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column < node_header_.size());

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeT>(lhs);
        auto* d_rhs = DerivedPtr<NodeT>(rhs);

        const NodeEnumT kColumn { column };
        switch (kColumn) {
        case NodeEnumT::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case NodeEnumT::kUserId:
            return (order == Qt::AscendingOrder) ? (lhs->user_id < rhs->user_id) : (lhs->user_id > rhs->user_id);
        case NodeEnumT::kCreateTime:
            return (order == Qt::AscendingOrder) ? (lhs->created_time < rhs->created_time) : (lhs->created_time > rhs->created_time);
        case NodeEnumT::kCreateBy:
            return (order == Qt::AscendingOrder) ? (lhs->created_by < rhs->created_by) : (lhs->created_by > rhs->created_by);
        case NodeEnumT::kUpdateTime:
            return (order == Qt::AscendingOrder) ? (lhs->updated_time < rhs->updated_time) : (lhs->updated_time > rhs->updated_time);
        case NodeEnumT::kUpdateBy:
            return (order == Qt::AscendingOrder) ? (lhs->updated_by < rhs->updated_by) : (lhs->updated_by > rhs->updated_by);
        case NodeEnumT::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case NodeEnumT::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case NodeEnumT::kNote:
            return (order == Qt::AscendingOrder) ? (lhs->note < rhs->note) : (lhs->note > rhs->note);
        case NodeEnumT::kDirectionRule:
            return (order == Qt::AscendingOrder) ? (lhs->direction_rule < rhs->direction_rule) : (lhs->direction_rule > rhs->direction_rule);
        case NodeEnumT::kKind:
            return (order == Qt::AscendingOrder) ? (lhs->kind < rhs->kind) : (lhs->kind > rhs->kind);
        case NodeEnumT::kStatus:
            return (order == Qt::AscendingOrder) ? (d_lhs->status < d_rhs->status) : (d_lhs->status > d_rhs->status);
        case NodeEnumT::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (d_lhs->issued_time < d_rhs->issued_time) : (d_lhs->issued_time > d_rhs->issued_time);
        case NodeEnumT::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case NodeEnumT::kColor:
            return (order == Qt::AscendingOrder) ? (d_lhs->color < d_rhs->color) : (d_lhs->color > d_rhs->color);
        case NodeEnumT::kDocument:
            return (order == Qt::AscendingOrder) ? (d_lhs->document.size() < d_rhs->document.size()) : (d_lhs->document.size() > d_rhs->document.size());
        case NodeEnumT::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        case NodeEnumT::kFinalTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    SortIterative(root_, Compare);
    emit layoutChanged();
}

Qt::ItemFlags TreeModelT::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };

    const NodeEnumT kColumn { index.column() };
    switch (kColumn) {
    case NodeEnumT::kName:
        flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        break;
    case NodeEnumT::kFinalTotal:
    case NodeEnumT::kInitialTotal:
    case NodeEnumT::kColor:
    case NodeEnumT::kUnit:
    case NodeEnumT::kKind:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    const bool status { index.siblingAtColumn(std::to_underlying(NodeEnumT::kStatus)).data().toBool() };
    if (status)
        flags &= ~Qt::ItemIsEditable;

    return flags;
}

void TreeModelT::ResetColor(const QModelIndex& index)
{
    auto* node { GetNodeByIndex(index) };
    auto* d_node { DerivedPtr<NodeT>(node) };
    const QUuid id { d_node->id };

    d_node->color.clear();
    emit dataChanged(index.siblingAtColumn(std::to_underlying(NodeEnumT::kColor)), index.siblingAtColumn(std::to_underlying(NodeEnumT::kColor)));

    auto& cache { caches_[d_node->id] };
    RestartTimer(id);
    cache.insert(kColor, QString());
}

void TreeModelT::AckTree(const QJsonObject& obj)
{
    const QJsonArray node_array { obj.value(kNode).toArray() };
    const QJsonArray path_array { obj.value(kPath).toArray() };

    beginResetModel();
    ResetModel();

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

        RegisterNode(node);
    }

    if (node_hash_.size() >= 2)
        BuildHierarchy(path_array);

    endResetModel();

    if (!node_array.isEmpty())
        HandleNode();
}

Node* TreeModelT::GetNode(const QUuid& node_id) const
{
    auto* node = node_cache_.value(node_id);
    if (!node) {
        qInfo() << "node_id not found in node_cache_";
    }

    return node;
}

void TreeModelT::UpdateStatus(NodeT* node, bool value)
{
    if (node->status == value)
        return;
}

void TreeModelT::ResetBranch(Node* node)
{
    assert(node->kind == kBranch && "ResetBranch: node must be of kind kBranch");

    node->children.clear();
    node->initial_total = 0.0;
    node->final_total = 0.0;
}

void TreeModelT::ResetModel()
{
    // Clear tree structure, paths and leaf_model
    root_->children.clear();
    leaf_path_.clear();
    branch_path_.clear();
    leaf_model_->Clear();

    // Clear non-branch nodes from node_hash_, keep branch nodes and unfinidhws nodes
    for (auto it = node_hash_.begin(); it != node_hash_.end();) {
        auto* node = static_cast<NodeT*>(it.value());

        if (node->kind == kBranch) {
            ResetBranch(node);
            ++it;
            continue;
        }

        if (node->status == std::to_underlying(NodeStatus::kUnfinished)) {
            ++it;
            continue;
        }

        it = node_hash_.erase(it);
    }
}
