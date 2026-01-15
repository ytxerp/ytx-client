#include "treemodelt.h"

#include <QJsonArray>

#include "utils/compareutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TreeModelT::TreeModelT(CSectionInfo& info, CString& separator, QObject* parent)
    : TreeModel(info, separator, parent)
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
        return std::to_underlying(d_node->kind);
    case NodeEnumT::kUnit:
        return std::to_underlying(d_node->unit);
    case NodeEnumT::kColor:
        return d_node->color;
    case NodeEnumT::kIssuedTime:
        return d_node->issued_time;
    case NodeEnumT::kStatus:
        return std::to_underlying(d_node->status);
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

    if (d_node->status == NodeStatus::kReleased && column != NodeEnumT::kStatus) {
        qInfo() << "Edit ignored: node is released";
        return false;
    }

    const QUuid id { node->id };

    switch (column) {
    case NodeEnumT::kCode:
        Utils::UpdateField(pending_updates_[id], node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kDescription:
        Utils::UpdateField(pending_updates_[id], node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kNote:
        Utils::UpdateField(pending_updates_[id], node, kNote, value.toString(), &Node::note, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kDirectionRule:
        UpdateDirectionRule(node, value.toBool());
        break;
    case NodeEnumT::kColor:
        Utils::UpdateField(pending_updates_[id], d_node, kColor, value.toString(), &NodeT::color, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kIssuedTime:
        Utils::UpdateIssuedTime(pending_updates_[id], d_node, kIssuedTime, value.toDateTime(), &NodeT::issued_time, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kDocument:
        Utils::UpdateDocument(pending_updates_[id], d_node, kDocument, value.toStringList(), &NodeT::document, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kStatus:
        UpdateStatus(node, NodeStatus(value.toInt()));
        break;
    case NodeEnumT::kId:
    case NodeEnumT::kUpdateBy:
    case NodeEnumT::kUpdateTime:
    case NodeEnumT::kCreateTime:
    case NodeEnumT::kCreateBy:
    case NodeEnumT::kVersion:
    case NodeEnumT::kUserId:
    case NodeEnumT::kName:
    case NodeEnumT::kKind:
    case NodeEnumT::kUnit:
    case NodeEnumT::kInitialTotal:
    case NodeEnumT::kFinalTotal:
        return false;
    }

    emit SResizeColumnToContents(index.column());
    return true;
}

void TreeModelT::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column < node_header_.size());

    const NodeEnumT e_column { column };

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeT>(lhs);
        auto* d_rhs = DerivedPtr<NodeT>(rhs);

        switch (e_column) {
        case NodeEnumT::kName:
            return Utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumT::kCode:
            return Utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumT::kDescription:
            return Utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumT::kNote:
            return Utils::CompareMember(lhs, rhs, &Node::note, order);
        case NodeEnumT::kDirectionRule:
            return Utils::CompareMember(lhs, rhs, &Node::direction_rule, order);
        case NodeEnumT::kKind:
            return Utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumT::kUnit:
            return Utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumT::kStatus:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeT::status, order);
        case NodeEnumT::kIssuedTime:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeT::issued_time, order);
        case NodeEnumT::kColor:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeT::color, order);
        case NodeEnumT::kDocument:
            return (order == Qt::AscendingOrder) ? (d_lhs->document.size() < d_rhs->document.size()) : (d_lhs->document.size() > d_rhs->document.size());
        case NodeEnumT::kInitialTotal:
            return Utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumT::kFinalTotal:
            return Utils::CompareMember(lhs, rhs, &Node::final_total, order);
        case NodeEnumT::kId:
        case NodeEnumT::kUpdateBy:
        case NodeEnumT::kUpdateTime:
        case NodeEnumT::kCreateTime:
        case NodeEnumT::kCreateBy:
        case NodeEnumT::kVersion:
        case NodeEnumT::kUserId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    Utils::SortIterative(root_, Compare);
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
    const QUuid node_id { d_node->id };

    d_node->color.clear();
    emit dataChanged(index.siblingAtColumn(std::to_underlying(NodeEnumT::kColor)), index.siblingAtColumn(std::to_underlying(NodeEnumT::kColor)));

    auto& update { pending_updates_[d_node->id] };
    RestartTimer(node_id);
    update.insert(kColor, QString());
}

void TreeModelT::UpdateStatus(const QUuid& node_id, NodeStatus status)
{
    auto* node = GetNode(node_id);
    if (!node)
        return;

    auto* d_node { DerivedPtr<NodeT>(node) };
    d_node->status = status;

    const int status_column { std::to_underlying(NodeEnumT::kStatus) };
    EmitRowChanged(node_id, status_column, status_column);
}

void TreeModelT::UpdateStatus(Node* node, NodeStatus value)
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
