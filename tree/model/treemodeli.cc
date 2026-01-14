#include "treemodeli.h"

#include "tree/excludemultiplefiltermodel.h"
#include "tree/includemultiplefiltermodel.h"
#include "utils/compareutils.h"

TreeModelI::TreeModelI(CSectionInfo& info, CString& separator, QObject* parent)
    : TreeModel(info, separator, parent)
{
    leaf_path_model_ = new ItemModel(this);
    leaf_path_model_->AppendItem(QString(), QUuid());
}

void TreeModelI::RemoveUnitSet(const QUuid& node_id, int unit)
{
    const UnitI kUnit { unit };

    switch (kUnit) {
    case UnitI::kPosition:
        pos_set_.remove(node_id);
        break;
    case UnitI::kInternal:
        int_set_.remove(node_id);
        break;
    case UnitI::kExternal:
        ext_set_.remove(node_id);
        break;
    default:
        break;
    }
}

void TreeModelI::InsertUnitSet(const QUuid& node_id, int unit)
{
    const UnitI kUnit { unit };

    switch (kUnit) {
    case UnitI::kPosition:
        pos_set_.insert(node_id);
        break;
    case UnitI::kInternal:
        int_set_.insert(node_id);
        break;
    case UnitI::kExternal:
        ext_set_.insert(node_id);
        break;
    default:
        break;
    }
}

const QSet<QUuid>* TreeModelI::UnitSet(int unit) const
{
    const UnitI kUnit { unit };

    switch (kUnit) {
    case UnitI::kPosition:
        return &pos_set_;
    case UnitI::kInternal:
        return &int_set_;
    case UnitI::kExternal:
        return &ext_set_;
    default:
        return nullptr;
    }
}

void TreeModelI::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column < node_header_.size());

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
        case NodeEnumI::kNote:
            return Utils::CompareMember(lhs, rhs, &Node::note, order);
        case NodeEnumI::kDirectionRule:
            return Utils::CompareMember(lhs, rhs, &Node::direction_rule, order);
        case NodeEnumI::kKind:
            return Utils::CompareMember(lhs, rhs, &Node::kind, order);
        case NodeEnumI::kUnit:
            return Utils::CompareMember(lhs, rhs, &Node::unit, order);
        case NodeEnumI::kColor:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeI::color, order);
        case NodeEnumI::kCommission:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeI::commission, order);
        case NodeEnumI::kUnitPrice:
            return Utils::CompareMember(d_lhs, d_rhs, &NodeI::unit_price, order);
        case NodeEnumI::kInitialTotal:
            return Utils::CompareMember(lhs, rhs, &Node::initial_total, order);
        case NodeEnumI::kFinalTotal:
            return Utils::CompareMember(lhs, rhs, &Node::final_total, order);
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
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_node { DerivedPtr<NodeI>(GetNodeByIndex(index)) };
    if (d_node == root_)
        return QVariant();

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
    case NodeEnumI::kDescription:
        return d_node->description;
    case NodeEnumI::kNote:
        return d_node->note;
    case NodeEnumI::kDirectionRule:
        return d_node->direction_rule;
    case NodeEnumI::kKind:
        return std::to_underlying(d_node->kind);
    case NodeEnumI::kUnit:
        return d_node->unit;
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
    default:
        return QVariant();
    }
}

bool TreeModelI::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    auto* node { GetNodeByIndex(index) };

    auto* d_node { DerivedPtr<NodeI>(node) };
    if (!d_node)
        return false;

    const NodeEnumI column { index.column() };
    const QUuid id { node->id };

    switch (column) {
    case NodeEnumI::kCode:
        Utils::UpdateField(pending_updates_[id], node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kDescription:
        Utils::UpdateField(pending_updates_[id], node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kNote:
        Utils::UpdateField(pending_updates_[id], node, kNote, value.toString(), &Node::note, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kDirectionRule:
        UpdateDirectionRule(node, value.toBool());
        break;
    case NodeEnumI::kColor:
        Utils::UpdateField(pending_updates_[id], d_node, kColor, value.toString(), &NodeI::color, [id, this]() { RestartTimer(id); });
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

    emit SResizeColumnToContents(index.column());
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

QSortFilterProxyModel* TreeModelI::IncludeUnitModel(int unit, QObject* parent)
{
    auto* set { UnitSet(unit) };
    auto* model { new IncludeMultipleFilterModel(set, parent) };
    model->setSourceModel(leaf_path_model_);
    return model;
}

QSortFilterProxyModel* TreeModelI::ExcludeMultipleModel(const QUuid& node_id, int unit, QObject* parent)
{
    auto* set { UnitSet(unit) };
    auto* model { new ExcludeMultipleFilterModel(node_id, set, parent) };
    model->setSourceModel(leaf_path_model_);
    return model;
}

void TreeModelI::ResetColor(const QModelIndex& index)
{
    auto* node { GetNodeByIndex(index) };
    auto* d_node { DerivedPtr<NodeI>(node) };
    const QUuid id { d_node->id };

    d_node->color.clear();
    emit dataChanged(index.siblingAtColumn(std::to_underlying(NodeEnumI::kColor)), index.siblingAtColumn(std::to_underlying(NodeEnumI::kColor)));

    auto& update { pending_updates_[d_node->id] };
    RestartTimer(id);
    update.insert(kColor, QString());
}
