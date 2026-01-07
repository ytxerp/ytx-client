#include "searchnodemodelt.h"

#include <QJsonArray>

#include "global/nodepool.h"
#include "utils/compareutils.h"

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

        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}
