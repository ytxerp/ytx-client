#include "searchnodemodelp.h"

#include "utils/compareutils.h"

SearchNodeModelP::SearchNodeModelP(CSectionInfo& info, CTreeModel* tree_model, QObject* parent)
    : SearchNodeModel { info, tree_model, parent }
{
}

QVariant SearchNodeModelP::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_node { DerivedPtr<NodeP>(node_list_.at(index.row())) };
    const NodeEnumP column { index.column() };

    switch (column) {
    case NodeEnumP::kName:
        return d_node->name;
    case NodeEnumP::kId:
        return d_node->id;
    case NodeEnumP::kUpdateBy:
        return d_node->updated_by;
    case NodeEnumP::kUpdateTime:
        return d_node->updated_time;
    case NodeEnumP::kCreateTime:
        return d_node->created_time;
    case NodeEnumP::kCreateBy:
        return d_node->created_by;
    case NodeEnumP::kVersion:
        return d_node->version;
    case NodeEnumP::kUserId:
        return d_node->user_id;
    case NodeEnumP::kCode:
        return d_node->code;
    case NodeEnumP::kColor:
        return d_node->color;
    case NodeEnumP::kDescription:
        return d_node->description;
    case NodeEnumP::kNote:
        return d_node->note;
    case NodeEnumP::kKind:
        return std::to_underlying(d_node->kind);
    case NodeEnumP::kUnit:
        return std::to_underlying(d_node->unit);
    case NodeEnumP::kPaymentTerm:
        return d_node->payment_term;
    case NodeEnumP::kInitialTotal:
        return d_node->initial_total;
    default:
        return QVariant();
    }
}

void SearchNodeModelP::sort(int column, Qt::SortOrder order)
{
    const NodeEnumP e_column { column };

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs { DerivedPtr<NodeP>(lhs) };
        auto* d_rhs { DerivedPtr<NodeP>(rhs) };

        switch (e_column) {
        case NodeEnumP::kName:
            return Utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumP::kCode:
            return Utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumP::kDescription:
            return Utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumP::kNote:
            return Utils::CompareMember(lhs, rhs, &Node::note, order);
        case NodeEnumP::kKind:
            return Utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumP::kUnit:
            return Utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumP::kColor:
            return Utils::CompareMember(lhs, rhs, &Node::color, order);
        case NodeEnumP::kPaymentTerm:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeP::payment_term, order);
        case NodeEnumP::kInitialTotal:
            return Utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumP::kId:
        case NodeEnumP::kUpdateBy:
        case NodeEnumP::kUpdateTime:
        case NodeEnumP::kCreateTime:
        case NodeEnumP::kCreateBy:
        case NodeEnumP::kVersion:
        case NodeEnumP::kUserId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}
