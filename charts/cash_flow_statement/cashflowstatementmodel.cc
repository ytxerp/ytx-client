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
    ResourcePool<CashFlowStatementRow>::Instance().Recycle(root_);
    ResourcePool<CashFlowStatementRow>::Instance().Recycle(node_list_);
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

void CashFlowStatementModel::ResetModel(CJsonArray& node_array)
{
    const auto node_list { AddRowsList(node_array) };

    beginResetModel();

    {
        ResourcePool<CashFlowStatementRow>::Instance().Recycle(node_list_);

        for (auto* node : std::as_const(second_level_nodes_)) {
            node->children.clear();
        }
    }

    node_list_ = node_list;

    BuildHierarchy();

    for (auto* node : std::as_const(node_list_)) {
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

    {
        root_->children = {
            operating,
            investing,
            financing,
        };

        operating->parent = root_;
        investing->parent = root_;
        financing->parent = root_;

        first_level_nodes_.emplaceBack(operating);
        first_level_nodes_.emplaceBack(investing);
        first_level_nodes_.emplaceBack(financing);
    }

    {
        operating_in_ = CreateBranchNode(tr("Inflows"), finance::CashKind::kNone, direction_rule::kDDCI);
        operating_out_ = CreateBranchNode(tr("Outflows"), finance::CashKind::kNone, direction_rule::kDICD);
        investing_in_ = CreateBranchNode(tr("Inflows"), finance::CashKind::kNone, direction_rule::kDDCI);
        investing_out_ = CreateBranchNode(tr("Outflows"), finance::CashKind::kNone, direction_rule::kDICD);
        financing_in_ = CreateBranchNode(tr("Inflows"), finance::CashKind::kNone, direction_rule::kDDCI);
        financing_out_ = CreateBranchNode(tr("Outflows"), finance::CashKind::kNone, direction_rule::kDICD);

        operating->children = { operating_in_, operating_out_ };
        operating_in_->parent = operating;
        operating_out_->parent = operating;

        investing->children = { investing_in_, investing_out_ };
        investing_in_->parent = investing;
        investing_out_->parent = investing;

        financing->children = { financing_in_, financing_out_ };
        financing_in_->parent = financing;
        financing_out_->parent = financing;

        second_level_nodes_.emplaceBack(operating_in_);
        second_level_nodes_.emplaceBack(operating_out_);
        second_level_nodes_.emplaceBack(investing_in_);
        second_level_nodes_.emplaceBack(investing_out_);
        second_level_nodes_.emplaceBack(financing_in_);
        second_level_nodes_.emplaceBack(financing_out_);
    }
}

QList<CashFlowStatementRow*> CashFlowStatementModel::AddRowsList(const CJsonArray& node_array)
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

void CashFlowStatementModel::BuildHierarchy() const
{
    // Attach nodes without parent to virtual root
    for (auto* node : std::as_const(node_list_)) {
        if (!node->parent) {
            auto* root { FindNodeGroup(node->cash_kind) };

            if (!root) {
                qWarning() << Q_FUNC_INFO << "Cannot find group node for cash kind:" << std::to_underlying(node->cash_kind);
                continue;
            }

            root->children.emplaceBack(node);
            node->parent = root;

            node->name = CashKindName(node->cash_kind);
        }
    }
}

CashFlowStatementRow* CashFlowStatementModel::FindNodeGroup(finance::CashKind kind) const
{
    switch (static_cast<int>(kind)) {
    // Operating
    case 100 ... 199:
        return operating_in_;
    case 200 ... 299:
        return operating_out_;

        // Investing
    case 300 ... 399:
        return investing_in_;
    case 400 ... 499:
        return investing_out_;

        // Financing
    case 500 ... 599:
        return financing_in_;
    case 600 ... 699:
        return financing_out_;

    default:
        return nullptr;
    }
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

    node->name = name;
    node->cash_kind = cash_kind;
    node->direction_rule = direction_rule;

    return node;
}
