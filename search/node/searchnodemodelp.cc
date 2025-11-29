#include "searchnodemodelp.h"

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
    case NodeEnumP::kCode:
        return d_node->code;
    case NodeEnumP::kDescription:
        return d_node->description;
    case NodeEnumP::kNote:
        return d_node->note;
    case NodeEnumP::kKind:
        return d_node->kind;
    case NodeEnumP::kUnit:
        return d_node->unit;
    case NodeEnumP::kPaymentTerm:
        return d_node->payment_term;
    case NodeEnumP::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumP::kFinalTotal:
        return d_node->final_total;
    default:
        return QVariant();
    }
}

void SearchNodeModelP::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.node_header.size())
        return;

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeP>(lhs);
        auto* d_rhs = DerivedPtr<NodeP>(rhs);

        switch (NodeEnumP(column)) {
        case NodeEnumP::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case NodeEnumP::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case NodeEnumP::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case NodeEnumP::kNote:
            return (order == Qt::AscendingOrder) ? (lhs->note < rhs->note) : (lhs->note > rhs->note);
        case NodeEnumP::kKind:
            return (order == Qt::AscendingOrder) ? (lhs->kind < rhs->kind) : (lhs->kind > rhs->kind);
        case NodeEnumP::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case NodeEnumP::kPaymentTerm:
            return (order == Qt::AscendingOrder) ? (d_lhs->payment_term < d_rhs->payment_term) : (d_lhs->payment_term > d_rhs->payment_term);
        case NodeEnumP::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        case NodeEnumP::kFinalTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}
