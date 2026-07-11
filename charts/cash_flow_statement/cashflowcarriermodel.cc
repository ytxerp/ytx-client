#include "cashflowcarriermodel.h"

#include <QJsonArray>

#include "cashflowstatementenum.h"
#include "component/constantbool.h"
#include "global/resourcepool.h"
#include "utils/nodeutils.h"
#include "utils/templateutils.h"

namespace cash_flow {

CarrierModel::CarrierModel(const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
    , header_ { header }
{
    InitFixedNodes();
}

CarrierModel::~CarrierModel()
{
    ResourcePool<Row>::Instance().Recycle(root_);
    ResourcePool<Row>::Instance().Recycle(carrier_list_);
    ResourcePool<Row>::Instance().Recycle(counterpart_list_);
    ResourcePool<Row>::Instance().Recycle(first_level_nodes_);
    ResourcePool<Row>::Instance().Recycle(second_level_nodes_);
}

QVariant CarrierModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return header_.at(section);
    }

    return QVariant();
}

QModelIndex CarrierModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return {};

    auto* parent_node { GetNodeByIndex(parent) };
    if (!parent_node) {
        qDebug() << "index: parent node not found";
        return {};
    }

    auto* node { parent_node->children.at(row) };
    if (!node) {
        qDebug() << "index: child node at row" << row << "is null";
        return {};
    }

    return createIndex(row, column, node);
}

QModelIndex CarrierModel::parent(const QModelIndex& index) const
{
    // root_'s index is QModelIndex(), root_'s id == -1
    if (!index.isValid()) {
        qDebug() << Q_FUNC_INFO << "invalid index";
        return {};
    }

    auto* node { static_cast<Row*>(index.internalPointer()) };
    if (!node) {
        qDebug() << Q_FUNC_INFO << "null node from internalPointer";
        return {};
    }

    // CashFlowStatementRow has no parent or parent is root
    auto* parent { node->parent };
    if (!parent) {
        qDebug() << Q_FUNC_INFO << "node has no parent:" << node->name;
        return {};
    }

    if (parent == root_) {
        return {};
    }

    // Parent node should have a parent (grandparent)
    auto* grandparent { parent->parent };
    if (!grandparent) {
        qDebug() << Q_FUNC_INFO << "parent has no grandparent:" << parent->name;
        return {};
    }

    // Find parent's row in grandparent's children
    const qsizetype row { grandparent->children.indexOf(parent) };
    if (row < 0) {
        qDebug() << Q_FUNC_INFO << "parent not found in grandparent children"
                 << "parent =" << parent->name << "grandparent =" << grandparent->name << "child_count =" << grandparent->children.size();

        Q_ASSERT(row >= 0);
        return {};
    }

    return createIndex(row, 0, parent);
}

int CarrierModel::rowCount(const QModelIndex& parent) const { return GetNodeByIndex(parent)->children.size(); }

int CarrierModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return header_.size();
}

QVariant CarrierModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* node { static_cast<Row*>(index.internalPointer()) };
    Q_ASSERT(node != nullptr);

    const RowField column { index.column() };

    switch (column) {
    case RowField::kName:
        return node->name;
    case RowField::kId:
        return node->id;
    case RowField::kCode:
        return node->code;
    case RowField::kDescription:
        return node->description;
    case RowField::kDirectionRule:
        return node->direction_rule;
    case RowField::kFinalTotal:
        return node->final_total;
    }
}

