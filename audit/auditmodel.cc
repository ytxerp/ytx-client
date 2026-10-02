#include "auditmodel.h"

#include "auditenum.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

namespace audit {

Model::Model(const Info& info, const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
    , info_ { info }
    , header_ { header }
{
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

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
        return row->target_id;
    case RowField::kUsername:
        return row->username;
    case RowField::kCreatedTime:
        return row->created_time;
    case RowField::kTargetCode:
        return row->target_code;
    case RowField::kBefore:
        return JsonValueToString(row->before);
    case RowField::kAfter:
        return JsonValueToString(row->after);
    case RowField::kTargetOperation:
        return row->target_operation;
    case RowField::kTargetType:
        return row->target_type;
    case RowField::kLhsNodeName:
        return row->lhs_node;
    case RowField::kRhsNodeName:
        return row->rhs_node;
    case RowField::kTargetField:
        return row->target_field;
    case RowField::kPlaceholder:
        return QVariant();
    }
}

void Model::Rebuild(const QJsonArray& array)
{
    if (array.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Received empty array";
    }

    // Parse outside the reset block
    QList<Row*> new_list {};
    new_list.reserve(array.size());

    for (const auto& value : array) {
        Q_ASSERT(value.isObject());

        auto* entry { ResourcePool<Row>::Instance().Allocate() };
        entry->ReadJson(value.toObject());
        new_list.emplaceBack(entry);
    }

    std::ranges::sort(new_list, [](const auto* lhs, const auto* rhs) { return utils::CompareMember(lhs, rhs, &Row::created_time, Qt::AscendingOrder); });

    // Keep reset block as short as possible
    beginResetModel();

    ResourcePool<Row>::Instance().Recycle(list_);
    list_ = std::move(new_list);

    endResetModel();
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
