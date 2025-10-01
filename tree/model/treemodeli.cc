#include "treemodeli.h"

#include "global/nodepool.h"
#include "tree/excludemultiplefiltermodel.h"
#include "tree/includemultiplefiltermodel.h"

TreeModelI::TreeModelI(CSectionInfo& info, CString& separator, int default_unit, QObject* parent)
    : TreeModel(info, separator, default_unit, parent)
{
    leaf_model_->AppendItem(QString(), QUuid());
}

TreeModelI::~TreeModelI() { NodePool::Instance().Recycle(node_hash_, section_); }

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

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs { DerivedPtr<NodeI>(lhs) };
        auto* d_rhs { DerivedPtr<NodeI>(rhs) };

        const NodeEnumI kColumn { column };
        switch (kColumn) {
        case NodeEnumI::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case NodeEnumI::kUserId:
            return (order == Qt::AscendingOrder) ? (lhs->user_id < rhs->user_id) : (lhs->user_id > rhs->user_id);
        case NodeEnumI::kCreateTime:
            return (order == Qt::AscendingOrder) ? (lhs->created_time < rhs->created_time) : (lhs->created_time > rhs->created_time);
        case NodeEnumI::kCreateBy:
            return (order == Qt::AscendingOrder) ? (lhs->created_by < rhs->created_by) : (lhs->created_by > rhs->created_by);
        case NodeEnumI::kUpdateTime:
            return (order == Qt::AscendingOrder) ? (lhs->updated_time < rhs->updated_time) : (lhs->updated_time > rhs->updated_time);
        case NodeEnumI::kUpdateBy:
            return (order == Qt::AscendingOrder) ? (lhs->updated_by < rhs->updated_by) : (lhs->updated_by > rhs->updated_by);
        case NodeEnumI::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case NodeEnumI::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case NodeEnumI::kNote:
            return (order == Qt::AscendingOrder) ? (lhs->note < rhs->note) : (lhs->note > rhs->note);
        case NodeEnumI::kDirectionRule:
            return (order == Qt::AscendingOrder) ? (lhs->direction_rule < rhs->direction_rule) : (lhs->direction_rule > rhs->direction_rule);
        case NodeEnumI::kKind:
            return (order == Qt::AscendingOrder) ? (lhs->kind < rhs->kind) : (lhs->kind > rhs->kind);
        case NodeEnumI::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case NodeEnumI::kColor:
            return (order == Qt::AscendingOrder) ? (d_lhs->color < d_rhs->color) : (d_lhs->color > d_rhs->color);
        case NodeEnumI::kCommission:
            return (order == Qt::AscendingOrder) ? (d_lhs->commission < d_rhs->commission) : (d_lhs->commission > d_rhs->commission);
        case NodeEnumI::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (d_lhs->unit_price < d_rhs->unit_price) : (d_lhs->unit_price > d_rhs->unit_price);
        case NodeEnumI::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        case NodeEnumI::kFinalTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    SortIterative(root_, Compare);
    emit layoutChanged();
}

QVariant TreeModelI::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_node { DerivedPtr<NodeI>(GetNodeByIndex(index)) };
    if (d_node == root_)
        return QVariant();

    const NodeEnumI kColumn { index.column() };
    const bool is_leaf { d_node->kind == kLeaf };

    switch (kColumn) {
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
    case NodeEnumI::kCode:
        return d_node->code;
    case NodeEnumI::kDescription:
        return d_node->description;
    case NodeEnumI::kNote:
        return d_node->note;
    case NodeEnumI::kDirectionRule:
        return d_node->direction_rule;
    case NodeEnumI::kKind:
        return d_node->kind;
    case NodeEnumI::kUnit:
        return d_node->unit;
    case NodeEnumI::kColor:
        return d_node->color;
    case NodeEnumI::kCommission:
        return is_leaf && d_node->commission != 0 ? d_node->commission : QVariant();
    case NodeEnumI::kUnitPrice:
        return is_leaf && d_node->unit_price != 0 ? d_node->unit_price : QVariant();
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

    if (node == root_)
        return false;

    const NodeEnumI kColumn { index.column() };
    const QUuid id { node->id };
    auto& cache { caches_[id] };

    switch (kColumn) {
    case NodeEnumI::kCode:
        NodeUtils::UpdateField(cache, node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kDescription:
        NodeUtils::UpdateField(cache, node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kNote:
        NodeUtils::UpdateField(cache, node, kNote, value.toString(), &Node::note, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kDirectionRule:
        UpdateDirectionRule(node, value.toBool());
        break;
    case NodeEnumI::kColor:
        NodeUtils::UpdateField(cache, d_node, kColor, value.toString(), &NodeI::color, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kCommission:
        if (d_node->kind == kLeaf)
            NodeUtils::UpdateDouble(cache, d_node, kCommission, value.toDouble(), &NodeI::commission, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumI::kUnitPrice:
        if (d_node->kind == kLeaf)
            NodeUtils::UpdateDouble(cache, d_node, kUnitPrice, value.toDouble(), &NodeI::unit_price, [id, this]() { RestartTimer(id); });
        break;
    default:
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
    const NodeEnumI kColumn { index.column() };

    switch (kColumn) {
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

QSortFilterProxyModel* TreeModelI::IncludeUnitModel(int unit)
{
    auto* set { UnitSet(unit) };
    auto* model { new IncludeMultipleFilterModel(set, this) };
    model->setSourceModel(leaf_model_);
    QObject::connect(this, &TreeModel::SSyncFilterModel, model, &IncludeMultipleFilterModel::RSyncFilterModel);
    return model;
}

QSortFilterProxyModel* TreeModelI::ExcludeMultipleModel(const QUuid& node_id, int unit)
{
    auto* set { UnitSet(unit) };
    auto* model { new ExcludeMultipleFilterModel(node_id, set, this) };
    model->setSourceModel(leaf_model_);
    QObject::connect(this, &TreeModel::SSyncFilterModel, model, &ExcludeMultipleFilterModel::RSyncFilterModel);
    return model;
}

void TreeModelI::ResetColor(const QModelIndex& index)
{
    auto* node { GetNodeByIndex(index) };
    auto* d_node { DerivedPtr<NodeI>(node) };
    const QUuid id { d_node->id };

    d_node->color.clear();
    emit dataChanged(index.siblingAtColumn(std::to_underlying(NodeEnumI::kColor)), index.siblingAtColumn(std::to_underlying(NodeEnumI::kColor)));

    auto& cache { caches_[d_node->id] };
    RestartTimer(id);
    cache.insert(kColor, QString());
}
