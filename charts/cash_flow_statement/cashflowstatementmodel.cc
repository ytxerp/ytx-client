#include "cashflowstatementmodel.h"

#include <QJsonArray>

#include "cashflowstatementenum.h"
#include "component/constantbool.h"
#include "global/resourcepool.h"
#include "utils/nodeutils.h"
#include "utils/templateutils.h"

CashFlowStatementModel::CashFlowStatementModel(const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
    , header_ { header }
{
    InitFixedNodes();
}

CashFlowStatementModel::~CashFlowStatementModel()
{
    ResourcePool<CashFlowStatementRow>::Instance().Recycle(carrier_node_hash_);
    ResourcePool<CashFlowStatementRow>::Instance().Recycle(category_node_hash_);
    ResourcePool<CashFlowStatementRow>::Instance().Recycle(first_level_nodes_);
    ResourcePool<CashFlowStatementRow>::Instance().Recycle(second_level_nodes_);
}

QVariant CashFlowStatementModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return header_.at(section);
    }

    return QVariant();
}

QModelIndex CashFlowStatementModel::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex CashFlowStatementModel::parent(const QModelIndex& index) const
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

int CashFlowStatementModel::rowCount(const QModelIndex& parent) const { return GetNodeByIndex(parent)->children.size(); }

int CashFlowStatementModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return header_.size();
}

QVariant CashFlowStatementModel::data(const QModelIndex& index, int role) const
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
    case CashFlowStatementEnum::kKind:
        return std::to_underlying(node->kind);
    case CashFlowStatementEnum::kFinalTotal:
        return node->final_total;
    }
}

void CashFlowStatementModel::sort(int column, Qt::SortOrder order)
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
        case CashFlowStatementEnum::kKind:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementRow::kind, order);
        case CashFlowStatementEnum::kFinalTotal:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementRow::final_total, order);
        case CashFlowStatementEnum::kId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    utils::SortIterative(root_, Compare);
    emit layoutChanged();
}

void CashFlowStatementModel::ResetModel(CJsonArray& node_array, CJsonArray& carrier_array)
{
    const auto category_node_hash { AddRowsHash(node_array) };
    const auto carrier_node_hash { AddRowsHash(carrier_array) };

    beginResetModel();

    {
        ResourcePool<CashFlowStatementRow>::Instance().Recycle(category_node_hash_);
        ResourcePool<CashFlowStatementRow>::Instance().Recycle(carrier_node_hash_);

        for (auto* node : std::as_const(second_level_nodes_)) {
            node->children.clear();
        }
    }

    category_node_hash_ = category_node_hash;
    carrier_node_hash_ = carrier_node_hash;

    BuildCategoryHierarchy();
    BuildCarrierHierarchy();

    for (auto* node : std::as_const(category_node_hash_)) {
        UpdateAncestorTotal(node, node->final_total);
    }

    sort(std::to_underlying(CashFlowStatementEnum::kName), Qt::AscendingOrder);
    endResetModel();
}

CashFlowStatementRow* CashFlowStatementModel::GetNodeByIndex(const QModelIndex& index) const
{
    if (index.isValid() && index.internalPointer())
        return static_cast<CashFlowStatementRow*>(index.internalPointer());

    return root_;
}

void CashFlowStatementModel::InitFixedNodes()
{
    root_ = CreateBranchNode(QString(), finance::CashKind::kNone, direction_rule::kDICD);

    auto* operating { CreateBranchNode(tr("Operating"), finance::CashKind::kNone, direction_rule::kDDCI) };
    auto* investing { CreateBranchNode(tr("Investing"), finance::CashKind::kNone, direction_rule::kDDCI) };
    auto* financing { CreateBranchNode(tr("Financing"), finance::CashKind::kNone, direction_rule::kDDCI) };

    auto* carrier { CreateBranchNode(tr("Carrier"), finance::CashKind::kNone, direction_rule::kDICD) };
    auto* cash { CreateBranchNode(tr("Cash"), finance::Role::kCash, direction_rule::kDICD) };
    auto* bank { CreateBranchNode(tr("Bank"), finance::Role::kBank, direction_rule::kDICD) };
    auto* wallet { CreateBranchNode(tr("Wallet"), finance::Role::kWallet, direction_rule::kDICD) };

    {
        first_level_nodes_.emplaceBack(root_);
        first_level_nodes_.emplaceBack(operating);
        first_level_nodes_.emplaceBack(investing);
        first_level_nodes_.emplaceBack(financing);
        first_level_nodes_.emplaceBack(carrier);
    }

    {
        root_->children = {
            operating,
            investing,
            financing,
            carrier,
        };

        operating->parent = root_;
        investing->parent = root_;
        financing->parent = root_;
        carrier->parent = root_;
    }

    {
        carrier->children = { cash, bank, wallet };
        cash->parent = carrier;
        bank->parent = carrier;
        wallet->parent = carrier;
    }

    {
        second_level_nodes_.emplaceBack(cash);
        second_level_nodes_.emplaceBack(bank);
        second_level_nodes_.emplaceBack(wallet);
    }

    {
        auto AddSecondLevelNodes = [&](CashFlowStatementRow* parent, finance::CashKind in_flow, finance::CashKind out_flow) {
            auto* in_node { CreateBranchNode(tr("Inflows"), in_flow, direction_rule::kDDCI) };
            auto* out_node { CreateBranchNode(tr("Outflows"), out_flow, direction_rule::kDICD) };

            second_level_nodes_.emplaceBack(in_node);
            second_level_nodes_.emplaceBack(out_node);

            in_node->parent = parent;
            out_node->parent = parent;

            parent->children = {
                in_node,
                out_node,
            };
        };

        AddSecondLevelNodes(operating, finance::CashKind::kOperatingIn, finance::CashKind::kOperatingOut);
        AddSecondLevelNodes(investing, finance::CashKind::kInvestingIn, finance::CashKind::kInvestingOut);
        AddSecondLevelNodes(financing, finance::CashKind::kFinancingIn, finance::CashKind::kFinancingOut);
    }
}

