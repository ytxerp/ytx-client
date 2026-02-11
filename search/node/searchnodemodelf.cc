#include "searchnodemodelf.h"

#include "utils/compareutils.h"

SearchNodeModelF::SearchNodeModelF(CSectionInfo& info, CTreeModel* tree_model, const QHash<QUuid, Tag*>& tag_hash, QObject* parent)
    : SearchNodeModel { info, tree_model, tag_hash, parent }
{
}

QVariant SearchNodeModelF::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* node { node_list_.at(index.row()) };
    const NodeEnumF column { index.column() };

    switch (column) {
    case NodeEnumF::kName:
        return node->name;
    case NodeEnumF::kUpdateBy:
        return node->updated_by;
    case NodeEnumF::kUpdateTime:
        return node->updated_time;
    case NodeEnumF::kCreateTime:
        return node->created_time;
    case NodeEnumF::kCreateBy:
        return node->created_by;
    case NodeEnumF::kVersion:
        return node->version;
    case NodeEnumF::kUserId:
        return node->user_id;
    case NodeEnumF::kId:
        return node->id;
    case NodeEnumF::kTag:
        return node->tag;
    case NodeEnumF::kCode:
        return node->code;
    case NodeEnumF::kColor:
        return node->color;
    case NodeEnumF::kDescription:
        return node->description;
    case NodeEnumF::kNote:
        return node->note;
    case NodeEnumF::kDirectionRule:
        return node->direction_rule;
    case NodeEnumF::kKind:
        return std::to_underlying(node->kind);
    case NodeEnumF::kUnit:
        return std::to_underlying(node->unit);
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
    const NodeEnumF e_column { column };

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        switch (e_column) {
        case NodeEnumF::kName:
            return Utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumF::kCode:
            return Utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumF::kDescription:
            return Utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumF::kNote:
            return Utils::CompareMember(lhs, rhs, &Node::note, order);
        case NodeEnumF::kDirectionRule:
            return Utils::CompareMember(lhs, rhs, &Node::direction_rule, order);
        case NodeEnumF::kKind:
            return Utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumF::kColor:
            return Utils::CompareMember(lhs, rhs, &Node::color, order);
        case NodeEnumF::kUnit:
            return Utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumF::kInitialTotal:
            return Utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumF::kFinalTotal:
            return Utils::CompareMember(lhs, rhs, &Node::final_total, order);
        case NodeEnumF::kTag:
            return Utils::CompareMember(lhs, rhs, &Node::tag, order);
        case NodeEnumF::kId:
        case NodeEnumF::kUpdateBy:
        case NodeEnumF::kUpdateTime:
        case NodeEnumF::kCreateTime:
        case NodeEnumF::kCreateBy:
        case NodeEnumF::kVersion:
        case NodeEnumF::kUserId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}
