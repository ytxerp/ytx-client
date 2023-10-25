#include "searchnodemodels.h"

#include "component/enumclass.h"

SearchNodeModelS::SearchNodeModelS(CSectionInfo& info, CTreeModel* tree_model, QObject* parent)
    : SearchNodeModel { info, tree_model, parent }
{
}

QVariant SearchNodeModelS::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_node { DerivedPtr<NodeS>(node_list_.at(index.row())) };
    const NodeEnumS kColumn { index.column() };

    switch (kColumn) {
    case NodeEnumS::kName:
        return d_node->name;
    case NodeEnumS::kId:
        return d_node->id;
    case NodeEnumS::kCode:
        return d_node->code;
    case NodeEnumS::kDescription:
        return d_node->description;
    case NodeEnumS::kNote:
        return d_node->note;
    case NodeEnumS::kKind:
        return d_node->kind;
    case NodeEnumS::kUnit:
        return d_node->unit;
    case NodeEnumS::kPaymentTerm:
        return d_node->payment_term;
    case NodeEnumS::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumS::kFinalTotal:
        return d_node->final_total;
    default:
        return QVariant();
    }
}

void SearchNodeModelS::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.node_header.size())
        return;

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeS>(lhs);
        auto* d_rhs = DerivedPtr<NodeS>(rhs);

        const NodeEnumS kColumn { column };
        switch (kColumn) {
        case NodeEnumS::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case NodeEnumS::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case NodeEnumS::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case NodeEnumS::kNote:
            return (order == Qt::AscendingOrder) ? (lhs->note < rhs->note) : (lhs->note > rhs->note);
        case NodeEnumS::kKind:
            return (order == Qt::AscendingOrder) ? (lhs->kind < rhs->kind) : (lhs->kind > rhs->kind);
        case NodeEnumS::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case NodeEnumS::kPaymentTerm:
            return (order == Qt::AscendingOrder) ? (d_lhs->payment_term < d_rhs->payment_term) : (d_lhs->payment_term > d_rhs->payment_term);
        case NodeEnumS::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        case NodeEnumS::kFinalTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}
