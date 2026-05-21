#include "incomestatementmodel.h"

#include <QJsonArray>

#include "global/resourcepool.h"
#include "incomestatementenum.h"
#include "utils/nodeutils.h"
#include "utils/templateutils.h"

IncomeStatementModel::IncomeStatementModel(const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
    , header_ { header }
{
    root_ = ResourcePool<IncomeStatementRow>::Instance().Allocate();
    root_->kind = NodeKind::kBranch;
    root_->direction_rule = false;
    root_->name = QString();
    root_->id = QUuid();
}

IncomeStatementModel::~IncomeStatementModel()
{
    ResourcePool<IncomeStatementRow>::Instance().Recycle(node_hash_);
    ResourcePool<IncomeStatementRow>::Instance().Recycle(root_);
}

QVariant IncomeStatementModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return header_.at(section);
    }

    return QVariant();
}

QModelIndex IncomeStatementModel::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex IncomeStatementModel::parent(const QModelIndex& index) const
{
    // root_'s index is QModelIndex(), root_'s id == -1
    if (!index.isValid()) {
        qDebug() << Q_FUNC_INFO << "invalid index";
        return {};
    }

    auto* node { static_cast<IncomeStatementRow*>(index.internalPointer()) };
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

int IncomeStatementModel::rowCount(const QModelIndex& parent) const { return GetNodeByIndex(parent)->children.size(); }

int IncomeStatementModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return header_.size();
}

QVariant IncomeStatementModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* node { static_cast<IncomeStatementRow*>(index.internalPointer()) };
    Q_ASSERT(node != nullptr);

    const IncomeStatementEnum column { index.column() };

    switch (column) {
    case IncomeStatementEnum::kName:
        return node->name;
    case IncomeStatementEnum::kId:
        return node->id;
    case IncomeStatementEnum::kCode:
        return node->code;
    case IncomeStatementEnum::kDescription:
        return node->description;
    case IncomeStatementEnum::kDirectionRule:
        return node->direction_rule;
    case IncomeStatementEnum::kKind:
        return std::to_underlying(node->kind);
    case IncomeStatementEnum::kFinalTotal:
        return node->final_total;
    }
}

void IncomeStatementModel::sort(int column, Qt::SortOrder order)
{
    const IncomeStatementEnum e_column { column };

    auto Compare = [e_column, order](const IncomeStatementRow* lhs, const IncomeStatementRow* rhs) -> bool {
        switch (e_column) {
        case IncomeStatementEnum::kName:
            return utils::CompareMember(lhs, rhs, &IncomeStatementRow::name, order);
        case IncomeStatementEnum::kCode:
            return utils::CompareMember(lhs, rhs, &IncomeStatementRow::code, order);
        case IncomeStatementEnum::kDescription:
            return utils::CompareMember(lhs, rhs, &IncomeStatementRow::description, order);
        case IncomeStatementEnum::kDirectionRule:
            return utils::CompareMember(lhs, rhs, &IncomeStatementRow::direction_rule, order);
        case IncomeStatementEnum::kKind:
            return utils::CompareMember(lhs, rhs, &IncomeStatementRow::kind, order);
        case IncomeStatementEnum::kFinalTotal:
            return utils::CompareMember(lhs, rhs, &IncomeStatementRow::final_total, order);
        case IncomeStatementEnum::kId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    utils::SortIterative(root_, Compare);
    emit layoutChanged();
}

void IncomeStatementModel::ResetModel(const QJsonArray& node_array, const QJsonArray& path_array)
{
    if (node_array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty node array";
    }

    if (path_array.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty path array";
    }

    // Build new hash outside the model reset lock
    QHash<QUuid, IncomeStatementRow*> new_hash;
    for (const QJsonValue& val : node_array) {
        const QJsonObject obj { val.toObject() };
        IncomeStatementRow* node { ResourcePool<IncomeStatementRow>::Instance().Allocate() };
        node->ReadJson(obj);
        new_hash.insert(node->id, node);
    }

    beginResetModel();

    {
        ResourcePool<IncomeStatementRow>::Instance().Recycle(node_hash_);
        root_->children.clear();
        node_hash_ = std::move(new_hash);
        BuildHierarchy(path_array);
    }

    sort(std::to_underlying(IncomeStatementEnum::kName), Qt::AscendingOrder);
    endResetModel();
}

IncomeStatementRow* IncomeStatementModel::GetNodeByIndex(const QModelIndex& index) const
{
    if (index.isValid() && index.internalPointer())
        return static_cast<IncomeStatementRow*>(index.internalPointer());

    return root_;
}

void IncomeStatementModel::BuildHierarchy(const QJsonArray& path_array)
{
    for (const QJsonValue& val : path_array) {
        const QJsonObject obj { val.toObject() };

        const QUuid ancestor_id { QUuid(obj.value(kAncestor).toString()) };
        const QUuid descendant_id { QUuid(obj.value(kDescendant).toString()) };

        IncomeStatementRow* ancestor { node_hash_.value(ancestor_id, nullptr) };
        IncomeStatementRow* descendant { node_hash_.value(descendant_id, nullptr) };

        assert(ancestor && "ancestor not found in node_hash_");
        assert(descendant && "descendant not found in node_hash_");

        ancestor->children.emplaceBack(descendant);
        descendant->parent = ancestor;
    }

    // Attach nodes without parent to virtual root
    for (IncomeStatementRow* node : std::as_const(node_hash_)) {
        if (!node->parent) {
            root_->children.emplaceBack(node);
            node->parent = root_;
        }
    }
}
