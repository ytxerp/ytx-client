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
    case NodeEnumI::kUpdateBy:
        return d_node->updated_by;
    case NodeEnumI::kUpdateTime:
        return d_node->updated_time;
    case NodeEnumI::kCreateTime:
        return d_node->created_time;
    case NodeEnumI::kCreateBy:
        return d_node->created_by;
    case NodeEnumI::kVersion:
        return d_node->version;
    case NodeEnumI::kUserId:
        return d_node->user_id;
    case NodeEnumI::kCode:
        return d_node->code;
    case NodeEnumI::kTag:
        return d_node->tag;
    case NodeEnumI::kDescription:
        return d_node->description;
    case NodeEnumI::kNote:
        return d_node->note;
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
    default:
        return QVariant();
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
            return Utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumI::kCode:
            return Utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumI::kDescription:
            return Utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumI::kNote:
            return Utils::CompareMember(lhs, rhs, &Node::note, order);
        case NodeEnumI::kDirectionRule:
            return Utils::CompareMember(lhs, rhs, &Node::direction_rule, order);
        case NodeEnumI::kKind:
            return Utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumI::kUnit:
            return Utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumI::kInitialTotal:
            return Utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumI::kFinalTotal:
            return Utils::CompareMember(lhs, rhs, &Node::final_total, order);
        case NodeEnumI::kTag:
            return Utils::CompareMember(lhs, rhs, &Node::tag, order);
        case NodeEnumI::kColor:
            return Utils::CompareMember(lhs, rhs, &Node::color, order);
        case NodeEnumI::kCommission:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeI::commission, order);
        case NodeEnumI::kUnitPrice:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeI::unit_price, order);
        case NodeEnumI::kId:
        case NodeEnumI::kUpdateBy:
        case NodeEnumI::kUpdateTime:
        case NodeEnumI::kCreateTime:
        case NodeEnumI::kCreateBy:
        case NodeEnumI::kVersion:
        case NodeEnumI::kUserId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}
