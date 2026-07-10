#include "auditmodel.h"

#include "auditenum.h"
#include "enum/section.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

namespace audit_hub {

AuditModel::AuditModel(const AuditInfo& info, QObject* parent)
    : QAbstractItemModel(parent)
    , info_ { info }
{
}

QVariant AuditModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.header.at(section);

    return QVariant();
}

QModelIndex AuditModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QVariant AuditModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const AuditField column { index.column() };
    const auto* row { static_cast<AuditRow*>(index.internalPointer()) };

    switch (column) {
    case AuditField::kTargetId:
        return row->target_id.toString(QUuid::WithoutBraces).left(12);
    case AuditField::kUserId:
        return info_.user_hash.value(row->user_id);
    case AuditField::kCreatedTime:
        return row->created_time;
    case AuditField::kCode:
        return row->code;
    case AuditField::kBefore:
        return JsonValueToString(row->before);
    case AuditField::kAfter:
        return JsonValueToString(row->after);
    case AuditField::kId:
        return row->id;
    case AuditField::kSection:
        return info_.section_hash.value(row->section);
    case AuditField::kOperation:
        return info_.ws_key_hash.value(row->operation);
    case AuditField::kLevel:
        return info_.level_hash.value(row->level);
    case AuditField::kTarget:
        return info_.target_type_hash.value(row->target);
    case AuditField::kLhsNode:
        return ResolveNode(row, row->lhs_node);
    case AuditField::kRhsNode:
        return ResolveNode(row, row->rhs_node);
    }
}

void AuditModel::sort(int column, Qt::SortOrder order)
{
    // Convert integer column to the structured enum using brace initialization
    const AuditField e_column { column };

    // Define a lambda for comparison based on the selected column and sort order
    auto Compare = [order, e_column](const AuditRow* lhs, const AuditRow* rhs) -> bool {
        switch (e_column) {
        case AuditField::kTargetId:
            return utils::CompareMember(lhs, rhs, &AuditRow::target_id, order);
        case AuditField::kUserId:
            return utils::CompareMember(lhs, rhs, &AuditRow::user_id, order);
        case AuditField::kLhsNode:
            return utils::CompareMember(lhs, rhs, &AuditRow::lhs_node, order);
        case AuditField::kRhsNode:
            return utils::CompareMember(lhs, rhs, &AuditRow::rhs_node, order);
        case AuditField::kCode:
            return utils::CompareMember(lhs, rhs, &AuditRow::code, order);
        case AuditField::kSection:
            return utils::CompareMember(lhs, rhs, &AuditRow::section, order);
        case AuditField::kOperation:
            return utils::CompareMember(lhs, rhs, &AuditRow::operation, order);
        case AuditField::kTarget:
            return utils::CompareMember(lhs, rhs, &AuditRow::target, order);
        case AuditField::kLevel:
            return utils::CompareMember(lhs, rhs, &AuditRow::level, order);
        case AuditField::kCreatedTime:
            return utils::CompareMember(lhs, rhs, &AuditRow::created_time, order);
        case AuditField::kId:
        case AuditField::kBefore:
        case AuditField::kAfter:
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

void AuditModel::ResetModel(const QJsonArray& array)
{
    if (array.isEmpty()) {
        qWarning() << "[AuditModel]" << "Received empty array";
    }

    // Parse outside the reset block
    QList<AuditRow*> new_list {};
    new_list.reserve(array.size());

    for (const auto& value : array) {
        if (!value.isObject()) {
            qWarning() << "[AuditModel]" << "Invalid data, expected object:" << value;
            continue;
        }

        auto* entry { ResourcePool<AuditRow>::Instance().Allocate() };
        entry->ReadJson(value.toObject());
        new_list.emplaceBack(entry);
    }

    // Keep reset block as short as possible
    beginResetModel();
    ResourcePool<AuditRow>::Instance().Recycle(list_);
    list_ = std::move(new_list);
    sort(std::to_underlying(AuditField::kCreatedTime), Qt::AscendingOrder);
    endResetModel();
}

const QString& AuditModel::NodePath(const QHash<QUuid, QString>* leaf, const QHash<QUuid, QString>* branch, const QUuid& node_id) const
{
    if (const auto it = leaf->constFind(node_id); it != leaf->constEnd())
        return it.value();

    if (const auto it = branch->constFind(node_id); it != branch->constEnd())
        return it.value();

    static const QString kEmpty {};
    return kEmpty;
}

QVariant AuditModel::ResolveNode(const AuditRow* row, const QUuid& node_id) const
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

QString AuditModel::JsonValueToString(const QJsonValue& value)
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
