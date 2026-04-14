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
            return utils::CompareMember(lhs, rhs, &Node::name, order);
        case NodeEnumI::kCode:
            return utils::CompareMember(lhs, rhs, &Node::code, order);
        case NodeEnumI::kDescription:
            return utils::CompareMember(lhs, rhs, &Node::description, order);
        case NodeEnumI::kDirectionRule:
            return utils::CompareMember(lhs, rhs, &Node::direction_rule, order);
        case NodeEnumI::kKind:
            return utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumI::kUnit:
            return utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumI::kColor:
            return utils::CompareMember(lhs, rhs, &Node::color, order);
        case NodeEnumI::kCommission:
            return utils::CompareMember(d_lhs, d_rhs, &NodeI::commission, order);
        case NodeEnumI::kUnitPrice:
            return utils::CompareMember(d_lhs, d_rhs, &NodeI::unit_price, order);
        case NodeEnumI::kInitialTotal:
            return utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumI::kFinalTotal:
            return utils::CompareMember(lhs, rhs, &Node::final_total, order);
        case NodeEnumI::kTag:
            return utils::CompareMember(lhs, rhs, &Node::tag, order);
        case NodeEnumI::kDocument:
            return (order == Qt::AscendingOrder) ? (d_lhs->document.size() < d_rhs->document.size()) : (d_lhs->document.size() > d_rhs->document.size());
        case NodeEnumI::kId:
        case NodeEnumI::kVersion:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    utils::SortIterative(root_, Compare);
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
    case NodeEnumI::kDocument:
        return d_node->document;
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
        utils::UpdateField(pending_updates_[id], node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kDescription:
        utils::UpdateField(pending_updates_[id], node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kTag:
        utils::UpdateStringList(pending_updates_[id], node, kTag, value.toStringList(), &Node::tag, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kDirectionRule:
        UpdateDirectionRule(node, value.toBool(), index);
        break;
    case NodeEnumI::kColor:
        utils::UpdateField(pending_updates_[id], node, kColor, value.toString(), &Node::color, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kCommission:
        utils::UpdateDouble(pending_updates_[id], d_node, kCommission, value.toDouble(), &NodeI::commission, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kUnitPrice:
        utils::UpdateDouble(pending_updates_[id], d_node, kUnitPrice, value.toDouble(), &NodeI::unit_price, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kDocument:
        utils::UpdateStringList(pending_updates_[id], node, kDocument, value.toStringList(), &Node::document, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kId:
    case NodeEnumI::kVersion:
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
    case NodeEnumI::kDocument:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}
