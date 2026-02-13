#include "searchnodemodelo.h"

#include <QJsonArray>

#include "global/nodepool.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

SearchNodeModelO::SearchNodeModelO(CSectionInfo& info, CTreeModel* tree_model, const QHash<QUuid, Tag*>& tag_hash, QObject* parent)
    : SearchNodeModel { info, tree_model, tag_hash, parent }
{
}

void SearchNodeModelO::RNodeSearch(const QJsonObject& obj)
{
    // 1. Prepare temporary list to store nodes
    QList<Node*> temp_list {};

    const QJsonArray node_array { obj.value(kNodeArray).toArray() };

    for (const QJsonValue& val : node_array) {
        const QJsonObject node_obj { val.toObject() };

        Node* node { NodePool::Instance().Allocate(section_) };
        node->ReadJson(node_obj);

        temp_list.append(node);
    }

    // 2. Update model in one step
    beginResetModel();

    // Recycle old nodes
    if (!node_list_.isEmpty()) {
        NodePool::Instance().Recycle(node_list_, section_);
        node_list_.clear();
    }

    // Move new nodes into model
    node_list_ = std::move(temp_list);

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
    case NodeEnumO::kUpdateBy:
        return d_node->updated_by;
    case NodeEnumO::kUpdateTime:
        return d_node->updated_time;
    case NodeEnumO::kCreateTime:
        return d_node->created_time;
    case NodeEnumO::kCreateBy:
        return d_node->created_by;
    case NodeEnumO::kVersion:
        return d_node->version;
    case NodeEnumO::kUserId:
        return d_node->user_id;
    case NodeEnumO::kDescription:
        return d_node->description;
    case NodeEnumO::kCode:
        return d_node->code;
    case NodeEnumO::kDirectionRule:
        return d_node->direction_rule;
    case NodeEnumO::kKind:
        return std::to_underlying(d_node->kind);
    case NodeEnumO::kUnit:
        return std::to_underlying(d_node->unit);
    case NodeEnumO::kPartnerId:
        return d_node->partner_id;
    case NodeEnumO::kEmployeeId:
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
        return std::to_underlying(d_node->status);
    case NodeEnumO::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumO::kFinalTotal:
        return d_node->final_total;
    case NodeEnumO::kIsSettled:
        return d_node->is_settled;
    case NodeEnumO::kSettlementId:
        return d_node->settlement_id;
    default:
        return QVariant();
    }
}

void SearchNodeModelO::sort(int column, Qt::SortOrder order)
{
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
        case NodeEnumO::kPartnerId:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::partner_id, order);
        case NodeEnumO::kEmployeeId:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::employee_id, order);
        case NodeEnumO::kIssuedTime:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeO::issued_time, order);
        case NodeEnumO::kCode:
            return Utils::CompareMember(lhs, rhs, &Node::code, order);
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
        case NodeEnumO::kId:
        case NodeEnumO::kUpdateBy:
        case NodeEnumO::kUpdateTime:
        case NodeEnumO::kCreateTime:
        case NodeEnumO::kCreateBy:
        case NodeEnumO::kVersion:
        case NodeEnumO::kUserId:
        case NodeEnumO::kIsSettled:
        case NodeEnumO::kSettlementId:
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
