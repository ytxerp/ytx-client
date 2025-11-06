#include "treemodelp.h"

#include "global/nodepool.h"
#include "tree/includemultiplefiltermodel.h"

TreeModelP::TreeModelP(CSectionInfo& info, CString& separator, int default_unit, QObject* parent)
    : TreeModel(info, separator, default_unit, parent)
{
    leaf_path_model_ = new ItemModel(this);
    leaf_path_model_->AppendItem(QString(), QUuid());
}

TreeModelP::~TreeModelP() { NodePool::Instance().Recycle(node_hash_, section_); }

void TreeModelP::RUpdateAmount(const QUuid& node_id, double initial_delta, double final_delta)
{
    assert(!node_id.isNull());

    if (initial_delta == 0.0 && final_delta == 0.0)
        return;

    auto* node { node_hash_.value(node_id) };
    if (!node || node == root_ || node->kind != std::to_underlying(NodeKind::kLeaf))
        return;

    node->initial_total += initial_delta;
    node->final_total += final_delta;
    SyncAncestorTotal(node, initial_delta, final_delta);
}

QList<QUuid> TreeModelP::PartnerList(CString& text, int unit) const
{
    QList<QUuid> list {};

    for (auto* node : node_hash_)
        if (node->unit == unit && node->name.contains(text))
            list.emplaceBack(node->id);

    return list;
}

const QSet<QUuid>* TreeModelP::UnitSet(int unit) const
{
    const UnitP kUnit { unit };

    switch (kUnit) {
    case UnitP::kCustomer:
        return &cset_;
    case UnitP::kVendor:
        return &vset_;
    case UnitP::kEmployee:
        return &eset_;
    default:
        return nullptr;
    }
}

QSortFilterProxyModel* TreeModelP::IncludeUnitModel(int unit, QObject* parent)
{
    auto* set { UnitSet(unit) };
    auto* model { new IncludeMultipleFilterModel(set, parent) };
    model->setSourceModel(leaf_path_model_);
    return model;
}

void TreeModelP::RemoveUnitSet(const QUuid& node_id, int unit)
{
    const UnitP kUnit { unit };

    switch (kUnit) {
    case UnitP::kCustomer:
        cset_.remove(node_id);
        break;
    case UnitP::kVendor:
        vset_.remove(node_id);
        break;
    case UnitP::kEmployee:
        eset_.remove(node_id);
        break;
    default:
        break;
    }
}

void TreeModelP::InsertUnitSet(const QUuid& node_id, int unit)
{
    const UnitP kUnit { unit };

    switch (kUnit) {
    case UnitP::kCustomer:
        cset_.insert(node_id);
        break;
    case UnitP::kVendor:
        vset_.insert(node_id);
        break;
    case UnitP::kEmployee:
        eset_.insert(node_id);
        break;
    default:
        break;
    }
}

void TreeModelP::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column < node_header_.size());

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        auto* d_lhs = DerivedPtr<NodeP>(lhs);
        auto* d_rhs = DerivedPtr<NodeP>(rhs);

        const NodeEnumP e_column { column };
        switch (e_column) {
        case NodeEnumP::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case NodeEnumP::kUserId:
            return (order == Qt::AscendingOrder) ? (lhs->user_id < rhs->user_id) : (lhs->user_id > rhs->user_id);
        case NodeEnumP::kCreateTime:
            return (order == Qt::AscendingOrder) ? (lhs->created_time < rhs->created_time) : (lhs->created_time > rhs->created_time);
        case NodeEnumP::kCreateBy:
            return (order == Qt::AscendingOrder) ? (lhs->created_by < rhs->created_by) : (lhs->created_by > rhs->created_by);
        case NodeEnumP::kUpdateTime:
            return (order == Qt::AscendingOrder) ? (lhs->updated_time < rhs->updated_time) : (lhs->updated_time > rhs->updated_time);
        case NodeEnumP::kUpdateBy:
            return (order == Qt::AscendingOrder) ? (lhs->updated_by < rhs->updated_by) : (lhs->updated_by > rhs->updated_by);
        case NodeEnumP::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case NodeEnumP::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case NodeEnumP::kNote:
            return (order == Qt::AscendingOrder) ? (lhs->note < rhs->note) : (lhs->note > rhs->note);
        case NodeEnumP::kKind:
            return (order == Qt::AscendingOrder) ? (lhs->kind < rhs->kind) : (lhs->kind > rhs->kind);
        case NodeEnumP::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case NodeEnumP::kPaymentTerm:
            return (order == Qt::AscendingOrder) ? (d_lhs->payment_term < d_rhs->payment_term) : (d_lhs->payment_term > d_rhs->payment_term);
        case NodeEnumP::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        case NodeEnumP::kFinalTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    SortIterative(root_, Compare);
    emit layoutChanged();
}

QVariant TreeModelP::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_node { DerivedPtr<NodeP>(GetNodeByIndex(index)) };
    if (d_node == root_)
        return QVariant();

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
    case NodeEnumP::kCode:
        return d_node->code;
    case NodeEnumP::kDescription:
        return d_node->description;
    case NodeEnumP::kNote:
        return d_node->note;
    case NodeEnumP::kKind:
        return d_node->kind;
    case NodeEnumP::kUnit:
        return d_node->unit;
    case NodeEnumP::kPaymentTerm:
        return d_node->payment_term;
    case NodeEnumP::kInitialTotal:
        return d_node->initial_total;
    case NodeEnumP::kFinalTotal:
        return d_node->final_total;
    default:
        return QVariant();
    }
}

bool TreeModelP::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    auto* node { GetNodeByIndex(index) };

    auto* d_node { DerivedPtr<NodeP>(node) };
    if (!d_node)
        return false;

    const NodeEnumP column { index.column() };
    const QUuid id { node->id };

    switch (column) {
    case NodeEnumP::kCode:
        NodeUtils::UpdateField(caches_[id], node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumP::kDescription:
        NodeUtils::UpdateField(caches_[id], node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumP::kNote:
        NodeUtils::UpdateField(caches_[id], node, kNote, value.toString(), &Node::note, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumP::kPaymentTerm:
        NodeUtils::UpdateField(caches_[id], d_node, kPaymentTerm, value.toInt(), &NodeP::payment_term, [id, this]() { RestartTimer(id); });
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());
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
    case NodeEnumP::kFinalTotal:
    case NodeEnumP::kInitialTotal:
    case NodeEnumP::kUnit:
    case NodeEnumP::kKind:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}
