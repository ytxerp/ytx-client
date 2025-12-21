#include "treemodelt.h"

#include <QJsonArray>

#include "global/collator.h"
#include "global/nodepool.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TreeModelT::TreeModelT(CSectionInfo& info, CString& separator, int default_unit, QObject* parent)
    : TreeModel(info, separator, default_unit, parent)
{
    leaf_path_model_ = new ItemModel(this);
}

QVariant TreeModelT::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_node { DerivedPtr<NodeT>(GetNodeByIndex(index)) };

    if (d_node == root_)
        return QVariant();

    const NodeEnumT column { index.column() };

    switch (column) {
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
    case NodeEnumT::kVersion:
        return d_node->version;
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
        return d_node->status;
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
    if (!d_node)
        return false;

    const NodeEnumT column { index.column() };

    if (d_node->status == std::to_underlying(NodeStatus::kReleased) && column != NodeEnumT::kStatus) {
        qInfo() << "Edit ignored: node is released";
        return false;
    }

    const QUuid id { node->id };

    switch (column) {
    case NodeEnumT::kCode:
        NodeUtils::UpdateField(pending_updates_[id], node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kDescription:
        NodeUtils::UpdateField(pending_updates_[id], node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kNote:
        NodeUtils::UpdateField(pending_updates_[id], node, kNote, value.toString(), &Node::note, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kDirectionRule:
        UpdateDirectionRule(node, value.toBool());
        break;
    case NodeEnumT::kColor:
        NodeUtils::UpdateField(pending_updates_[id], d_node, kColor, value.toString(), &NodeT::color, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kIssuedTime:
        NodeUtils::UpdateIssuedTime(pending_updates_[id], d_node, kIssuedTime, value.toDateTime(), &NodeT::issued_time, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kDocument:
        NodeUtils::UpdateDocument(pending_updates_[id], d_node, kDocument, value.toStringList(), &NodeT::document, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kStatus:
        UpdateStatus(node, value.toInt());
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

    const NodeEnumT e_column { column };

    switch (e_column) {
    case NodeEnumT::kId:
    case NodeEnumT::kUserId:
    case NodeEnumT::kCreateTime:
    case NodeEnumT::kCreateBy:
    case NodeEnumT::kUpdateTime:
    case NodeEnumT::kUpdateBy:
    case NodeEnumT::kVersion:
        return;
    default:
        break;
    }

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeT>(lhs);
        auto* d_rhs = DerivedPtr<NodeT>(rhs);
        const auto& collator { Collator::Instance() };

        switch (e_column) {
        case NodeEnumT::kName:
            return (order == Qt::AscendingOrder) ? (collator.compare(lhs->name, rhs->name) < 0) : (collator.compare(lhs->name, rhs->name) > 0);
        case NodeEnumT::kCode:
            return (order == Qt::AscendingOrder) ? (collator.compare(lhs->code, rhs->code) < 0) : (collator.compare(lhs->code, rhs->code) > 0);
        case NodeEnumT::kDescription:
            return (order == Qt::AscendingOrder) ? (collator.compare(lhs->description, rhs->description) < 0)
                                                 : (collator.compare(lhs->description, rhs->description) > 0);
        case NodeEnumT::kNote:
            return (order == Qt::AscendingOrder) ? (collator.compare(lhs->note, rhs->note) < 0) : (collator.compare(lhs->note, rhs->note) > 0);
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

    const NodeEnumT column { index.column() };
    switch (column) {
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

    const int status { index.siblingAtColumn(std::to_underlying(NodeEnumT::kStatus)).data().toInt() };
    if (status == std::to_underlying(NodeStatus::kReleased))
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

    auto& update { pending_updates_[d_node->id] };
    RestartTimer(id);
    update.insert(kColor, QString());
}

void TreeModelT::UpdateNodeStatus(const QUuid& node_id, int status)
{
    auto* node = GetNode(node_id);
    if (!node)
        return;

    auto* d_node { DerivedPtr<NodeT>(node) };
    d_node->status = status;

    auto index { GetIndex(node_id) };
    if (index.isValid()) {
        emit dataChanged(index.siblingAtColumn(std::to_underlying(NodeEnumT::kStatus)), index.siblingAtColumn(std::to_underlying(NodeEnumT::kStatus)));
    }
}

void TreeModelT::UpdateStatus(Node* node, int value)
{
    auto* d_node { DerivedPtr<NodeT>(node) };
    if (!d_node)
        return;

    if (d_node->status == value)
        return;

    d_node->status = value;

    QJsonObject message { JsonGen::NodeStatus(section_, node->id, value) };
    WebSocket::Instance()->SendMessage(kNodeStatus, message);
}

void TreeModelT::ResetBranch(Node* node)
{
    assert(node->kind == std::to_underlying(NodeKind::kBranch) && "ResetBranch: node must be of kind NodeKind::kBranch");

    node->children.clear();
    node->initial_total = 0.0;
    node->final_total = 0.0;
}

void TreeModelT::ClearModel()
{
    // Clear tree structure, paths and item_model
    leaf_path_.clear();
    branch_path_.clear();
    leaf_path_model_->Clear();

    // Clear non-branch nodes from node_hash_, keep branch nodes and unfinishded nodes
    for (auto it = node_hash_.begin(); it != node_hash_.end();) {
        auto* node = static_cast<NodeT*>(it.value());

        if (node->kind == std::to_underlying(NodeKind::kBranch)) {
            ResetBranch(node);
            ++it;
            continue;
        }

        if (node->status == std::to_underlying(NodeStatus::kRecalled)) {
            ++it;
            continue;
        }

        NodePool::Instance().Recycle(node, section_);
        it = node_hash_.erase(it);
    }
}
