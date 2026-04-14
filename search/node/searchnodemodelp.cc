#include "searchnodemodelp.h"

SearchNodeModelP::SearchNodeModelP(CSectionInfo& info, CTreeModel* tree_model, const QHash<QUuid, Tag*>& tag_hash, QObject* parent)
    : SearchNodeModel { info, tree_model, tag_hash, parent }
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
    case NodeEnumP::kVersion:
        return d_node->version;
    case NodeEnumP::kTag:
        return d_node->tag;
    case NodeEnumP::kCode:
        return d_node->code;
    case NodeEnumP::kColor:
        return d_node->color;
    case NodeEnumP::kDocument:
        return d_node->document;
    case NodeEnumP::kDescription:
        return d_node->description;
    case NodeEnumP::kKind:
        return std::to_underlying(d_node->kind);
    case NodeEnumP::kUnit:
        return std::to_underlying(d_node->unit);
    case NodeEnumP::kPaymentTerm:
        return d_node->payment_term;
    case NodeEnumP::kInitialTotal:
        return d_node->initial_total;
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
            return utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumP::kCode:
            return utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumP::kDescription:
            return utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumP::kTag:
            return utils::CompareMember(lhs, rhs, &Node::tag, order);
        case NodeEnumP::kKind:
            return utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumP::kUnit:
            return utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumP::kColor:
            return utils::CompareMember(lhs, rhs, &Node::color, order);
        case NodeEnumP::kPaymentTerm:
            return utils::CompareMember(d_lhs, d_rhs, &NodeP::payment_term, order);
        case NodeEnumP::kInitialTotal:
            return utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumP::kDocument:
            return (order == Qt::AscendingOrder) ? (d_lhs->document.size() < d_rhs->document.size()) : (d_lhs->document.size() > d_rhs->document.size());
        case NodeEnumP::kId:
        case NodeEnumP::kVersion:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}
