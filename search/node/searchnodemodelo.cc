#include "searchnodemodelo.h"

#include <QJsonArray>

#include "global/nodepool.h"
#include "utils/compareutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

SearchNodeModelO::SearchNodeModelO(CSectionInfo& info, CTreeModel* tree_model, CTreeModel* partner_tree_model, QObject* parent)
    : SearchNodeModel { info, tree_model, parent }
    , partner_tree_model_ { partner_tree_model }
{
}

void SearchNodeModelO::RNodeSearch(const QJsonObject& obj)
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

QVariant SearchNodeModelO::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_node { DerivedPtr<NodeO>(node_list_.at(index.row())) };
    const NodeEnumO column { index.column() };

    switch (column) {
    case NodeEnumO::kName:
        return d_node->name;
    case NodeEnumO::kId:
        return d_node->id;
    case NodeEnumO::kDescription:
        return d_node->description;
    case NodeEnumO::kDirectionRule:
        return d_node->direction_rule;
    case NodeEnumO::kKind:
        return d_node->kind;
    case NodeEnumO::kUnit:
        return d_node->unit;
    case NodeEnumO::kPartner:
        return d_node->partner_id;
    case NodeEnumO::kEmployee:
        return d_node->employee_id;
    case NodeEnumO::kIssuedTime:
        return d_node->issued_time;
    case NodeEnumO::kCountTotal:
        return d_node->count_total;
    case NodeEnumO::kMeasureTotal:
        return d_node->measure_total;
    case NodeEnumO::kDiscountTotal:
        return d_node->discount_total;
    case NodeEnumO::kStatus:
        return d_node->status;
    case NodeEnumO::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumO::kFinalTotal:
        return d_node->final_total;
    default:
        return QVariant();
    }
}

void SearchNodeModelO::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column < info_.node_header.size());

    const NodeEnumO e_column { column };

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeO>(lhs);
        auto* d_rhs = DerivedPtr<NodeO>(rhs);

        switch (e_column) {
        case NodeEnumO::kName:
            return Utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumO::kDescription:
            return Utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumO::kDirectionRule:
            return Utils::CompareMember(lhs, rhs, &Node::direction_rule, order);
        case NodeEnumO::kKind:
            return Utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumO::kUnit:
            return Utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumO::kPartner:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::partner_id, order);
        case NodeEnumO::kEmployee:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::employee_id, order);
        case NodeEnumO::kIssuedTime:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::issued_time, order);
        case NodeEnumO::kCountTotal:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::count_total, order);
        case NodeEnumO::kMeasureTotal:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::measure_total, order);
        case NodeEnumO::kDiscountTotal:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::discount_total, order);
        case NodeEnumO::kStatus:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::status, order);
        case NodeEnumO::kInitialTotal:
            return Utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumO::kFinalTotal:
            return Utils::CompareMember(lhs, rhs, &Node::final_total, order);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}

void SearchNodeModelO::Search(CString& text)
{
    if (text.isEmpty()) {
        if (!node_list_.isEmpty()) {
            beginResetModel();
            NodePool::Instance().Recycle(node_list_, section_);
            node_list_.clear();
            endResetModel();
        }
        return;
    }

    WebSocket::Instance()->SendMessage(kNodeSearch, JsonGen::NodeSearch(section_, text));
}