void CarrierModel::sort(int column, Qt::SortOrder order)
{
    const RowField e_column { column };

    auto Compare = [e_column, order](const Row* lhs, const Row* rhs) -> bool {
        switch (e_column) {
        case RowField::kName:
            return utils::CompareMember(lhs, rhs, &Row::name, order);
        case RowField::kCode:
            return utils::CompareMember(lhs, rhs, &Row::code, order);
        case RowField::kDescription:
            return utils::CompareMember(lhs, rhs, &Row::description, order);
        case RowField::kDirectionRule:
            return utils::CompareMember(lhs, rhs, &Row::direction_rule, order);
        case RowField::kFinalTotal:
            return utils::CompareMember(lhs, rhs, &Row::final_total, order);
        case RowField::kId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    node::SortSubtree(root_, Compare);
    emit layoutChanged();
}

void CarrierModel::Rebuild(CJsonArray& carrier_array, CJsonArray& counterpart_array)
{
    const auto carrier_list { AddRowsList(carrier_array) };
    const auto counterpart_list { AddRowsList(counterpart_array) };

    beginResetModel();

    {
        ResourcePool<Row>::Instance().Recycle(carrier_list_);
        ResourcePool<Row>::Instance().Recycle(counterpart_list_);

        for (auto* node : std::as_const(second_level_nodes_)) {
            node->children.clear();
        }

        counterpart_->children.clear();
    }

    carrier_list_ = carrier_list;
    counterpart_list_ = counterpart_list;

    BuildCarrierHierarchy();
    BuildCounterPartHierarchy();

    sort(std::to_underlying(RowField::kName), Qt::AscendingOrder);
    endResetModel();
}

Row* CarrierModel::GetNodeByIndex(const QModelIndex& index) const
{
    if (index.isValid() && index.internalPointer())
        return static_cast<Row*>(index.internalPointer());

    return root_;
}

void CarrierModel::InitFixedNodes()
{
    root_ = CreateBranchNode(QString(), finance::Role::kNone, direction_rule::kDICD);

    auto* carrier { CreateBranchNode(tr("Carrier"), finance::Role::kNone, direction_rule::kDICD) };
    counterpart_ = CreateBranchNode(tr("Counterpart"), finance::Role::kNone, direction_rule::kDDCI);

    {
        root_->children = {
            counterpart_,
            carrier,
        };

        counterpart_->parent = root_;
        carrier->parent = root_;

        first_level_nodes_.emplaceBack(counterpart_);
        first_level_nodes_.emplaceBack(carrier);
    }

    {
        cash_ = CreateBranchNode(tr("Cash"), finance::Role::kCash, direction_rule::kDICD);
        bank_ = CreateBranchNode(tr("Bank"), finance::Role::kBank, direction_rule::kDICD);
        wallet_ = CreateBranchNode(tr("Wallet"), finance::Role::kWallet, direction_rule::kDICD);

        carrier->children = { cash_, bank_, wallet_ };
        cash_->parent = carrier;
        bank_->parent = carrier;
        wallet_->parent = carrier;

        second_level_nodes_.emplaceBack(cash_);
        second_level_nodes_.emplaceBack(bank_);
        second_level_nodes_.emplaceBack(wallet_);
    }
}

QList<Row*> CarrierModel::AddRowsList(const CJsonArray& node_array)
{
    if (node_array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty node array";
    }

    QList<Row*> list {};

    for (const QJsonValue& val : node_array) {
        const QJsonObject obj { val.toObject() };
        auto* node { ResourcePool<Row>::Instance().Allocate() };
        node->ReadJson(obj);
        list.emplaceBack(node);
    }

    return list;
}

void CarrierModel::BuildCarrierHierarchy() const
{
    // Attach nodes without parent to virtual root
    for (auto* node : std::as_const(carrier_list_)) {
        if (!node->parent) {
            auto* root { FindCarrierGroup(node->roles) };

            if (!root) {
                qWarning() << Q_FUNC_INFO << "Cannot find group node for roles";
                continue;
            }

            root->children.emplaceBack(node);
            node->parent = root;
        }
    }
}

void CarrierModel::BuildCounterPartHierarchy() const
{
    for (auto* node : std::as_const(counterpart_list_)) {
        node->parent = counterpart_;
        counterpart_->children.emplaceBack(node);
    }
}

Row* CarrierModel::FindCarrierGroup(finance::Roles roles) const
{
    if (roles.testFlag(finance::Role::kCash))
        return cash_;

    if (roles.testFlag(finance::Role::kBank))
        return bank_;

    if (roles.testFlag(finance::Role::kWallet))
        return wallet_;

    return nullptr;
}

Row* CarrierModel::CreateBranchNode(const QString& name, finance::Roles roles, bool direction_rule) const
{
    auto* node { ResourcePool<Row>::Instance().Allocate() };

    node->id = QUuid::createUuidV7();

    node->name = name;
    node->roles = roles;
    node->direction_rule = direction_rule;

    return node;
}
}
