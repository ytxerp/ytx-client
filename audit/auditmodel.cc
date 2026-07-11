#include "auditmodel.h"

#include "auditenum.h"
#include "enum/section.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

namespace audit {

Model::Model(const Info& info, QObject* parent)
    : QAbstractItemModel(parent)
    , info_ { info }
{
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.header.at(section);

    return QVariant();
}

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QVariant Model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const RowField column { index.column() };
    const auto* row { static_cast<Row*>(index.internalPointer()) };

    switch (column) {
    case RowField::kTargetId:
        return row->target_id.toString(QUuid::WithoutBraces).left(12);
    case RowField::kUserId:
        return info_.user_hash.value(row->user_id);
    case RowField::kCreatedTime:
        return row->created_time;
    case RowField::kCode:
        return row->code;
    case RowField::kBefore:
        return JsonValueToString(row->before);
    case RowField::kAfter:
        return JsonValueToString(row->after);
    case RowField::kId:
        return row->id;
    case RowField::kSection:
        return info_.section_hash.value(row->section);
    case RowField::kOperation:
        return info_.ws_key_hash.value(row->operation);
    case RowField::kLevel:
        return info_.level_hash.value(row->level);
    case RowField::kTarget:
        return info_.target_type_hash.value(row->target);
    case RowField::kLhsNode:
        return ResolveNode(row, row->lhs_node);
    case RowField::kRhsNode:
        return ResolveNode(row, row->rhs_node);
    }
}

void Model::sort(int column, Qt::SortOrder order)
{
    // Convert integer column to the structured enum using brace initialization
    const RowField e_column { column };

    // Define a lambda for comparison based on the selected column and sort order
    auto Compare = [order, e_column](const Row* lhs, const Row* rhs) -> bool {
        switch (e_column) {
        case RowField::kTargetId:
            return utils::CompareMember(lhs, rhs, &Row::target_id, order);
        case RowField::kUserId:
            return utils::CompareMember(lhs, rhs, &Row::user_id, order);
        case RowField::kLhsNode:
            return utils::CompareMember(lhs, rhs, &Row::lhs_node, order);
        case RowField::kRhsNode:
            return utils::CompareMember(lhs, rhs, &Row::rhs_node, order);
        case RowField::kCode:
            return utils::CompareMember(lhs, rhs, &Row::code, order);
        case RowField::kSection:
            return utils::CompareMember(lhs, rhs, &Row::section, order);
        case RowField::kOperation:
            return utils::CompareMember(lhs, rhs, &Row::operation, order);
        case RowField::kTarget:
            return utils::CompareMember(lhs, rhs, &Row::target, order);
        case RowField::kLevel:
            return utils::CompareMember(lhs, rhs, &Row::level, order);
        case RowField::kCreatedTime:
            return utils::CompareMember(lhs, rhs, &Row::created_time, order);
        case RowField::kId:
        case RowField::kBefore:
        case RowField::kAfter:
            return false;
        }
    };

    // Notify the view that the layout is about to change
    emit layoutAboutToBeChanged();

    // Perform the sort on the underlying data list
    std::sort(list_.begin(), list_.end(), Compare);

    // Notify the view that the layout has been updated
    emit layoutChanged();
}

void Model::Rebuild(const QJsonArray& array)
{
    if (array.isEmpty()) {
        qWarning() << "[AuditModel]" << "Received empty array";
    }

    // Parse outside the reset block
    QList<Row*> new_list {};
    new_list.reserve(array.size());

    for (const auto& value : array) {
        if (!value.isObject()) {
            qWarning() << "[AuditModel]" << "Invalid data, expected object:" << value;
            continue;
        }

        auto* entry { ResourcePool<Row>::Instance().Allocate() };
        entry->ReadJson(value.toObject());
        new_list.emplaceBack(entry);
    }

    // Keep reset block as short as possible
    beginResetModel();
    ResourcePool<Row>::Instance().Recycle(list_);
    list_ = std::move(new_list);
    sort(std::to_underlying(RowField::kCreatedTime), Qt::AscendingOrder);
    endResetModel();
}

const QString& Model::NodePath(const QHash<QUuid, QString>* leaf, const QHash<QUuid, QString>* branch, const QUuid& node_id) const
{
    if (const auto it = leaf->constFind(node_id); it != leaf->constEnd())
        return it.value();

    if (const auto it = branch->constFind(node_id); it != branch->constEnd())
        return it.value();

    static const QString kEmpty {};
    return kEmpty;
}

QVariant Model::ResolveNode(const Row* row, const QUuid& node_id) const
{
    switch (static_cast<Section>(row->section)) {
    case Section::kFinance:
        return NodePath(info_.f_leaf_path, info_.f_branch_path, node_id);
    case Section::kTask:
        return NodePath(info_.t_leaf_path, info_.t_branch_path, node_id);
    case Section::kInventory:
        return NodePath(info_.i_leaf_path, info_.i_branch_path, node_id);
    case Section::kPartner:
    case Section::kSale:
    case Section::kPurchase:
        return NodePath(info_.p_leaf_path, info_.p_branch_path, node_id);
    }
    return QVariant();
}

QString Model::JsonValueToString(const QJsonValue& value)
{
    switch (value.type()) {
    case QJsonValue::String:
        return value.toString();
    case QJsonValue::Object:
        return QString::fromUtf8(QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact));
    case QJsonValue::Array:
        return QString::fromUtf8(QJsonDocument(value.toArray()).toJson(QJsonDocument::Indented));
    case QJsonValue::Bool:
        return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    case QJsonValue::Double:
        return QString::number(value.toDouble());
    case QJsonValue::Null:
    case QJsonValue::Undefined:
        return {};
    }
}

}
