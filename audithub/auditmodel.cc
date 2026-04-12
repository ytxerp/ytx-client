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

    const auto* entry { static_cast<AuditEntry*>(index.internalPointer()) };

    switch (static_cast<AuditField>(index.column())) {
    case AuditField::kTargetId:
        return entry->target_id.toString(QUuid::WithoutBraces).left(12);
    case AuditField::kUserId:
        return info_.user_hash.value(entry->user_id);
    case AuditField::kCreatedTime:
        return entry->created_time;
    case AuditField::kTargetCode:
        return entry->target_code;
    case AuditField::kBefore:
        return JsonValueToString(entry->before);
    case AuditField::kAfter:
        return JsonValueToString(entry->after);
    case AuditField::kId:
        return entry->id;
    case AuditField::kSection:
        return info_.section_hash.value(entry->section);
    case AuditField::kWsKey:
        return info_.ws_key_hash.value(entry->ws_key);
    case AuditField::kLevel:
        return info_.level_hash.value(entry->level);
    case AuditField::kTargetType:
        return info_.target_type_hash.value(entry->target_type);
    case AuditField::kLhsNode:
        return ResolveNode(entry, entry->lhs_node);
    case AuditField::kRhsNode:
        return ResolveNode(entry, entry->rhs_node);
    }
}

void AuditModel::sort(int column, Qt::SortOrder order)
{
    // Convert integer column to the structured enum using brace initialization
    const auto e_column { static_cast<AuditField>(column) };

    // Define a lambda for comparison based on the selected column and sort order
    auto Compare = [order, e_column](const AuditEntry* lhs, const AuditEntry* rhs) -> bool {
        switch (e_column) {
        case AuditField::kTargetId:
            return Utils::CompareMember(lhs, rhs, &AuditEntry::target_id, order);
        case AuditField::kUserId:
            return Utils::CompareMember(lhs, rhs, &AuditEntry::user_id, order);
        case AuditField::kLhsNode:
            return Utils::CompareMember(lhs, rhs, &AuditEntry::lhs_node, order);
        case AuditField::kRhsNode:
            return Utils::CompareMember(lhs, rhs, &AuditEntry::rhs_node, order);
        case AuditField::kTargetCode:
            return Utils::CompareMember(lhs, rhs, &AuditEntry::target_code, order);
        case AuditField::kSection:
            return Utils::CompareMember(lhs, rhs, &AuditEntry::section, order);
        case AuditField::kWsKey:
            return Utils::CompareMember(lhs, rhs, &AuditEntry::ws_key, order);
        case AuditField::kTargetType:
            return Utils::CompareMember(lhs, rhs, &AuditEntry::target_type, order);
        case AuditField::kLevel:
            return Utils::CompareMember(lhs, rhs, &AuditEntry::level, order);
        case AuditField::kCreatedTime:
            return Utils::CompareMember(lhs, rhs, &AuditEntry::created_time, order);
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
    QList<AuditEntry*> new_list {};
    new_list.reserve(array.size());

    for (const auto& value : array) {
        if (!value.isObject()) {
            qWarning() << "[AuditModel]" << "Invalid data, expected object:" << value;
            continue;
        }

        auto* entry { ResourcePool<AuditEntry>::Instance().Allocate() };
        entry->ReadJson(value.toObject());
        new_list.emplaceBack(entry);
    }

    // Keep reset block as short as possible
    beginResetModel();
    ResourcePool<AuditEntry>::Instance().Recycle(list_);
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

QVariant AuditModel::ResolveNode(const AuditEntry* entry, const QUuid& node_id) const
{
    switch (static_cast<Section>(entry->section)) {
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
        return QString::fromUtf8(QJsonDocument(value.toArray()).toJson(QJsonDocument::Compact));
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
