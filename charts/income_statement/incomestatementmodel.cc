#include "incomestatementmodel.h"

#include <QJsonArray>

#include "global/resourcepool.h"
#include "incomestatementenum.h"
#include "utils/nodeutils.h"
#include "utils/templateutils.h"

namespace income_statement {

Model::Model(const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
    , header_ { header }
{
    InitFixedNodes();
}

Model::~Model()
{
    ResourcePool<Row>::Instance().Recycle(node_hash_);
    ResourcePool<Row>::Instance().Recycle(root_);
    ResourcePool<Row>::Instance().Recycle(net_profit_);
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return {};
    }

    if (role == Qt::DisplayRole) {
        return header_.at(section);
    }

    if (role == Qt::ToolTipRole) {
        switch (static_cast<RowField>(section)) {
        case RowField::kYoyFinalTotal:
        case RowField::kYoyGrowthRate:
            return yoy_tooltip_;
        case RowField::kMomFinalTotal:
        case RowField::kMomGrowthRate:
            return mom_tooltip_;
        case RowField::kName:
        case RowField::kId:
        case RowField::kCode:
        case RowField::kDescription:
        case RowField::kDirectionRule:
        case RowField::kKind:
        case RowField::kFinalTotal:
            return {};
        }
    }

    return {};
}

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex Model::parent(const QModelIndex& index) const
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

    // IncomeStatementRow has no parent or parent is root
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

int Model::rowCount(const QModelIndex& parent) const { return GetNodeByIndex(parent)->children.size(); }

int Model::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return header_.size();
}

QVariant Model::data(const QModelIndex& index, int role) const
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
    case RowField::kKind:
        return std::to_underlying(node->kind);
    case RowField::kFinalTotal:
        return node->final_total;
    case RowField::kYoyFinalTotal:
        return node->yoy_final_total;
    case RowField::kMomFinalTotal:
        return node->mom_final_total;
    case RowField::kYoyGrowthRate:
        return node->yoy_growth_rate;
    case RowField::kMomGrowthRate:
        return node->mom_growth_rate;
    }
}

void Model::sort(int column, Qt::SortOrder order)
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
        case RowField::kKind:
            return utils::CompareMember(lhs, rhs, &Row::kind, order);
        case RowField::kFinalTotal:
            return utils::CompareMember(lhs, rhs, &Row::final_total, order);
        case RowField::kId:
            return false;
        case RowField::kYoyFinalTotal:
            return utils::CompareMember(lhs, rhs, &Row::yoy_final_total, order);
        case RowField::kMomFinalTotal:
            return utils::CompareMember(lhs, rhs, &Row::mom_final_total, order);
        case RowField::kYoyGrowthRate:
            return utils::CompareMember(lhs, rhs, &Row::yoy_growth_rate, order);
        case RowField::kMomGrowthRate:
            return utils::CompareMember(lhs, rhs, &Row::mom_growth_rate, order);
        }
    };

    emit layoutAboutToBeChanged();
    node::SortSubtree(net_profit_, Compare);
    emit layoutChanged();
}

void Model::ResetModel(
    const QJsonArray& node_array, const QJsonArray& path_array, double net_profit, double yoy_net_profit, double mom_net_profit)
{
    if (node_array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty node array";
    }

    if (path_array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty path array";
    }

    // Build new hash outside the model reset lock
    QHash<QUuid, Row*> new_hash;
    for (const QJsonValue& val : node_array) {
        const QJsonObject obj { val.toObject() };
        Row* node { ResourcePool<Row>::Instance().Allocate() };
        node->ReadJson(obj);
        new_hash.insert(node->id, node);
    }

    beginResetModel();

    {
        ResourcePool<Row>::Instance().Recycle(node_hash_);
        net_profit_->children.clear();
        node_hash_ = std::move(new_hash);
        BuildHierarchy(path_array);
        net_profit_->final_total = net_profit;
        net_profit_->yoy_final_total = yoy_net_profit;
        net_profit_->mom_final_total = mom_net_profit;

        net_profit_->yoy_growth_rate = utils::GrowthRate(net_profit, yoy_net_profit);
        net_profit_->mom_growth_rate = utils::GrowthRate(net_profit, mom_net_profit);
    }

    sort(std::to_underlying(RowField::kName), Qt::AscendingOrder);
    endResetModel();
}

void Model::UpdateHeaderTooltip(const QString& yoy_tooltip, const QString& mom_tooltip)
{
    yoy_tooltip_ = yoy_tooltip;
    mom_tooltip_ = mom_tooltip;

    emit headerDataChanged(Qt::Horizontal, static_cast<int>(RowField::kYoyFinalTotal), static_cast<int>(RowField::kMomGrowthRate));
}

Row* Model::GetNodeByIndex(const QModelIndex& index) const
{
    if (index.isValid() && index.internalPointer())
        return static_cast<Row*>(index.internalPointer());

    return root_;
}

void Model::BuildHierarchy(const QJsonArray& path_array)
{
    for (const QJsonValue& val : path_array) {
        const QJsonObject obj { val.toObject() };

        const QUuid ancestor_id { QUuid(obj.value(kAncestor).toString()) };
        const QUuid descendant_id { QUuid(obj.value(kDescendant).toString()) };

        Row* ancestor { node_hash_.value(ancestor_id, nullptr) };
        Row* descendant { node_hash_.value(descendant_id, nullptr) };

        assert(ancestor && "ancestor not found in node_hash_");
        assert(descendant && "descendant not found in node_hash_");

        ancestor->children.emplaceBack(descendant);
        descendant->parent = ancestor;
    }

    // Attach nodes without parent to virtual root
    for (Row* node : std::as_const(node_hash_)) {
        if (!node->parent) {
            net_profit_->children.emplaceBack(node);
            node->parent = net_profit_;
        }
    }
}

void Model::InitFixedNodes()
{
    root_ = CreateBranchNode(QString(), false);
    net_profit_ = CreateBranchNode(tr("Net Profit"), direction_rule::kDDCI);

    root_->children.emplaceBack(net_profit_);
    net_profit_->parent = root_;
}

Row* Model::CreateBranchNode(const QString& name, bool direction_rule) const
{
    auto* node { ResourcePool<Row>::Instance().Allocate() };

    node->id = QUuid::createUuidV7();
    node->kind = NodeKind::kBranch;

    node->name = name;
    node->direction_rule = direction_rule;

    return node;
}
}
