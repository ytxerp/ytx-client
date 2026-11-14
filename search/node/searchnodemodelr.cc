#include "searchnodemodelr.h"

#include <QJsonArray>

#include "component/constantwebsocket.h"
#include "global/nodepool.h"
#include "utils/tagutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

namespace search {

NodeModelR::NodeModelR(Section section, const QStringList& header, CTreeModel* tree_model, const QHash<QUuid, tag::Row>& tag_hash, QObject* parent)
    : NodeModel { section, header, tree_model, tag_hash, parent }
{
}

NodeModelR::~NodeModelR() { NodePool::Instance().Recycle(list_, section_); }

void NodeModelR::Rebuild(const QJsonArray& array)
{
    // 1. Prepare temporary list to store nodes
    QList<Node*> new_list {};

    for (const QJsonValue& val : array) {
        const QJsonObject node_obj { val.toObject() };

        Node* node { NodePool::Instance().Allocate(section_) };
        node->ReadJson(node_obj);

        new_list.append(node);
    }

    // 2. Update model in one step
    beginResetModel();

    // Recycle old nodes
    if (!list_.isEmpty()) {
        NodePool::Instance().Recycle(list_, section_);
    }

    // Move new nodes into model
    list_ = std::move(new_list);

    endResetModel();
}

QVariant NodeModelR::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const NodeEnumR column { index.column() };
    auto* d_node { static_cast<NodeR*>(index.internalPointer()) };

    switch (column) {
    case NodeEnumR::kName:
        return d_node->name;
    case NodeEnumR::kDescription:
        return d_node->description;
    case NodeEnumR::kCode:
        return d_node->code;
    case NodeEnumR::kKind:
        return std::to_underlying(d_node->kind);
    case NodeEnumR::kIssuedTime:
        return d_node->issued_time;
    case NodeEnumR::kStatus:
        return std::to_underlying(d_node->reimbursement_status);
    case NodeEnumR::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumR::kTag:
        return d_node->tag;
    case NodeEnumR::kColor:
        return d_node->color;
    case NodeEnumR::kDocument:
        return d_node->document;
    }
}

void NodeModelR::sort(int column, Qt::SortOrder order)
{
    const NodeEnumR e_column { column };

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeR>(lhs);
        auto* d_rhs = DerivedPtr<NodeR>(rhs);

        switch (e_column) {
        case NodeEnumR::kName:
            return utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumR::kDescription:
            return utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumR::kKind:
            return utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumR::kIssuedTime:
            return utils::CompareMember(d_lhs, d_rhs, &NodeR::issued_time, order);
        case NodeEnumR::kCode:
            return utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumR::kStatus:
            return utils::CompareMember(d_lhs, d_rhs, &NodeR::reimbursement_status, order);
        case NodeEnumR::kInitialTotal:
            return utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumR::kTag:
            return utils::CompareMember(lhs, rhs, &Node::tag, order);
        case NodeEnumR::kColor:
            return utils::CompareMember(lhs, rhs, &Node::color, order);
        case NodeEnumR::kDocument:
            return utils::CompareMember(lhs, rhs, &Node::document, order);
        }
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(list_, Compare);
    emit layoutChanged();
}

void NodeModelR::Search(QUuid widget_id, CString& text)
{
    if (text.trimmed().isEmpty()) {
        ClearModel();
        return;
    }

    // Parse search input into text and tag set
    const auto query { tag::ParseSearchQuery(text, tag_hash_) };

    // Perform the search (tag search has higher priority)
    if (!query.tags.isEmpty()) {
        WebSocket::Instance()->SendMessage(WsKey::kNodeTagSearch, JsonGen::NodeTagSearch(section_, widget_id, query.tags));
        return;
    }

    // Send order partner name search request to server
    if (!query.text.isEmpty()) {
        WebSocket::Instance()->SendMessage(WsKey::kNodeNameSearch, JsonGen::NodeNameSearch(section_, widget_id, query.text));
        return;
    }

    // Both tags and text are empty → clear the model
    ClearModel();
}

void NodeModelR::ClearModel()
{
    if (!list_.isEmpty()) {
        beginResetModel();
        NodePool::Instance().Recycle(list_, section_);
        endResetModel();
    }
}
}
