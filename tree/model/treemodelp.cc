#include "treemodelp.h"

#include "global/collator.h"
#include "tree/includemultiplefiltermodel.h"
#include "utils/compareutils.h"

TreeModelP::TreeModelP(CSectionInfo& info, CString& separator, int default_unit, QObject* parent)
    : TreeModel(info, separator, default_unit, parent)
{
    leaf_path_model_ = new ItemModel(this);
    leaf_path_model_->AppendItem(QString(), QUuid());
}

void TreeModelP::RUpdateAmount(const QUuid& node_id, double initial_delta)
{
    assert(!node_id.isNull());

    if (FloatEqual(initial_delta, 0.0))
        return;

    auto* node { node_hash_.value(node_id) };
    if (!node || node == root_ || node->kind != std::to_underlying(NodeKind::kLeaf))
        return;

    node->initial_total += initial_delta;

    const auto& affected_ids { UpdateAncestorTotal(node, initial_delta, 0.0) };
    RefreshAffectedTotal(affected_ids);
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

QSet<QUuid> TreeModelP::UpdateAncestorTotal(Node* node, double initial_delta, double /*final_delta*/)
{
    QSet<QUuid> affected_ids {};

    if (!node || !node->parent || node->parent == root_)
        return affected_ids;

    if (FloatEqual(initial_delta, 0.0))
        return affected_ids;

    const int unit { node->unit };

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
    assert(column >= 0 && column < node_header_.size());

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
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    NodeUtils::SortIterative(root_, Compare);
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
    case NodeEnumP::kVersion:
        return d_node->version;
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
        NodeUtils::UpdateField(pending_updates_[id], node, kCode, value.toString(), &Node::code, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumP::kDescription:
        NodeUtils::UpdateField(pending_updates_[id], node, kDescription, value.toString(), &Node::description, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumP::kNote:
        NodeUtils::UpdateField(pending_updates_[id], node, kNote, value.toString(), &Node::note, [id, this]() { RestartTimer(id); });
        break;
    case NodeEnumP::kPaymentTerm:
        NodeUtils::UpdateField(pending_updates_[id], d_node, kPaymentTerm, value.toInt(), &NodeP::payment_term, [id, this]() { RestartTimer(id); });
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
