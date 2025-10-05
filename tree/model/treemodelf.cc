#include "treemodelf.h"

#include "global/nodepool.h"

TreeModelF::TreeModelF(CSectionInfo& info, CString& separator, int default_unit, QObject* parent)
    : TreeModel(info, separator, default_unit, parent)
{
    leaf_model_ = new ItemModel(this);
}

TreeModelF::~TreeModelF() { NodePool::Instance().Recycle(node_hash_, section_); }

QVariant TreeModelF::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* node { GetNodeByIndex(index) };
    if (node == root_)
        return QVariant();

    const NodeEnumF kColumn { index.column() };

    switch (kColumn) {
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

    const NodeEnumF kColumn { index.column() };
    const QUuid id { node->id };

    switch (kColumn) {
    case NodeEnumF::kCode:
        NodeUtils::UpdateField(caches_[id], node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumF::kDescription:
        NodeUtils::UpdateField(caches_[id], node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumF::kNote:
        NodeUtils::UpdateField(caches_[id], node, kNote, value.toString(), &Node::note, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumF::kDirectionRule:
        UpdateDirectionRule(node, value.toBool());
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());
    return true;
}

void TreeModelF::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column < node_header_.size());

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        const NodeEnumF kColumn { column };
        switch (kColumn) {
        case NodeEnumF::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case NodeEnumF::kUserId:
            return (order == Qt::AscendingOrder) ? (lhs->user_id < rhs->user_id) : (lhs->user_id > rhs->user_id);
        case NodeEnumF::kCreateTime:
            return (order == Qt::AscendingOrder) ? (lhs->created_time < rhs->created_time) : (lhs->created_time > rhs->created_time);
        case NodeEnumF::kCreateBy:
            return (order == Qt::AscendingOrder) ? (lhs->created_by < rhs->created_by) : (lhs->created_by > rhs->created_by);
        case NodeEnumF::kUpdateTime:
            return (order == Qt::AscendingOrder) ? (lhs->updated_time < rhs->updated_time) : (lhs->updated_time > rhs->updated_time);
        case NodeEnumF::kUpdateBy:
            return (order == Qt::AscendingOrder) ? (lhs->updated_by < rhs->updated_by) : (lhs->updated_by > rhs->updated_by);
        case NodeEnumF::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case NodeEnumF::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case NodeEnumF::kNote:
            return (order == Qt::AscendingOrder) ? (lhs->note < rhs->note) : (lhs->note > rhs->note);
        case NodeEnumF::kDirectionRule:
            return (order == Qt::AscendingOrder) ? (lhs->direction_rule < rhs->direction_rule) : (lhs->direction_rule > rhs->direction_rule);
        case NodeEnumF::kKind:
            return (order == Qt::AscendingOrder) ? (lhs->kind < rhs->kind) : (lhs->kind > rhs->kind);
        case NodeEnumF::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case NodeEnumF::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        case NodeEnumF::kFinalTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    SortIterative(root_, Compare);
    emit layoutChanged();
}

Qt::ItemFlags TreeModelF::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const NodeEnumF kColumn { index.column() };

    switch (kColumn) {
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

QSet<QUuid> TreeModelF::UpdateAncestorValue(Node* node, double initial_delta, double final_delta, double /*first*/, double /*second*/, double /*discount*/)
{
    assert(node && node != root_ && node->parent);
    QSet<QUuid> affected_ids {};

    if (node->parent == root_)
        return affected_ids;

    if (initial_delta == 0.0 && final_delta == 0.0)
        return affected_ids;

    const int unit { node->unit };
    const bool rule { node->direction_rule };

    affected_ids.insert(node->id);

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
