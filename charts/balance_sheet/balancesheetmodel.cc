#include "balancesheetmodel.h"

#include "balancesheetenum.h"
#include "global/resourcepool.h"
#include "utils/nodeutils.h"
#include "utils/templateutils.h"

BalanceSheetModel::BalanceSheetModel(const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
    , header_ { header }
{
    root_ = ResourcePool<BalanceSheetRow>::Instance().Allocate();
    root_->kind = NodeKind::kBranch;
    root_->direction_rule = false;
    root_->name = QString();
    root_->id = QUuid();
}

BalanceSheetModel::~BalanceSheetModel()
{
    ResourcePool<BalanceSheetRow>::Instance().Recycle(node_hash_);
    ResourcePool<BalanceSheetRow>::Instance().Recycle(root_);
}

QVariant BalanceSheetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return header_.at(section);
    }

    return QVariant();
}

QModelIndex BalanceSheetModel::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex BalanceSheetModel::parent(const QModelIndex& index) const
{
    // root_'s index is QModelIndex(), root_'s id == -1
    if (!index.isValid()) {
        qDebug() << Q_FUNC_INFO << "invalid index";
        return {};
    }

    auto* node { static_cast<BalanceSheetRow*>(index.internalPointer()) };
    if (!node) {
        qDebug() << Q_FUNC_INFO << "null node from internalPointer";
        return {};
    }

    // BalanceSheetRow has no parent or parent is root
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

int BalanceSheetModel::rowCount(const QModelIndex& parent) const { return GetNodeByIndex(parent)->children.size(); }

int BalanceSheetModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return header_.size();
}

QVariant BalanceSheetModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* node { static_cast<BalanceSheetRow*>(index.internalPointer()) };
    Q_ASSERT(node != nullptr);

    const BalanceSheetEnum column { index.column() };

    switch (column) {
    case BalanceSheetEnum::kName:
        return node->name;
    case BalanceSheetEnum::kId:
        return node->id;
    case BalanceSheetEnum::kCode:
        return node->code;
    case BalanceSheetEnum::kDescription:
        return node->description;
    case BalanceSheetEnum::kDirectionRule:
        return node->direction_rule;
    case BalanceSheetEnum::kKind:
        return std::to_underlying(node->kind);
    case BalanceSheetEnum::kClosingBalance:
        return node->closing_balance;
    case BalanceSheetEnum::kOpeningBalance:
        return node->opening_balance;
    case BalanceSheetEnum::kChangeAmount:
        return node->change_amount;
    case BalanceSheetEnum::kChangeRate:
        return node->change_rate;
    }
}

void BalanceSheetModel::sort(int column, Qt::SortOrder order)
{
    const BalanceSheetEnum e_column { column };

    auto Compare = [e_column, order](const BalanceSheetRow* lhs, const BalanceSheetRow* rhs) -> bool {
        switch (e_column) {
        case BalanceSheetEnum::kName:
            return utils::CompareMember(lhs, rhs, &BalanceSheetRow::name, order);
        case BalanceSheetEnum::kCode:
            return utils::CompareMember(lhs, rhs, &BalanceSheetRow::code, order);
        case BalanceSheetEnum::kDescription:
            return utils::CompareMember(lhs, rhs, &BalanceSheetRow::description, order);
        case BalanceSheetEnum::kDirectionRule:
            return utils::CompareMember(lhs, rhs, &BalanceSheetRow::direction_rule, order);
        case BalanceSheetEnum::kKind:
            return utils::CompareMember(lhs, rhs, &BalanceSheetRow::kind, order);
        case BalanceSheetEnum::kClosingBalance:
            return utils::CompareMember(lhs, rhs, &BalanceSheetRow::closing_balance, order);
        case BalanceSheetEnum::kOpeningBalance:
            return utils::CompareMember(lhs, rhs, &BalanceSheetRow::opening_balance, order);
        case BalanceSheetEnum::kChangeAmount:
            return utils::CompareMember(lhs, rhs, &BalanceSheetRow::change_amount, order);
        case BalanceSheetEnum::kChangeRate:
            return utils::CompareMember(lhs, rhs, &BalanceSheetRow::change_rate, order);
        case BalanceSheetEnum::kId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    node::SortIterative(root_, Compare);
    emit layoutChanged();
}

void BalanceSheetModel::ResetModel(const QJsonArray& node_array, const QJsonArray& path_array)
{
    if (node_array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty node array";
    }

    if (path_array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty path array";
    }

    // Build new hash outside the model reset lock
    QHash<QUuid, BalanceSheetRow*> new_hash;
    for (const QJsonValue& val : node_array) {
        const QJsonObject obj { val.toObject() };
        BalanceSheetRow* node { ResourcePool<BalanceSheetRow>::Instance().Allocate() };
        node->ReadJson(obj);
        new_hash.insert(node->id, node);
    }

    beginResetModel();

    {
        ResourcePool<BalanceSheetRow>::Instance().Recycle(node_hash_);
        root_->children.clear();
        node_hash_ = std::move(new_hash);
        BuildHierarchy(path_array);
    }

    sort(std::to_underlying(BalanceSheetEnum::kName), Qt::AscendingOrder);
    endResetModel();
}

BalanceSheetRow* BalanceSheetModel::GetNodeByIndex(const QModelIndex& index) const
{
    if (index.isValid() && index.internalPointer())
        return static_cast<BalanceSheetRow*>(index.internalPointer());

    return root_;
}

void BalanceSheetModel::BuildHierarchy(const QJsonArray& path_array)
{
    for (const QJsonValue& val : path_array) {
        const QJsonObject obj { val.toObject() };

        const QUuid ancestor_id { QUuid(obj.value(kAncestor).toString()) };
        const QUuid descendant_id { QUuid(obj.value(kDescendant).toString()) };

        BalanceSheetRow* ancestor { node_hash_.value(ancestor_id, nullptr) };
        BalanceSheetRow* descendant { node_hash_.value(descendant_id, nullptr) };

        assert(ancestor && "ancestor not found in node_hash_");
        assert(descendant && "descendant not found in node_hash_");

        ancestor->children.emplaceBack(descendant);
        descendant->parent = ancestor;
    }

    // Attach nodes without parent to virtual root
    for (BalanceSheetRow* node : std::as_const(node_hash_)) {
        if (!node->parent) {
            root_->children.emplaceBack(node);
            node->parent = root_;
        }
    }
}
