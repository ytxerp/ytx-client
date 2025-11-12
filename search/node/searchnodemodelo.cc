#include "searchnodemodelo.h"

#include <QJsonArray>

#include "component/enumclass.h"
#include "global/nodepool.h"
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
        return d_node->partner;
    case NodeEnumO::kEmployee:
        return d_node->employee;
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
    if (column <= -1 || column >= info_.node_header.size())
        return;

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeO>(lhs);
        auto* d_rhs = DerivedPtr<NodeO>(rhs);

        const NodeEnumO e_column { column };
        switch (e_column) {
        case NodeEnumO::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case NodeEnumO::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case NodeEnumO::kDirectionRule:
            return (order == Qt::AscendingOrder) ? (lhs->direction_rule < rhs->direction_rule) : (lhs->direction_rule > rhs->direction_rule);
        case NodeEnumO::kKind:
            return (order == Qt::AscendingOrder) ? (lhs->kind < rhs->kind) : (lhs->kind > rhs->kind);
        case NodeEnumO::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case NodeEnumO::kPartner:
            return (order == Qt::AscendingOrder) ? (d_lhs->partner < d_rhs->partner) : (d_lhs->partner > d_rhs->partner);
        case NodeEnumO::kEmployee:
            return (order == Qt::AscendingOrder) ? (d_lhs->employee < d_rhs->employee) : (d_lhs->employee > d_rhs->employee);
        case NodeEnumO::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (d_lhs->issued_time < d_rhs->issued_time) : (d_lhs->issued_time > d_rhs->issued_time);
        case NodeEnumO::kCountTotal:
            return (order == Qt::AscendingOrder) ? (d_lhs->count_total < d_rhs->count_total) : (d_lhs->count_total > d_rhs->count_total);
        case NodeEnumO::kMeasureTotal:
            return (order == Qt::AscendingOrder) ? (d_lhs->measure_total < d_rhs->measure_total) : (d_lhs->measure_total > d_rhs->measure_total);
        case NodeEnumO::kDiscountTotal:
            return (order == Qt::AscendingOrder) ? (d_lhs->discount_total < d_rhs->discount_total) : (d_lhs->discount_total > d_rhs->discount_total);
        case NodeEnumO::kStatus:
            return (order == Qt::AscendingOrder) ? (d_lhs->status < d_rhs->status) : (d_lhs->status > d_rhs->status);
        case NodeEnumO::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        case NodeEnumO::kFinalTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
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
