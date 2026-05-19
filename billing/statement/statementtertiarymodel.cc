#include "statementtertiarymodel.h"

#include <QJsonArray>
#include <QJsonObject>

#include "global/resourcepool.h"
#include "statementenum.h"
#include "utils/templateutils.h"

StatementTertiaryModel::StatementTertiaryModel(EntryHubP* entry_hub_p, const QStringList& header, CUuid& partner_id, QObject* parent)
    : QAbstractItemModel { parent }
    , header_ { header }
    , entry_hub_p_ { entry_hub_p }
    , partner_id_ { partner_id }
{
}

StatementTertiaryModel::~StatementTertiaryModel() { ResourcePool<StatementTertiary>::Instance().Recycle(list_); }

QModelIndex StatementTertiaryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex StatementTertiaryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int StatementTertiaryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int StatementTertiaryModel::columnCount(const QModelIndex& /*parent*/) const { return header_.size(); }

QVariant StatementTertiaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* entry { list_.at(index.row()) };
    const StatementTertiaryEnum column { index.column() };

    switch (column) {
    case StatementTertiaryEnum::kIssuedTime:
        return entry->issued_time;
    case StatementTertiaryEnum::kInternalSku:
        return entry->internal_sku;
    case StatementTertiaryEnum::kExternalSku:
        return entry_hub_p_->ExternalSku(partner_id_, entry->internal_sku);
    case StatementTertiaryEnum::kCount:
        return entry->count;
    case StatementTertiaryEnum::kMeasure:
        return entry->measure;
    case StatementTertiaryEnum::kUnitPrice:
        return entry->unit_price;
    case StatementTertiaryEnum::kDescription:
        return entry->description;
    case StatementTertiaryEnum::kCode:
        return entry->code;
    case StatementTertiaryEnum::kAmount:
        return entry->amount;
    case StatementTertiaryEnum::kStatus:
        return entry->status;
    }
}

bool StatementTertiaryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    const StatementTertiaryEnum column { index.column() };
    const int kRow { index.row() };

    auto* entry { list_.at(kRow) };

    switch (column) {
    case StatementTertiaryEnum::kStatus:
        entry->status = value.toInt();
        break;
    case StatementTertiaryEnum::kIssuedTime:
    case StatementTertiaryEnum::kAmount:
    case StatementTertiaryEnum::kCount:
    case StatementTertiaryEnum::kDescription:
    case StatementTertiaryEnum::kMeasure:
    case StatementTertiaryEnum::kUnitPrice:
    case StatementTertiaryEnum::kInternalSku:
    case StatementTertiaryEnum::kExternalSku:
    case StatementTertiaryEnum::kCode:
        return false;
    }

    return true;
}

QVariant StatementTertiaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

void StatementTertiaryModel::sort(int column, Qt::SortOrder order)
{
    const StatementTertiaryEnum e_column { column };

    auto Compare = [e_column, order](const StatementTertiary* lhs, const StatementTertiary* rhs) -> bool {
        switch (e_column) {
        case StatementTertiaryEnum::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &StatementTertiary::issued_time, order);
        case StatementTertiaryEnum::kInternalSku:
            return utils::CompareMember(lhs, rhs, &StatementTertiary::internal_sku, order);
        case StatementTertiaryEnum::kExternalSku:
            return false;
        case StatementTertiaryEnum::kCount:
            return utils::CompareMember(lhs, rhs, &StatementTertiary::count, order);
        case StatementTertiaryEnum::kMeasure:
            return utils::CompareMember(lhs, rhs, &StatementTertiary::measure, order);
        case StatementTertiaryEnum::kUnitPrice:
            return utils::CompareMember(lhs, rhs, &StatementTertiary::unit_price, order);
        case StatementTertiaryEnum::kDescription:
            return utils::CompareMember(lhs, rhs, &StatementTertiary::description, order);
        case StatementTertiaryEnum::kCode:
            return utils::CompareMember(lhs, rhs, &StatementTertiary::code, order);
        case StatementTertiaryEnum::kAmount:
            return utils::CompareMember(lhs, rhs, &StatementTertiary::amount, order);
        case StatementTertiaryEnum::kStatus:
            return utils::CompareMember(lhs, rhs, &StatementTertiary::status, order);
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void StatementTertiaryModel::ResetModel(const QJsonArray& array)
{
    beginResetModel();

    ResourcePool<StatementTertiary>::Instance().Recycle(list_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* statement_secondary { ResourcePool<StatementTertiary>::Instance().Allocate() };
        statement_secondary->ReadJson(obj);

        list_.emplaceBack(statement_secondary);
    }

    sort(std::to_underlying(StatementTertiaryEnum::kIssuedTime), Qt::AscendingOrder);
    endResetModel();
}

void StatementTertiaryModel::MarkBatch(Mark mark)
{
    if (list_.isEmpty())
        return;

    for (auto* entry : std::as_const(list_)) {
        switch (mark) {
        case Mark::kSelect:
            entry->status = std::to_underlying(Status::kMarked);
            break;
        case Mark::kClear:
            entry->status = std::to_underlying(Status::kUnmarked);
            break;
        case Mark::kToggle:
            entry->status ^= std::to_underlying(Status::kMarked);
            break;
        }
    }

    const int column { std::to_underlying(StatementTertiaryEnum::kStatus) };

    const QModelIndex top_left { index(0, column) };
    const QModelIndex bottom_right { index(rowCount() - 1, column) };

    emit dataChanged(top_left, bottom_right, QList<int> { Qt::DisplayRole, Qt::EditRole });
}
