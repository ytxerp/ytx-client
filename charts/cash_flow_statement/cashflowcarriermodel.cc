#include "cashflowcarriermodel.h"

#include <QJsonArray>

#include "cashflowstatementenum.h"
#include "component/constantbool.h"
#include "global/resourcepool.h"
#include "utils/nodeutils.h"
#include "utils/templateutils.h"

CashFlowCarrierModel::CashFlowCarrierModel(const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
    , header_ { header }
{
    InitFixedNodes();
}

CashFlowCarrierModel::~CashFlowCarrierModel()
{
    ResourcePool<CashFlowStatementRow>::Instance().Recycle(root_);
    ResourcePool<CashFlowStatementRow>::Instance().Recycle(carrier_list_);
    ResourcePool<CashFlowStatementRow>::Instance().Recycle(counterpart_list_);
    ResourcePool<CashFlowStatementRow>::Instance().Recycle(first_level_nodes_);
    ResourcePool<CashFlowStatementRow>::Instance().Recycle(second_level_nodes_);
}

QVariant CashFlowCarrierModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return header_.at(section);
    }

    return QVariant();
}

QModelIndex CashFlowCarrierModel::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex CashFlowCarrierModel::parent(const QModelIndex& index) const
{
    // root_'s index is QModelIndex(), root_'s id == -1
    if (!index.isValid()) {
        qDebug() << Q_FUNC_INFO << "invalid index";
        return {};
    }

    auto* node { static_cast<CashFlowStatementRow*>(index.internalPointer()) };
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

int CashFlowCarrierModel::rowCount(const QModelIndex& parent) const { return GetNodeByIndex(parent)->children.size(); }

int CashFlowCarrierModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return header_.size();
}

QVariant CashFlowCarrierModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* node { static_cast<CashFlowStatementRow*>(index.internalPointer()) };
    Q_ASSERT(node != nullptr);

    const CashFlowStatementEnum column { index.column() };

    switch (column) {
    case CashFlowStatementEnum::kName:
        return node->name;
    case CashFlowStatementEnum::kId:
        return node->id;
    case CashFlowStatementEnum::kCode:
        return node->code;
    case CashFlowStatementEnum::kDescription:
        return node->description;
    case CashFlowStatementEnum::kDirectionRule:
        return node->direction_rule;
    case CashFlowStatementEnum::kFinalTotal:
        return node->final_total;
    }
}

void CashFlowCarrierModel::sort(int column, Qt::SortOrder order)
{
    const CashFlowStatementEnum e_column { column };

    auto Compare = [e_column, order](const CashFlowStatementRow* lhs, const CashFlowStatementRow* rhs) -> bool {
        switch (e_column) {
        case CashFlowStatementEnum::kName:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementRow::name, order);
        case CashFlowStatementEnum::kCode:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementRow::code, order);
        case CashFlowStatementEnum::kDescription:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementRow::description, order);
        case CashFlowStatementEnum::kDirectionRule:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementRow::direction_rule, order);
        case CashFlowStatementEnum::kFinalTotal:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementRow::final_total, order);
        case CashFlowStatementEnum::kId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    node::SortSubtree(root_, Compare);
    emit layoutChanged();
}

void CashFlowCarrierModel::ResetModel(CJsonArray& carrier_array, CJsonArray& counterpart_array)
{
    const auto carrier_list { AddRowsList(carrier_array) };
    const auto counterpart_list { AddRowsList(counterpart_array) };

    beginResetModel();

    {
        ResourcePool<CashFlowStatementRow>::Instance().Recycle(carrier_list_);
        ResourcePool<CashFlowStatementRow>::Instance().Recycle(counterpart_list_);

        for (auto* node : std::as_const(second_level_nodes_)) {
            node->children.clear();
        }

        counterpart_->children.clear();
    }

    carrier_list_ = carrier_list;
    counterpart_list_ = counterpart_list;

    BuildCarrierHierarchy();
    BuildCounterPartHierarchy();

    sort(std::to_underlying(CashFlowStatementEnum::kName), Qt::AscendingOrder);
    endResetModel();
}

CashFlowStatementRow* CashFlowCarrierModel::GetNodeByIndex(const QModelIndex& index) const
{
    if (index.isValid() && index.internalPointer())
        return static_cast<CashFlowStatementRow*>(index.internalPointer());

    return root_;
}

void CashFlowCarrierModel::InitFixedNodes()
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

QList<CashFlowStatementRow*> CashFlowCarrierModel::AddRowsList(const CJsonArray& node_array)
{
    if (node_array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty node array";
    }

    QList<CashFlowStatementRow*> list {};

    for (const QJsonValue& val : node_array) {
        const QJsonObject obj { val.toObject() };
        auto* node { ResourcePool<CashFlowStatementRow>::Instance().Allocate() };
        node->ReadJson(obj);
        list.emplaceBack(node);
    }

    return list;
}

void CashFlowCarrierModel::BuildCarrierHierarchy() const
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

void CashFlowCarrierModel::BuildCounterPartHierarchy() const
{
    for (auto* node : std::as_const(counterpart_list_)) {
        node->parent = counterpart_;
        counterpart_->children.emplaceBack(node);
    }
}

CashFlowStatementRow* CashFlowCarrierModel::FindCarrierGroup(finance::Roles roles) const
{
    if (roles.testFlag(finance::Role::kCash))
        return cash_;

    if (roles.testFlag(finance::Role::kBank))
        return bank_;

    if (roles.testFlag(finance::Role::kWallet))
        return wallet_;

    return nullptr;
}

CashFlowStatementRow* CashFlowCarrierModel::CreateBranchNode(const QString& name, finance::Roles roles, bool direction_rule) const
{
    auto* node { ResourcePool<CashFlowStatementRow>::Instance().Allocate() };

    node->id = QUuid::createUuidV7();

    node->name = name;
    node->roles = roles;
    node->direction_rule = direction_rule;

    return node;
}
