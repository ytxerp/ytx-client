#include "statementtertiarymodel.h"

#include <QJsonArray>
#include <QJsonObject>

#include "enum/statusenum.h"
#include "global/partner_inventory_registry.h"
#include "global/resourcepool.h"
#include "statementenum.h"
#include "utils/templateutils.h"

namespace statement {
TertiaryModel::TertiaryModel(const QStringList& header, CUuid& partner_id, QObject* parent)
    : QAbstractItemModel { parent }
    , header_ { header }
    , partner_id_ { partner_id }
{
}

TertiaryModel::~TertiaryModel() { ResourcePool<TertiaryRow>::Instance().Recycle(list_); }

QModelIndex TertiaryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex TertiaryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int TertiaryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int TertiaryModel::columnCount(const QModelIndex& /*parent*/) const { return header_.size(); }

QVariant TertiaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    const TertiaryField column { index.column() };
    auto* statement { static_cast<TertiaryRow*>(index.internalPointer()) };

    switch (column) {
    case TertiaryField::kIssuedTime:
        return statement->issued_time;
    case TertiaryField::kInternalSku:
        return statement->internal_sku;
    case TertiaryField::kExternalSku:
        return PartnerInventoryRegistry::Instance().ExternalSku(partner_id_, statement->internal_sku);
    case TertiaryField::kCount:
        return statement->count;
    case TertiaryField::kMeasure:
        return statement->measure;
    case TertiaryField::kUnitPrice:
        return statement->unit_price;
    case TertiaryField::kDescription:
        return statement->description;
    case TertiaryField::kCode:
        return statement->code;
    case TertiaryField::kAmount:
        return statement->amount;
    case TertiaryField::kStatus:
        return statement->status;
    }
}

bool TertiaryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    const TertiaryField column { index.column() };
    auto* statement { static_cast<TertiaryRow*>(index.internalPointer()) };

    switch (column) {
    case TertiaryField::kStatus:
        statement->status = value.toInt();
        break;
    case TertiaryField::kIssuedTime:
    case TertiaryField::kAmount:
    case TertiaryField::kCount:
    case TertiaryField::kDescription:
    case TertiaryField::kMeasure:
    case TertiaryField::kUnitPrice:
    case TertiaryField::kInternalSku:
    case TertiaryField::kExternalSku:
    case TertiaryField::kCode:
        return false;
    }

    return true;
}

QVariant TertiaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

void TertiaryModel::sort(int column, Qt::SortOrder order)
{
    const TertiaryField e_column { column };

    auto Compare = [e_column, order](const TertiaryRow* lhs, const TertiaryRow* rhs) -> bool {
        switch (e_column) {
        case TertiaryField::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &TertiaryRow::issued_time, order);
        case TertiaryField::kInternalSku:
            return utils::CompareMember(lhs, rhs, &TertiaryRow::internal_sku, order);
        case TertiaryField::kExternalSku:
            return false;
        case TertiaryField::kCount:
            return utils::CompareMember(lhs, rhs, &TertiaryRow::count, order);
        case TertiaryField::kMeasure:
            return utils::CompareMember(lhs, rhs, &TertiaryRow::measure, order);
        case TertiaryField::kUnitPrice:
            return utils::CompareMember(lhs, rhs, &TertiaryRow::unit_price, order);
        case TertiaryField::kDescription:
            return utils::CompareMember(lhs, rhs, &TertiaryRow::description, order);
        case TertiaryField::kCode:
            return utils::CompareMember(lhs, rhs, &TertiaryRow::code, order);
        case TertiaryField::kAmount:
            return utils::CompareMember(lhs, rhs, &TertiaryRow::amount, order);
        case TertiaryField::kStatus:
            return utils::CompareMember(lhs, rhs, &TertiaryRow::status, order);
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void TertiaryModel::Rebuild(const QJsonArray& array)
{
    beginResetModel();

    ResourcePool<TertiaryRow>::Instance().Recycle(list_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* statement_secondary { ResourcePool<TertiaryRow>::Instance().Allocate() };
        statement_secondary->ReadJson(obj);

        list_.emplaceBack(statement_secondary);
    }

    sort(std::to_underlying(TertiaryField::kIssuedTime), Qt::AscendingOrder);
    endResetModel();
}

void TertiaryModel::MarkEntries(MarkOperation operation)
{
    if (list_.isEmpty())
        return;

    for (auto* entry : std::as_const(list_)) {
        switch (operation) {
        case MarkOperation::kSelect:
            entry->status = std::to_underlying(EntryStatus::kMarked);
            break;
        case MarkOperation::kClear:
            entry->status = std::to_underlying(EntryStatus::kUnmarked);
            break;
        case MarkOperation::kToggle:
            entry->status ^= std::to_underlying(EntryStatus::kMarked);
            break;
        }
    }

    const int column { std::to_underlying(TertiaryField::kStatus) };

    const QModelIndex top_left { index(0, column) };
    const QModelIndex bottom_right { index(rowCount() - 1, column) };

    emit dataChanged(top_left, bottom_right, QList<int> { Qt::DisplayRole, Qt::EditRole });
}
}
