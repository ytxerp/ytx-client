#include "searchnodemodelf.h"

#include "component/enumclass.h"

SearchNodeModelF::SearchNodeModelF(CSectionInfo& info, CTreeModel* tree_model, QObject* parent)
    : SearchNodeModel { info, tree_model, parent }
{
}

QVariant SearchNodeModelF::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* node { node_list_.at(index.row()) };
    const NodeEnumF kColumn { index.column() };

    switch (kColumn) {
    case NodeEnumF::kName:
        return node->name;
    case NodeEnumF::kId:
        return node->id;
    case NodeEnumF::kCode:
        return node->code;
    case NodeEnumF::kDescription:
        return node->description;
    case NodeEnumF::kNote:
        return node->note;
    case NodeEnumF::kDirectionRule:
        return node->direction_rule;
    case NodeEnumF::kKind:
        return node->kind;
    case NodeEnumF::kUnit:
        return node->unit;
    case NodeEnumF::kInitialTotal:
        return node->initial_total;
    case NodeEnumF::kFinalTotal:
        return node->final_total;
    default:
        return QVariant();
    }
}

void SearchNodeModelF::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.node_header.size())
        return;

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        const NodeEnumF kColumn { column };

        switch (kColumn) {
        case NodeEnumF::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case NodeEnumF::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case NodeEnumF::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case NodeEnumF::kNote:
            return (order == Qt::AscendingOrder) ? (lhs->note < rhs->note) : (lhs->note > rhs->note);
        case NodeEnumF::kDirectionRule:
            return (order == Qt::AscendingOrder) ? (lhs->direction_rule < rhs->direction_rule) : (lhs->direction_rule > rhs->direction_rule);
        case NodeEnumF::kKind:
            return (order == Qt::AscendingOrder) ? (lhs->kind < rhs->kind) : (lhs->kind > rhs->kind);
        case NodeEnumF::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case NodeEnumF::kFinalTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
        case NodeEnumF::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}
