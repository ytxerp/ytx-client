#include "searchnodemodelo.h"

#include <QJsonArray>

#include "component/constantwebsocket.h"
#include "global/masterdataregistry.h"
#include "global/nodepool.h"
#include "utils/tagutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

namespace search {

NodeModelO::NodeModelO(CSectionInfo& info, CTreeModel* tree_model, const QHash<QUuid, TagRow*>& tag_hash, QObject* parent)
    : NodeModel { info, tree_model, tag_hash, parent }
{
}

void NodeModelO::RNodeSearch(const QJsonObject& obj)
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
    }

    // Move new nodes into model
    node_list_ = std::move(temp_list);

    endResetModel();
}

QVariant NodeModelO::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const NodeEnumO column { index.column() };
    auto* d_node { static_cast<NodeO*>(index.internalPointer()) };

    switch (column) {
    case NodeEnumO::kName:
        return MasterDataRegistry::Instance().PartnerName(d_node->partner_id);
    case NodeEnumO::kId:
        return d_node->id;
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
    case NodeEnumO::kTag:
        return d_node->tag;
    }
}

void NodeModelO::sort(int column, Qt::SortOrder order)
{
    const NodeEnumO e_column { column };

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeO>(lhs);
        auto* d_rhs = DerivedPtr<NodeO>(rhs);

        switch (e_column) {
        case NodeEnumO::kName: {
            const auto& master = MasterDataRegistry::Instance();
            return utils::CompareString(master.PartnerName(d_lhs->partner_id), master.PartnerName(d_rhs->partner_id), order);
        }
        case NodeEnumO::kDescription:
            return utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumO::kDirectionRule:
            return utils::CompareMember(lhs, rhs, &Node::direction_rule, order);
        case NodeEnumO::kKind:
            return utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumO::kUnit:
            return utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumO::kEmployeeId:
            return utils::CompareMember(d_lhs, d_rhs, &NodeO::employee_id, order);
        case NodeEnumO::kIssuedTime:
            return utils::CompareMember(d_lhs, d_rhs, &NodeO::issued_time, order);
        case NodeEnumO::kCode:
            return utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumO::kCountTotal:
            return utils::CompareMember(d_lhs, d_rhs, &NodeO::count_total, order);
        case NodeEnumO::kMeasureTotal:
            return utils::CompareMember(d_lhs, d_rhs, &NodeO::measure_total, order);
        case NodeEnumO::kDiscountTotal:
            return utils::CompareMember(d_lhs, d_rhs, &NodeO::discount_total, order);
        case NodeEnumO::kStatus:
            return utils::CompareMember(d_lhs, d_rhs, &NodeO::status, order);
        case NodeEnumO::kInitialTotal:
            return utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumO::kFinalTotal:
            return utils::CompareMember(lhs, rhs, &Node::final_total, order);
        case NodeEnumO::kTag:
            return utils::CompareMember(lhs, rhs, &Node::tag, order);
        case NodeEnumO::kId:
        case NodeEnumO::kIsSettled:
        case NodeEnumO::kSettlementId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}

void NodeModelO::Search(CString& text)
{
    if (text.trimmed().isEmpty()) {
        ClearModel();
        return;
    }

    // Parse search input into text and tag set
    const SearchQuery query { utils::ParseSearchQuery(text, tag_hash_) };

    // Perform the search (tag search has higher priority)
    if (!query.tags.isEmpty()) {
        WebSocket::Instance()->SendMessage(WsKey::kNodeTagSearch, JsonGen::NodeTagSearch(section_, query.tags));
        return;
    }

    // Send order partner name search request to server
    if (!query.text.isEmpty()) {
        WebSocket::Instance()->SendMessage(WsKey::kNodeNameSearch, JsonGen::NodeNameSearch(section_, query.text));
        return;
    }

    // Both tags and text are empty → clear the model
    ClearModel();
}

void NodeModelO::ClearModel()
{
    if (!node_list_.isEmpty()) {
        beginResetModel();
        NodePool::Instance().Recycle(node_list_, section_);
        endResetModel();
    }
}
}
