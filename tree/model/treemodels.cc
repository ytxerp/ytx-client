#include "treemodels.h"

#include "global/nodepool.h"
#include "tree/includemultiplefiltermodel.h"

TreeModelS::TreeModelS(CSectionInfo& info, CString& separator, int default_unit, QObject* parent)
    : TreeModel(info, separator, default_unit, parent)
{
    leaf_model_->AppendItem(QString(), QUuid());
}

TreeModelS::~TreeModelS() { NodePool::Instance().Recycle(node_hash_, section_); }

void TreeModelS::RUpdateAmount(const QUuid& node_id, double initial_delta, double final_delta)
{
    assert(!node_id.isNull());

    if (initial_delta == 0.0 && final_delta == 0.0)
        return;

    auto* node { node_hash_.value(node_id) };
    if (!node || node == root_ || node->kind != kLeaf)
        return;

    node->initial_total += initial_delta;
    node->final_total += final_delta;
    UpdateAncestorValue(node, initial_delta, final_delta);
}

QList<QUuid> TreeModelS::PartyList(CString& text, int unit) const
{
    QList<QUuid> list {};

    for (auto* node : node_hash_)
        if (node->unit == unit && node->name.contains(text))
            list.emplaceBack(node->id);

    return list;
}

const QSet<QUuid>* TreeModelS::UnitSet(int unit) const
{
    const UnitS kUnit { unit };

    switch (kUnit) {
    case UnitS::kCust:
        return &cset_;
    case UnitS::kVend:
        return &vset_;
    case UnitS::kEmp:
        return &eset_;
    default:
        return nullptr;
    }
}

QSortFilterProxyModel* TreeModelS::IncludeUnitModel(int unit)
{
    auto* set { UnitSet(unit) };
    auto* model { new IncludeMultipleFilterModel(set, this) };
    model->setSourceModel(leaf_model_);
    QObject::connect(this, &TreeModel::SSyncFilterModel, model, &IncludeMultipleFilterModel::RSyncFilterModel);
    return model;
}

void TreeModelS::RemoveUnitSet(const QUuid& node_id, int unit)
{
    const UnitS kUnit { unit };

    switch (kUnit) {
    case UnitS::kCust:
        cset_.remove(node_id);
        break;
    case UnitS::kVend:
        vset_.remove(node_id);
        break;
    case UnitS::kEmp:
        eset_.remove(node_id);
        break;
    default:
        break;
    }
}

void TreeModelS::InsertUnitSet(const QUuid& node_id, int unit)
{
    const UnitS kUnit { unit };

    switch (kUnit) {
    case UnitS::kCust:
        cset_.insert(node_id);
        break;
    case UnitS::kVend:
        vset_.insert(node_id);
        break;
    case UnitS::kEmp:
        eset_.insert(node_id);
        break;
    default:
        break;
    }
}

void TreeModelS::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column < node_header_.size());

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeS>(lhs);
        auto* d_rhs = DerivedPtr<NodeS>(rhs);

        const NodeEnumS kColumn { column };
        switch (kColumn) {
        case NodeEnumS::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case NodeEnumS::kUserId:
            return (order == Qt::AscendingOrder) ? (lhs->user_id < rhs->user_id) : (lhs->user_id > rhs->user_id);
        case NodeEnumS::kCreateTime:
            return (order == Qt::AscendingOrder) ? (lhs->created_time < rhs->created_time) : (lhs->created_time > rhs->created_time);
        case NodeEnumS::kCreateBy:
            return (order == Qt::AscendingOrder) ? (lhs->created_by < rhs->created_by) : (lhs->created_by > rhs->created_by);
        case NodeEnumS::kUpdateTime:
            return (order == Qt::AscendingOrder) ? (lhs->updated_time < rhs->updated_time) : (lhs->updated_time > rhs->updated_time);
        case NodeEnumS::kUpdateBy:
            return (order == Qt::AscendingOrder) ? (lhs->updated_by < rhs->updated_by) : (lhs->updated_by > rhs->updated_by);
        case NodeEnumS::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case NodeEnumS::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case NodeEnumS::kNote:
            return (order == Qt::AscendingOrder) ? (lhs->note < rhs->note) : (lhs->note > rhs->note);
        case NodeEnumS::kKind:
            return (order == Qt::AscendingOrder) ? (lhs->kind < rhs->kind) : (lhs->kind > rhs->kind);
        case NodeEnumS::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case NodeEnumS::kPaymentTerm:
            return (order == Qt::AscendingOrder) ? (d_lhs->payment_term < d_rhs->payment_term) : (d_lhs->payment_term > d_rhs->payment_term);
        case NodeEnumS::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        case NodeEnumS::kFinalTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    SortIterative(root_, Compare);
    emit layoutChanged();
}

QVariant TreeModelS::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_node { DerivedPtr<NodeS>(GetNodeByIndex(index)) };
    if (d_node == root_)
        return QVariant();

    const NodeEnumS kColumn { index.column() };
    const bool kIsLeaf { d_node->kind == kLeaf };

    switch (kColumn) {
    case NodeEnumS::kName:
        return d_node->name;
    case NodeEnumS::kId:
        return d_node->id;
    case NodeEnumS::kUserId:
        return d_node->user_id;
    case NodeEnumS::kCreateTime:
        return d_node->created_time;
    case NodeEnumS::kCreateBy:
        return d_node->created_by;
    case NodeEnumS::kUpdateTime:
        return d_node->updated_time;
    case NodeEnumS::kUpdateBy:
        return d_node->updated_by;
    case NodeEnumS::kCode:
        return d_node->code;
    case NodeEnumS::kDescription:
        return d_node->description;
    case NodeEnumS::kNote:
        return d_node->note;
    case NodeEnumS::kKind:
        return d_node->kind;
    case NodeEnumS::kUnit:
        return d_node->unit;
    case NodeEnumS::kPaymentTerm:
        return kIsLeaf && d_node->payment_term != 0 ? d_node->payment_term : QVariant();
    case NodeEnumS::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumS::kFinalTotal:
        return d_node->final_total;
    default:
        return QVariant();
    }
}

bool TreeModelS::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    auto* node { GetNodeByIndex(index) };
    auto* d_node { DerivedPtr<NodeS>(node) };

    if (node == root_)
        return false;

    const NodeEnumS kColumn { index.column() };
    const QUuid id { node->id };
    auto& cache { caches_[id] };

    switch (kColumn) {
    case NodeEnumS::kCode:
        NodeUtils::UpdateField(cache, node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumS::kDescription:
        NodeUtils::UpdateField(cache, node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumS::kNote:
        NodeUtils::UpdateField(cache, node, kNote, value.toString(), &Node::note, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumS::kPaymentTerm:
        if (d_node->kind == kLeaf)
            NodeUtils::UpdateField(cache, d_node, kPaymentTerm, value.toInt(), &NodeS::payment_term, [id, this]() { RestartTimer(id); });
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());
    return true;
}

Qt::ItemFlags TreeModelS::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const NodeEnumS kColumn { index.column() };

    switch (kColumn) {
    case NodeEnumS::kName:
        flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        flags &= ~Qt::ItemIsEditable;
        break;
    case NodeEnumS::kFinalTotal:
    case NodeEnumS::kInitialTotal:
    case NodeEnumS::kUnit:
    case NodeEnumS::kKind:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}