QHash<QUuid, CashFlowStatementRow*> CashFlowStatementModel::AddRowsHash(const CJsonArray& node_array)
{
    if (node_array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty node array";
    }

    QHash<QUuid, CashFlowStatementRow*> hash {};

    for (const QJsonValue& val : node_array) {
        const QJsonObject obj { val.toObject() };
        auto* node { ResourcePool<CashFlowStatementRow>::Instance().Allocate() };
        node->ReadJson(obj);
        hash.insert(node->id, node);
    }

    return hash;
}

void CashFlowStatementModel::BuildCategoryHierarchy() const
{
    // Attach nodes without parent to virtual root
    for (auto* node : std::as_const(category_node_hash_)) {
        if (!node->parent) {
            auto* root { FindCategoryGroup(node->cash_kind) };

            if (!root) {
                qWarning() << Q_FUNC_INFO << "Cannot find group node for cash kind:" << std::to_underlying(node->cash_kind);
                continue;
            }

            root->children.emplaceBack(node);
            node->parent = root;
        }
    }
}

void CashFlowStatementModel::BuildCarrierHierarchy() const
{
    // Attach nodes without parent to virtual root
    for (auto* node : std::as_const(carrier_node_hash_)) {
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

CashFlowStatementRow* CashFlowStatementModel::FindCategoryGroup(finance::CashKind kind) const
{
    auto it = std::find_if(second_level_nodes_.cbegin(), second_level_nodes_.cend(), [kind](const auto* node) { return node->cash_kind == kind; });

    return it != second_level_nodes_.cend() ? *it : nullptr;
}

CashFlowStatementRow* CashFlowStatementModel::FindCarrierGroup(finance::Roles roles) const
{
    auto it = std::find_if(second_level_nodes_.cbegin(), second_level_nodes_.cend(), [roles](const auto* node) { return roles & node->roles; });

    return it != second_level_nodes_.cend() ? *it : nullptr;
}

void CashFlowStatementModel::UpdateAncestorTotal(CashFlowStatementRow* node, double final_delta) const
{
    if (!node || node == root_ || !node->parent || node->parent == root_)
        return;

    if (FloatEqual(final_delta, 0.0))
        return;

    const bool rule { node->direction_rule };

    for (auto* current = node->parent; current && current != root_; current = current->parent) {
        const int multiplier { current->direction_rule == rule ? 1 : -1 };

        current->final_total += multiplier * final_delta;
    }
}

CashFlowStatementRow* CashFlowStatementModel::CreateBranchNode(const QString& name, finance::CashKind cash_kind, bool direction_rule) const
{
    auto* node { ResourcePool<CashFlowStatementRow>::Instance().Allocate() };

    node->id = QUuid::createUuidV7();
    node->kind = NodeKind::kBranch;

    node->name = name;
    node->cash_kind = cash_kind;
    node->direction_rule = direction_rule;

    return node;
}

CashFlowStatementRow* CashFlowStatementModel::CreateBranchNode(const QString& name, finance::Roles roles, bool direction_rule) const
{
    auto* node { ResourcePool<CashFlowStatementRow>::Instance().Allocate() };

    node->id = QUuid::createUuidV7();
    node->kind = NodeKind::kBranch;

    node->name = name;
    node->roles = roles;
    node->direction_rule = direction_rule;

    return node;
}
