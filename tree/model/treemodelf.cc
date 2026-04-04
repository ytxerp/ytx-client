#include "treemodelf.h"

#include "component/constantdouble.h"

TreeModelF::TreeModelF(CSectionInfo& info, CString& separator, QObject* parent)
    : TreeModel(info, separator, parent)
{
    leaf_path_model_ = new ItemModel(this);
}

QVariant TreeModelF::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* node { static_cast<Node*>(index.internalPointer()) };
    Q_ASSERT(node != nullptr);

    const NodeEnumF column { index.column() };

    switch (column) {
    case NodeEnumF::kName:
        return node->name;
    case NodeEnumF::kId:
        return node->id;
    case NodeEnumF::kVersion:
        return node->version;
    case NodeEnumF::kCode:
        return node->code;
    case NodeEnumF::kDescription:
        return node->description;
    case NodeEnumF::kTag:
        return node->tag;
    case NodeEnumF::kColor:
        return node->color;
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
    case NodeEnumF::kDocument:
        return node->document;
    }
}

bool TreeModelF::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    auto* node { static_cast<Node*>(index.internalPointer()) };
    Q_ASSERT(node != nullptr);

    const NodeEnumF column { index.column() };
    const QUuid id { node->id };

    switch (column) {
    case NodeEnumF::kCode:
        Utils::UpdateField(pending_updates_[id], node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumF::kDescription:
        Utils::UpdateField(pending_updates_[id], node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumF::kColor:
        Utils::UpdateField(pending_updates_[id], node, kColor, value.toString(), &Node::color, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumF::kTag:
        Utils::UpdateStringList(pending_updates_[id], node, kTag, value.toStringList(), &Node::tag, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumF::kDirectionRule:
        UpdateDirectionRule(node, value.toBool(), index);
        break;
    case NodeEnumF::kDocument:
        Utils::UpdateStringList(pending_updates_[id], node, kDocument, value.toStringList(), &Node::document, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumF::kId:
    case NodeEnumF::kVersion:
    case NodeEnumF::kName:
    case NodeEnumF::kKind:
    case NodeEnumF::kUnit:
    case NodeEnumF::kInitialTotal:
    case NodeEnumF::kFinalTotal:
        return false;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    return true;
}

void TreeModelF::sort(int column, Qt::SortOrder order)
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
        case NodeEnumF::kDirectionRule:
            return Utils::CompareMember(lhs, rhs, &Node::direction_rule, order);
        case NodeEnumF::kKind:
            return Utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumF::kUnit:
            return Utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumF::kColor:
            return Utils::CompareMember(lhs, rhs, &Node::color, order);
        case NodeEnumF::kTag:
            return Utils::CompareMember(lhs, rhs, &Node::tag, order);
        case NodeEnumF::kInitialTotal:
            return Utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumF::kFinalTotal:
            return Utils::CompareMember(lhs, rhs, &Node::final_total, order);
        case NodeEnumF::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document.size() < rhs->document.size()) : (lhs->document.size() > rhs->document.size());
        case NodeEnumF::kId:
        case NodeEnumF::kVersion:
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
    case NodeEnumF::kColor:
    case NodeEnumF::kTag:
    case NodeEnumF::kDocument:
    case NodeEnumF::kUnit:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}

QSet<QUuid> TreeModelF::UpdateAncestorTotal(Node* node, double initial_delta, double final_delta, double, double, double) const
{
    QSet<QUuid> affected_ids {};

    if (!node || node == root_)
        return affected_ids;

    affected_ids.insert(node->id);

    if (!node->parent || node->parent == root_)
        return affected_ids;

    if (FloatEqual(initial_delta, 0.0) && FloatEqual(final_delta, 0.0))
        return affected_ids;

    const auto unit { node->unit };
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

void TreeModelF::InitAncestorTotal(Node* node, double initial_delta, double final_delta, double, double, double) const
{
    if (!node || node == root_ || !node->parent || node->parent == root_)
        return;

    if (FloatEqual(initial_delta, 0.0) && FloatEqual(final_delta, 0.0))
        return;

    const auto unit { node->unit };
    const bool direction_rule { node->direction_rule };

    for (Node* current = node->parent; current && current != root_; current = current->parent) {
        const int multiplier { current->direction_rule == direction_rule ? 1 : -1 };

        current->final_total += multiplier * final_delta;

        if (current->unit == unit) {
            current->initial_total += multiplier * initial_delta;
        }
    }
}
