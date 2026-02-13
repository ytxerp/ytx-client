#include "treemodelp.h"

TreeModelP::TreeModelP(CSectionInfo& info, CString& separator, QObject* parent)
    : TreeModel(info, separator, parent)
{
    leaf_path_model_ = new ItemModel(this);
    leaf_path_model_->AppendItem(QString(), QUuid());
}

void TreeModelP::RUpdateAmount(const QUuid& node_id, double initial_delta)
{
    Q_ASSERT(!node_id.isNull());

    if (FloatEqual(initial_delta, 0.0))
        return;

    auto* node { node_hash_.value(node_id) };
    if (!node || node == root_ || node->kind != NodeKind::kLeaf)
        return;

    node->initial_total += initial_delta;

    const auto& affected_ids { UpdateAncestorTotal(node, initial_delta, 0.0) };
    RefreshAffectedTotal(affected_ids);
}

QSet<QUuid>* TreeModelP::UnitSet(NodeUnit unit)
{
    const NodeUnit kUnit { unit };

    switch (kUnit) {
    case NodeUnit::PCustomer:
        return &cset_;
    case NodeUnit::PVendor:
        return &vset_;
    case NodeUnit::PEmployee:
        return &eset_;
    default:
        return nullptr;
    }
}

QSet<QUuid> TreeModelP::UpdateAncestorTotal(Node* node, double initial_delta, double /*final_delta*/)
{
    QSet<QUuid> affected_ids {};

    if (!node || !node->parent || node->parent == root_)
        return affected_ids;

    if (FloatEqual(initial_delta, 0.0))
        return affected_ids;

    const auto unit { node->unit };

    for (Node* current = node->parent; current && current != root_; current = current->parent) {
        if (current->unit != unit)
            continue;

        current->initial_total += initial_delta;

        affected_ids.insert(current->id);
    }

    return affected_ids;
}

void TreeModelP::sort(int column, Qt::SortOrder order)
{
    const NodeEnumP e_column { column };

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeP>(lhs);
        auto* d_rhs = DerivedPtr<NodeP>(rhs);

        switch (e_column) {
        case NodeEnumP::kName:
            return Utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumP::kCode:
            return Utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumP::kDescription:
            return Utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumP::kNote:
            return Utils::CompareMember(lhs, rhs, &Node::note, order);
        case NodeEnumP::kKind:
            return Utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumP::kUnit:
            return Utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumP::kPaymentTerm:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeP::payment_term, order);
        case NodeEnumP::kInitialTotal:
            return Utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumP::kColor:
            return Utils::CompareMember(lhs, rhs, &Node::color, order);
        case NodeEnumP::kTag:
            return Utils::CompareMember(lhs, rhs, &Node::tag, order);
        case NodeEnumP::kId:
        case NodeEnumP::kUpdateBy:
        case NodeEnumP::kUpdateTime:
        case NodeEnumP::kCreateTime:
        case NodeEnumP::kCreateBy:
        case NodeEnumP::kVersion:
        case NodeEnumP::kUserId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    Utils::SortIterative(root_, Compare);
    emit layoutChanged();
}

QVariant TreeModelP::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* d_node { static_cast<NodeP*>(index.internalPointer()) };
    Q_ASSERT(d_node != nullptr);

    const NodeEnumP column { index.column() };

    switch (column) {
    case NodeEnumP::kName:
        return d_node->name;
    case NodeEnumP::kId:
        return d_node->id;
    case NodeEnumP::kUserId:
        return d_node->user_id;
    case NodeEnumP::kCreateTime:
        return d_node->created_time;
    case NodeEnumP::kCreateBy:
        return d_node->created_by;
    case NodeEnumP::kUpdateTime:
        return d_node->updated_time;
    case NodeEnumP::kUpdateBy:
        return d_node->updated_by;
    case NodeEnumP::kVersion:
        return d_node->version;
    case NodeEnumP::kCode:
        return d_node->code;
    case NodeEnumP::kDescription:
        return d_node->description;
    case NodeEnumP::kNote:
        return d_node->note;
    case NodeEnumP::kKind:
        return std::to_underlying(d_node->kind);
    case NodeEnumP::kTag:
        return d_node->tag;
    case NodeEnumP::kUnit:
        return std::to_underlying(d_node->unit);
    case NodeEnumP::kPaymentTerm:
        return d_node->payment_term;
    case NodeEnumP::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumP::kColor:
        return d_node->color;
    default:
        return QVariant();
    }
}

bool TreeModelP::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    auto* node { static_cast<Node*>(index.internalPointer()) };
    auto* d_node { static_cast<NodeP*>(node) };

    Q_ASSERT(node != nullptr);
    Q_ASSERT(d_node != nullptr);

    const NodeEnumP column { index.column() };
    const QUuid id { node->id };

    switch (column) {
    case NodeEnumP::kCode:
        Utils::UpdateField(pending_updates_[id], node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumP::kTag:
        Utils::UpdateStringList(pending_updates_[id], node, kTag, value.toStringList(), &Node::tag, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumP::kDescription:
        Utils::UpdateField(pending_updates_[id], node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumP::kNote:
        Utils::UpdateField(pending_updates_[id], node, kNote, value.toString(), &Node::note, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumP::kPaymentTerm:
        Utils::UpdateField(pending_updates_[id], d_node, kPaymentTerm, value.toInt(), &NodeP::payment_term, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumP::kColor:
        Utils::UpdateField(pending_updates_[id], node, kColor, value.toString(), &Node::color, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumP::kId:
    case NodeEnumP::kUpdateBy:
    case NodeEnumP::kUpdateTime:
    case NodeEnumP::kCreateTime:
    case NodeEnumP::kCreateBy:
    case NodeEnumP::kVersion:
    case NodeEnumP::kUserId:
    case NodeEnumP::kName:
    case NodeEnumP::kKind:
    case NodeEnumP::kUnit:
    case NodeEnumP::kInitialTotal:
        return false;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    return true;
}

Qt::ItemFlags TreeModelP::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const NodeEnumP column { index.column() };

    switch (column) {
    case NodeEnumP::kName:
        flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        flags &= ~Qt::ItemIsEditable;
        break;
    case NodeEnumP::kInitialTotal:
    case NodeEnumP::kUnit:
    case NodeEnumP::kColor:
    case NodeEnumP::kTag:
    case NodeEnumP::kKind:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}
