#include "searchnodemodelf.h"

namespace search {

NodeModelF::NodeModelF(CSectionInfo& info, CTreeModel* tree_model, const QHash<QUuid, TagRow*>& tag_hash, QObject* parent)
    : NodeModel { info, tree_model, tag_hash, parent }
{
}

QVariant NodeModelF::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const NodeEnumF column { index.column() };
    auto* d_node { static_cast<NodeF*>(index.internalPointer()) };

    switch (column) {
    case NodeEnumF::kName:
        return d_node->name;
    case NodeEnumF::kId:
        return d_node->id;
    case NodeEnumF::kTag:
        return d_node->tag;
    case NodeEnumF::kCode:
        return d_node->code;
    case NodeEnumF::kColor:
        return d_node->color;
    case NodeEnumF::kDescription:
        return d_node->description;
    case NodeEnumF::kDirectionRule:
        return d_node->direction_rule;
    case NodeEnumF::kKind:
        return std::to_underlying(d_node->kind);
    case NodeEnumF::kUnit:
        return std::to_underlying(d_node->unit);
    case NodeEnumF::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumF::kFinalTotal:
        return d_node->final_total;
    case NodeEnumF::kDocument:
        return d_node->document;
    case NodeEnumF::kRoles:
        return static_cast<int>(d_node->roles);
    }
}

void NodeModelF::sort(int column, Qt::SortOrder order)
{
    const NodeEnumF e_column { column };

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeF>(lhs);
        auto* d_rhs = DerivedPtr<NodeF>(rhs);

        switch (e_column) {
        case NodeEnumF::kName:
            return utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumF::kCode:
            return utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumF::kDescription:
            return utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumF::kDirectionRule:
            return utils::CompareMember(lhs, rhs, &Node::direction_rule, order);
        case NodeEnumF::kKind:
            return utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumF::kColor:
            return utils::CompareMember(lhs, rhs, &Node::color, order);
        case NodeEnumF::kUnit:
            return utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumF::kInitialTotal:
            return utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumF::kFinalTotal:
            return utils::CompareMember(lhs, rhs, &Node::final_total, order);
        case NodeEnumF::kTag:
            return utils::CompareMember(lhs, rhs, &Node::tag, order);
        case NodeEnumF::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document.size() < rhs->document.size()) : (lhs->document.size() > rhs->document.size());
        case NodeEnumF::kRoles:
            return utils::CompareMember(d_lhs, d_rhs, &NodeF::roles, order);
        case NodeEnumF::kId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}
}
