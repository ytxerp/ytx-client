#include "treemodelt.h"

#include <QJsonArray>

#include "websocket/jsongen.h"

TreeModelT::TreeModelT(CSectionInfo& info, CString& separator, QObject* parent)
    : TreeModel(info, separator, parent)
{
    leaf_path_model_ = new ItemModel(this);
}

QVariant TreeModelT::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* d_node { static_cast<NodeT*>(index.internalPointer()) };
    Q_ASSERT(d_node != nullptr);

    const NodeEnumT column { index.column() };

    switch (column) {
    case NodeEnumT::kName:
        return d_node->name;
    case NodeEnumT::kId:
        return d_node->id;
    case NodeEnumT::kVersion:
        return d_node->version;
    case NodeEnumT::kCode:
        return d_node->code;
    case NodeEnumT::kDescription:
        return d_node->description;
    case NodeEnumT::kDirectionRule:
        return d_node->direction_rule;
    case NodeEnumT::kTag:
        return d_node->tag;
    case NodeEnumT::kKind:
        return std::to_underlying(d_node->kind);
    case NodeEnumT::kUnit:
        return std::to_underlying(d_node->unit);
    case NodeEnumT::kColor:
        return d_node->color;
    case NodeEnumT::kDocument:
        return d_node->document;
    case NodeEnumT::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumT::kFinalTotal:
        return d_node->final_total;
    }
}

bool TreeModelT::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    auto* node { static_cast<Node*>(index.internalPointer()) };
    auto* d_node { static_cast<NodeT*>(node) };

    Q_ASSERT(node != nullptr);
    Q_ASSERT(d_node != nullptr);

    const NodeEnumT column { index.column() };

    const QUuid id { node->id };

    switch (column) {
    case NodeEnumT::kCode:
        utils::UpdateField(pending_updates_[id], node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kDescription:
        utils::UpdateField(pending_updates_[id], node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kTag:
        utils::UpdateStringList(pending_updates_[id], node, kTag, value.toStringList(), &Node::tag, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kDirectionRule:
        UpdateDirectionRule(node, value.toBool(), index);
        break;
    case NodeEnumT::kColor:
        utils::UpdateField(pending_updates_[id], node, kColor, value.toString(), &Node::color, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kDocument:
        utils::UpdateStringList(pending_updates_[id], node, kDocument, value.toStringList(), &Node::document, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumT::kId:
    case NodeEnumT::kVersion:
    case NodeEnumT::kName:
    case NodeEnumT::kKind:
    case NodeEnumT::kUnit:
    case NodeEnumT::kInitialTotal:
    case NodeEnumT::kFinalTotal:
        return false;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    return true;
}

void TreeModelT::sort(int column, Qt::SortOrder order)
{
    const NodeEnumT e_column { column };

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeT>(lhs);
        auto* d_rhs = DerivedPtr<NodeT>(rhs);

        switch (e_column) {
        case NodeEnumT::kName:
            return utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumT::kCode:
            return utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumT::kDescription:
            return utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumT::kDirectionRule:
            return utils::CompareMember(lhs, rhs, &Node::direction_rule, order);
        case NodeEnumT::kKind:
            return utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumT::kUnit:
            return utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumT::kColor:
            return utils::CompareMember(lhs, rhs, &Node::color, order);
        case NodeEnumT::kDocument:
            return (order == Qt::AscendingOrder) ? (d_lhs->document.size() < d_rhs->document.size()) : (d_lhs->document.size() > d_rhs->document.size());
        case NodeEnumT::kInitialTotal:
            return utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumT::kFinalTotal:
            return utils::CompareMember(lhs, rhs, &Node::final_total, order);
        case NodeEnumT::kTag:
            return utils::CompareMember(lhs, rhs, &Node::tag, order);
        case NodeEnumT::kId:
        case NodeEnumT::kVersion:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    utils::SortIterative(root_, Compare);
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
    case NodeEnumT::kTag:
    case NodeEnumT::kKind:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}
