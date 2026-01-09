#include "treemodelf.h"

#include "utils/compareutils.h"

TreeModelF::TreeModelF(CSectionInfo& info, CString& separator, QObject* parent)
    : TreeModel(info, separator, parent)
{
    leaf_path_model_ = new ItemModel(this);
}

QVariant TreeModelF::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* node { GetNodeByIndex(index) };
    if (node == root_)
        return QVariant();

    const NodeEnumF column { index.column() };

    switch (column) {
    case NodeEnumF::kName:
        return node->name;
    case NodeEnumF::kId:
        return node->id;
    case NodeEnumF::kUserId:
        return node->user_id;
    case NodeEnumF::kCreateTime:
        return node->created_time;
    case NodeEnumF::kCreateBy:
        return node->created_by;
    case NodeEnumF::kUpdateTime:
        return node->updated_time;
    case NodeEnumF::kUpdateBy:
        return node->updated_by;
    case NodeEnumF::kVersion:
        return node->version;
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
        return node->unit == root_->unit ? QVariant() : node->initial_total;
    case NodeEnumF::kFinalTotal:
        return node->final_total;
    default:
        return QVariant();
    }
}

bool TreeModelF::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    auto* node { GetNodeByIndex(index) };
    if (node == root_)
        return false;

    const NodeEnumF column { index.column() };
    const QUuid id { node->id };

    switch (column) {
    case NodeEnumF::kCode:
        Utils::UpdateField(pending_updates_[id], node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumF::kDescription:
        Utils::UpdateField(pending_updates_[id], node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumF::kNote:
        Utils::UpdateField(pending_updates_[id], node, kNote, value.toString(), &Node::note, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumF::kDirectionRule:
        UpdateDirectionRule(node, value.toBool());
        break;
    case NodeEnumF::kId:
    case NodeEnumF::kUpdateBy:
    case NodeEnumF::kUpdateTime:
    case NodeEnumF::kCreateTime:
    case NodeEnumF::kCreateBy:
    case NodeEnumF::kVersion:
    case NodeEnumF::kUserId:
    case NodeEnumF::kName:
    case NodeEnumF::kKind:
    case NodeEnumF::kUnit:
    case NodeEnumF::kInitialTotal:
    case NodeEnumF::kFinalTotal:
        break;
    }

    emit SResizeColumnToContents(index.column());
    return true;
}

void TreeModelF::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column < node_header_.size());

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
        case NodeEnumF::kUnit:
            return Utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumF::kInitialTotal:
            return Utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumF::kFinalTotal:
            return Utils::CompareMember(lhs, rhs, &Node::final_total, order);
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
    Utils::SortIterative(root_, Compare);
    emit layoutChanged();
}

Qt::ItemFlags TreeModelF::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const NodeEnumF column { index.column() };

    switch (column) {
    case NodeEnumF::kName:
        flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        flags &= ~Qt::ItemIsEditable;
        break;
    case NodeEnumF::kInitialTotal:
    case NodeEnumF::kFinalTotal:
    case NodeEnumF::kKind:
    case NodeEnumF::kUnit:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}

QSet<QUuid> TreeModelF::UpdateAncestorTotal(Node* node, double initial_delta, double final_delta)
{
    assert(node && node != root_ && node->parent);
    QSet<QUuid> affected_ids {};

    if (node->parent == root_)
        return affected_ids;

    if (initial_delta == 0.0 && final_delta == 0.0)
        return affected_ids;

    const int unit { node->unit };
    const bool rule { node->direction_rule };

    for (Node* current = node->parent; current && current != root_; current = current->parent) {
        const int multiplier { current->direction_rule == rule ? 1 : -1 };

        current->final_total += multiplier * final_delta;

        if (current->unit == unit) {
            current->initial_total += multiplier * initial_delta;
        }

        affected_ids.insert(current->id);
    }

    return affected_ids;
}
