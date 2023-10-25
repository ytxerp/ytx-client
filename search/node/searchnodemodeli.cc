#include "searchnodemodeli.h"

#include "component/enumclass.h"

SearchNodeModelI::SearchNodeModelI(CSectionInfo& info, CTreeModel* tree_model, QObject* parent)
    : SearchNodeModel { info, tree_model, parent }
{
}

QVariant SearchNodeModelI::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_node { DerivedPtr<NodeI>(node_list_.at(index.row())) };
    const NodeEnumI kColumn { index.column() };

    switch (kColumn) {
    case NodeEnumI::kName:
        return d_node->name;
    case NodeEnumI::kId:
        return d_node->id;
    case NodeEnumI::kCode:
        return d_node->code;
    case NodeEnumI::kDescription:
        return d_node->description;
    case NodeEnumI::kNote:
        return d_node->note;
    case NodeEnumI::kDirectionRule:
        return d_node->direction_rule;
    case NodeEnumI::kKind:
        return d_node->kind;
    case NodeEnumI::kUnit:
        return d_node->unit;
    case NodeEnumI::kColor:
        return d_node->color;
    case NodeEnumI::kCommission:
        return QVariant();
    case NodeEnumI::kUnitPrice:
        return QVariant();
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
    if (column <= -1 || column >= info_.node_header.size())
        return;

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        const NodeEnumI kColumn { column };

        auto* d_lhs = DerivedPtr<NodeI>(lhs);
        auto* d_rhs = DerivedPtr<NodeI>(rhs);

        switch (kColumn) {
        case NodeEnumI::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case NodeEnumI::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case NodeEnumI::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case NodeEnumI::kNote:
            return (order == Qt::AscendingOrder) ? (lhs->note < rhs->note) : (lhs->note > rhs->note);
        case NodeEnumI::kDirectionRule:
            return (order == Qt::AscendingOrder) ? (lhs->direction_rule < rhs->direction_rule) : (lhs->direction_rule > rhs->direction_rule);
        case NodeEnumI::kKind:
            return (order == Qt::AscendingOrder) ? (lhs->kind < rhs->kind) : (lhs->kind > rhs->kind);
        case NodeEnumI::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case NodeEnumI::kFinalTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
        case NodeEnumI::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        case NodeEnumI::kColor:
            return (order == Qt::AscendingOrder) ? (d_lhs->color < d_rhs->color) : (d_lhs->color > d_rhs->color);
        case NodeEnumI::kCommission:
            return (order == Qt::AscendingOrder) ? (d_lhs->commission < d_rhs->commission) : (d_lhs->commission > d_rhs->commission);
        case NodeEnumI::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (d_lhs->unit_price < d_rhs->unit_price) : (d_lhs->unit_price > d_rhs->unit_price);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}
