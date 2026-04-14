#include "searchnodemodeli.h"

SearchNodeModelI::SearchNodeModelI(CSectionInfo& info, CTreeModel* tree_model, const QHash<QUuid, Tag*>& tag_hash, QObject* parent)
    : SearchNodeModel { info, tree_model, tag_hash, parent }
{
}

QVariant SearchNodeModelI::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_node { DerivedPtr<NodeI>(node_list_.at(index.row())) };
    const NodeEnumI column { index.column() };

    switch (column) {
    case NodeEnumI::kName:
        return d_node->name;
    case NodeEnumI::kId:
        return d_node->id;
    case NodeEnumI::kVersion:
        return d_node->version;
    case NodeEnumI::kCode:
        return d_node->code;
    case NodeEnumI::kTag:
        return d_node->tag;
    case NodeEnumI::kDocument:
        return d_node->document;
    case NodeEnumI::kDescription:
        return d_node->description;
    case NodeEnumI::kDirectionRule:
        return d_node->direction_rule;
    case NodeEnumI::kKind:
        return std::to_underlying(d_node->kind);
    case NodeEnumI::kUnit:
        return std::to_underlying(d_node->unit);
    case NodeEnumI::kColor:
        return d_node->color;
    case NodeEnumI::kCommission:
        return d_node->commission;
    case NodeEnumI::kUnitPrice:
        return d_node->unit_price;
    case NodeEnumI::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumI::kFinalTotal:
        return d_node->final_total;
    }
}

void SearchNodeModelI::sort(int column, Qt::SortOrder order)
{
    const NodeEnumI e_column { column };

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeI>(lhs);
        auto* d_rhs = DerivedPtr<NodeI>(rhs);

        switch (e_column) {
        case NodeEnumI::kName:
            return utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumI::kCode:
            return utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumI::kDescription:
            return utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumI::kDirectionRule:
            return utils::CompareMember(lhs, rhs, &Node::direction_rule, order);
        case NodeEnumI::kKind:
            return utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumI::kUnit:
            return utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumI::kInitialTotal:
            return utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumI::kFinalTotal:
            return utils::CompareMember(lhs, rhs, &Node::final_total, order);
        case NodeEnumI::kTag:
            return utils::CompareMember(lhs, rhs, &Node::tag, order);
        case NodeEnumI::kColor:
            return utils::CompareMember(lhs, rhs, &Node::color, order);
        case NodeEnumI::kCommission:
            return utils::CompareMember(d_lhs, d_rhs, &NodeI::commission, order);
        case NodeEnumI::kUnitPrice:
            return utils::CompareMember(d_lhs, d_rhs, &NodeI::unit_price, order);
        case NodeEnumI::kDocument:
            return (order == Qt::AscendingOrder) ? (d_lhs->document.size() < d_rhs->document.size()) : (d_lhs->document.size() > d_rhs->document.size());
        case NodeEnumI::kId:
        case NodeEnumI::kVersion:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}
