#include "searchnodemodelt.h"

#include <QJsonArray>

#include "global/nodepool.h"

SearchNodeModelT::SearchNodeModelT(CSectionInfo& info, CTreeModel* tree_model, QObject* parent)
    : SearchNodeModel { info, tree_model, parent }
{
}

void SearchNodeModelT::RNodeSearch(const QJsonObject& obj)
{
    const QJsonArray node_array { obj.value(kNodeArray).toArray() };

    beginResetModel();

    if (!node_list_.isEmpty()) {
        NodePool::Instance().Recycle(node_list_, section_);
        node_list_.clear();
    }

    for (const QJsonValue& val : node_array) {
        const QJsonObject obj { val.toObject() };

        Node* node { NodePool::Instance().Allocate(section_) };
        node->ReadJson(obj);

        node_list_.append(node);
    }
    endResetModel();
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
        return d_node->status;
    case NodeEnumT::kDocument:
        return d_node->document;
    case NodeEnumT::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumT::kFinalTotal:
        return d_node->final_total;
    default:
        return QVariant();
    }
}

void SearchNodeModelT::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.node_header.size())
        return;

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeT>(lhs);
        auto* d_rhs = DerivedPtr<NodeT>(rhs);

        switch (NodeEnumT(column)) {
        case NodeEnumT::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case NodeEnumT::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case NodeEnumT::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case NodeEnumT::kNote:
            return (order == Qt::AscendingOrder) ? (lhs->note < rhs->note) : (lhs->note > rhs->note);
        case NodeEnumT::kDirectionRule:
            return (order == Qt::AscendingOrder) ? (lhs->direction_rule < rhs->direction_rule) : (lhs->direction_rule > rhs->direction_rule);
        case NodeEnumT::kKind:
            return (order == Qt::AscendingOrder) ? (lhs->kind < rhs->kind) : (lhs->kind > rhs->kind);
        case NodeEnumT::kStatus:
            return (order == Qt::AscendingOrder) ? (d_lhs->status < d_rhs->status) : (d_lhs->status > d_rhs->status);
        case NodeEnumT::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case NodeEnumT::kColor:
            return (order == Qt::AscendingOrder) ? (d_lhs->color < d_rhs->color) : (d_lhs->color > d_rhs->color);
        case NodeEnumT::kDocument:
            return (order == Qt::AscendingOrder) ? (d_lhs->document.size() < d_rhs->document.size()) : (d_lhs->document.size() > d_rhs->document.size());
        case NodeEnumT::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        case NodeEnumT::kFinalTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}
