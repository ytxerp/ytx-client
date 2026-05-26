#include "cashflowstatementmodel.h"

#include <QJsonArray>
#include <QTimer>

#include "cashflowstatementenum.h"
#include "component/constantbool.h"
#include "component/constantstring.h"
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
    ResourcePool<CashFlowStatementRow>::Instance().Recycle(node_hash_);
    ResourcePool<CashFlowStatementRow>::Instance().Recycle(fixed_nodes_);
    ResourcePool<CashFlowStatementRow>::Instance().Recycle(cash_flow_group_nodes_);
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
    case CashFlowStatementEnum::kCashKind:
        return std::to_underlying(node->cash_kind);
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
        case CashFlowStatementEnum::kCashKind:
            return utils::CompareMember(lhs, rhs, &CashFlowStatementRow::cash_kind, order);
        case CashFlowStatementEnum::kId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    utils::SortIterative(root_, Compare);
    emit layoutChanged();
}

void CashFlowStatementModel::ResetModel(CJsonArray& operating_node, CJsonArray& operating_path, CJsonArray& investing_node, CJsonArray& investing_path,
    CJsonArray& finance_node, CJsonArray& finance_path)
{
    const auto operating_hash { AddServerRows(operating_node, operating_path) };
    const auto investing_hash { AddServerRows(investing_node, investing_path) };
    const auto finance_hash { AddServerRows(finance_node, finance_path) };

    beginResetModel();

    {
        ResourcePool<CashFlowStatementRow>::Instance().Recycle(node_hash_);
        for (auto* node : std::as_const(cash_flow_group_nodes_)) {
            node->children.clear();
        }

        node_hash_.insert(operating_hash);
        node_hash_.insert(investing_hash);
        node_hash_.insert(finance_hash);
    }

    {
        BuildHierarchy(operating_hash, operating_path);
        BuildHierarchy(investing_hash, investing_path);
        BuildHierarchy(finance_hash, finance_path);
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
    root_ = CreateBranchNode(string_const::kEmpty, finance::CashKind::kNone, direction_rule::kDICD);

    auto* operating { CreateBranchNode(tr("Operating"), finance::CashKind::kNone, direction_rule::kDDCI) };
    auto* investing { CreateBranchNode(tr("Investing"), finance::CashKind::kNone, direction_rule::kDDCI) };
    auto* financing { CreateBranchNode(tr("Financing"), finance::CashKind::kNone, direction_rule::kDDCI) };

    {
        fixed_nodes_.emplaceBack(root_);
        fixed_nodes_.emplaceBack(operating);
        fixed_nodes_.emplaceBack(investing);
        fixed_nodes_.emplaceBack(financing);
    }

    {
        root_->children = {
            operating,
            investing,
            financing,
        };

        operating->parent = root_;
        investing->parent = root_;
        financing->parent = root_;
    }

    {
        auto AddSecondLevelNodes = [&](CashFlowStatementRow* parent, finance::CashKind in_flow, finance::CashKind out_flow) {
            auto* in_node { CreateBranchNode(tr("Inflows"), in_flow, direction_rule::kDDCI) };
            auto* out_node { CreateBranchNode(tr("Outflows"), out_flow, direction_rule::kDICD) };

            cash_flow_group_nodes_.emplaceBack(in_node);
            cash_flow_group_nodes_.emplaceBack(out_node);

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

QHash<QUuid, CashFlowStatementRow*> CashFlowStatementModel::AddServerRows(const CJsonArray& node_array, const CJsonArray& path_array)
{
    if (node_array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty node array";
    }

    if (path_array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty path array";
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

void CashFlowStatementModel::BuildHierarchy(const QHash<QUuid, CashFlowStatementRow*>& hash, CJsonArray& path_array)
{
    for (const QJsonValue& val : path_array) {
        const QJsonObject obj { val.toObject() };

        const QUuid ancestor_id { QUuid(obj.value(kAncestor).toString()) };
        const QUuid descendant_id { QUuid(obj.value(kDescendant).toString()) };

        auto* ancestor { hash.value(ancestor_id, nullptr) };
        auto* descendant { hash.value(descendant_id, nullptr) };

        assert(ancestor && "ancestor not found in node_hash_");
        assert(descendant && "descendant not found in node_hash_");

        ancestor->children.emplaceBack(descendant);
        descendant->parent = ancestor;
    }

    // Attach nodes without parent to virtual root
    for (auto* node : std::as_const(hash)) {
        if (!node->parent) {
            auto* root { FindGroupNode(node->cash_kind) };

            if (!root) {
                qWarning() << Q_FUNC_INFO << "Cannot find group node for cash kind:" << std::to_underlying(node->cash_kind);
                continue;
            }

            root->children.emplaceBack(node);
            node->parent = root;
        }
    }
}

CashFlowStatementRow* CashFlowStatementModel::FindGroupNode(finance::CashKind kind) const
{
    for (auto* node : std::as_const(cash_flow_group_nodes_)) {
        if (node->cash_kind == kind) {
            return node;
        }
    }

    return nullptr;
}

CashFlowStatementRow* CashFlowStatementModel::CreateBranchNode(const QString& name, finance::CashKind cash_kind, bool direction_rule)
{
    auto* node = ResourcePool<CashFlowStatementRow>::Instance().Allocate();

    node->id = QUuid::createUuid();
    node->kind = NodeKind::kBranch;

    node->name = name;
    node->cash_kind = cash_kind;
    node->direction_rule = direction_rule;

    return node;
}
