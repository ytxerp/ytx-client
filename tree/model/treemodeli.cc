#include "treemodeli.h"

TreeModelI::TreeModelI(CSectionInfo& info, CString& separator, QObject* parent)
    : TreeModel(info, separator, parent)
{
    leaf_path_model_ = new ItemModel(this);
    leaf_path_model_->AppendItem(QString(), QUuid());
}

QSet<QUuid>* TreeModelI::UnitSet(NodeUnit unit)
{
    switch (unit) {
    case NodeUnit::IPosition:
        return &pos_set_;
    case NodeUnit::IInternal:
        return &int_set_;
    case NodeUnit::IExternal:
        return &ext_set_;
    default:
        return nullptr;
    }
}

void TreeModelI::sort(int column, Qt::SortOrder order)
{
    const NodeEnumI e_column { column };

    auto Compare = [e_column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs { DerivedPtr<NodeI>(lhs) };
        auto* d_rhs { DerivedPtr<NodeI>(rhs) };

        switch (e_column) {
        case NodeEnumI::kName:
            return Utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumI::kCode:
            return Utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumI::kDescription:
            return Utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumI::kDirectionRule:
            return Utils::CompareMember(lhs, rhs, &Node::direction_rule, order);
        case NodeEnumI::kKind:
            return Utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumI::kUnit:
            return Utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumI::kColor:
            return Utils::CompareMember(lhs, rhs, &Node::color, order);
        case NodeEnumI::kCommission:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeI::commission, order);
        case NodeEnumI::kUnitPrice:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeI::unit_price, order);
        case NodeEnumI::kInitialTotal:
            return Utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumI::kFinalTotal:
            return Utils::CompareMember(lhs, rhs, &Node::final_total, order);
        case NodeEnumI::kTag:
            return Utils::CompareMember(lhs, rhs, &Node::tag, order);
        case NodeEnumI::kId:
        case NodeEnumI::kUpdateBy:
        case NodeEnumI::kUpdateTime:
        case NodeEnumI::kCreateTime:
        case NodeEnumI::kCreateBy:
        case NodeEnumI::kVersion:
        case NodeEnumI::kUserId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    Utils::SortIterative(root_, Compare);
    emit layoutChanged();
}

QVariant TreeModelI::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* d_node { static_cast<NodeI*>(index.internalPointer()) };
    Q_ASSERT(d_node != nullptr);

    const NodeEnumI column { index.column() };

    switch (column) {
    case NodeEnumI::kName:
        return d_node->name;
    case NodeEnumI::kId:
        return d_node->id;
    case NodeEnumI::kUserId:
        return d_node->user_id;
    case NodeEnumI::kCreateTime:
        return d_node->created_time;
    case NodeEnumI::kCreateBy:
        return d_node->created_by;
    case NodeEnumI::kUpdateTime:
        return d_node->updated_time;
    case NodeEnumI::kUpdateBy:
        return d_node->updated_by;
    case NodeEnumI::kVersion:
        return d_node->version;
    case NodeEnumI::kCode:
        return d_node->code;
    case NodeEnumI::kTag:
        return d_node->tag;
    case NodeEnumI::kDescription:
        return d_node->description;
    case NodeEnumI::kDirectionRule:
        return d_node->direction_rule;
    case NodeEnumI::kKind:
        return std::to_underlying(d_node->kind);
    case NodeEnumI::kUnit:
        return std::to_underlying(d_node->unit);
    case NodeEnumI::kColor:
        return d_node->color;
    case NodeEnumI::kCommission:
        return d_node->commission;
    case NodeEnumI::kUnitPrice:
        return d_node->unit_price;
    case NodeEnumI::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumI::kFinalTotal:
        return d_node->final_total;
    }
}

bool TreeModelI::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    auto* node { static_cast<Node*>(index.internalPointer()) };
    auto* d_node { static_cast<NodeI*>(node) };

    Q_ASSERT(node != nullptr);
    Q_ASSERT(d_node != nullptr);

    const NodeEnumI column { index.column() };
    const QUuid id { node->id };

    switch (column) {
    case NodeEnumI::kCode:
        Utils::UpdateField(pending_updates_[id], node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kDescription:
        Utils::UpdateField(pending_updates_[id], node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kTag:
        Utils::UpdateStringList(pending_updates_[id], node, kTag, value.toStringList(), &Node::tag, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kDirectionRule:
        UpdateDirectionRule(node, value.toBool(), index);
        break;
    case NodeEnumI::kColor:
        Utils::UpdateField(pending_updates_[id], node, kColor, value.toString(), &Node::color, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kCommission:
        Utils::UpdateDouble(pending_updates_[id], d_node, kCommission, value.toDouble(), &NodeI::commission, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kUnitPrice:
        Utils::UpdateDouble(pending_updates_[id], d_node, kUnitPrice, value.toDouble(), &NodeI::unit_price, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kId:
    case NodeEnumI::kUpdateBy:
    case NodeEnumI::kUpdateTime:
    case NodeEnumI::kCreateTime:
    case NodeEnumI::kCreateBy:
    case NodeEnumI::kVersion:
    case NodeEnumI::kUserId:
    case NodeEnumI::kName:
    case NodeEnumI::kKind:
    case NodeEnumI::kUnit:
    case NodeEnumI::kInitialTotal:
    case NodeEnumI::kFinalTotal:
        return false;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    return true;
}

Qt::ItemFlags TreeModelI::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const NodeEnumI column { index.column() };

    switch (column) {
    case NodeEnumI::kName:
        flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        flags &= ~Qt::ItemIsEditable;
        break;
    case NodeEnumI::kInitialTotal:
    case NodeEnumI::kFinalTotal:
    case NodeEnumI::kColor:
    case NodeEnumI::kTag:
    case NodeEnumI::kKind:
    case NodeEnumI::kUnit:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}
