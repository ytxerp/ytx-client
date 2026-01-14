#include "searchnodemodelt.h"

#include <QJsonArray>

#include "utils/compareutils.h"

SearchNodeModelT::SearchNodeModelT(CSectionInfo& info, CTreeModel* tree_model, QObject* parent)
    : SearchNodeModel { info, tree_model, parent }
{
}

QVariant SearchNodeModelT::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_node { DerivedPtr<NodeT>(node_list_.at(index.row())) };
    const NodeEnumT column { index.column() };

    switch (column) {
    case NodeEnumT::kName:
        return d_node->name;
    case NodeEnumT::kId:
        return d_node->id;
    case NodeEnumT::kUpdateBy:
        return d_node->updated_by;
    case NodeEnumT::kUpdateTime:
        return d_node->updated_time;
    case NodeEnumT::kCreateTime:
        return d_node->created_time;
    case NodeEnumT::kCreateBy:
        return d_node->created_by;
    case NodeEnumT::kVersion:
        return d_node->version;
    case NodeEnumT::kUserId:
        return d_node->user_id;
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
    case NodeEnumT::kStatus:
        return std::to_underlying(d_node->status);
    case NodeEnumT::kDocument:
        return d_node->document;
    case NodeEnumT::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumT::kFinalTotal:
        return d_node->final_total;
    case NodeEnumT::kIssuedTime:
        return d_node->issued_time;
    default:
        return QVariant();
    }
}

void SearchNodeModelT::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column < info_.node_header.size());

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
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}
