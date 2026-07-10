#include "statementsecondarymodel.h"

#include <QJsonArray>
#include <QJsonObject>

#include "global/resourcepool.h"
#include "statementenum.h"
#include "utils/templateutils.h"

namespace statement {
SecondaryModel::SecondaryModel(const QStringList& header, const QUuid& partner_id, QObject* parent)
    : QAbstractItemModel { parent }
    , header_ { header }
    , partner_id_ { partner_id }
{
}

SecondaryModel::~SecondaryModel() { ResourcePool<SecondaryRow>::Instance().Recycle(list_); }

QModelIndex SecondaryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex SecondaryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SecondaryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int SecondaryModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return header_.size();
}

QVariant SecondaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    const SecondaryField column { index.column() };
    auto* statement { static_cast<SecondaryRow*>(index.internalPointer()) };

    switch (column) {
    case SecondaryField::kDescription:
        return statement->description;
    case SecondaryField::kCode:
        return statement->code;
    case SecondaryField::kEmployee:
        return statement->employee_id;
    case SecondaryField::kIssuedTime:
        return statement->issued_time;
    case SecondaryField::kCount:
        return statement->count;
    case SecondaryField::kMeasure:
        return statement->measure;
    case SecondaryField::kStatus:
        return statement->status;
    case SecondaryField::kAmount:
        return statement->amount;
    case SecondaryField::kSettlement:
        return statement->settlement;
    }
}

bool SecondaryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const SecondaryField column { index.column() };
    auto* statement { static_cast<SecondaryRow*>(index.internalPointer()) };

    switch (column) {
    case SecondaryField::kStatus:
        statement->status = value.toInt();
        break;
    case SecondaryField::kIssuedTime:
    case SecondaryField::kAmount:
    case SecondaryField::kCount:
    case SecondaryField::kDescription:
    case SecondaryField::kMeasure:
    case SecondaryField::kEmployee:
    case SecondaryField::kSettlement:
    case SecondaryField::kCode:
        return false;
    }

    return true;
}

QVariant SecondaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

void SecondaryModel::sort(int column, Qt::SortOrder order)
{
    const SecondaryField e_column { column };

    auto Compare = [e_column, order](const SecondaryRow* lhs, const SecondaryRow* rhs) -> bool {
        switch (e_column) {
        case SecondaryField::kDescription:
            return utils::CompareMember(lhs, rhs, &SecondaryRow::description, order);
        case SecondaryField::kCode:
            return utils::CompareMember(lhs, rhs, &SecondaryRow::code, order);
        case SecondaryField::kEmployee:
            return utils::CompareMember(lhs, rhs, &SecondaryRow::employee_id, order);
        case SecondaryField::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &SecondaryRow::issued_time, order);
        case SecondaryField::kCount:
            return utils::CompareMember(lhs, rhs, &SecondaryRow::count, order);
        case SecondaryField::kMeasure:
            return utils::CompareMember(lhs, rhs, &SecondaryRow::measure, order);
        case SecondaryField::kStatus:
            return utils::CompareMember(lhs, rhs, &SecondaryRow::status, order);
        case SecondaryField::kSettlement:
            return utils::CompareMember(lhs, rhs, &SecondaryRow::settlement, order);
        case SecondaryField::kAmount:
            return utils::CompareMember(lhs, rhs, &SecondaryRow::amount, order);
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void SecondaryModel::ResetModel(const QJsonArray& array)
{
    beginResetModel();

    ResourcePool<SecondaryRow>::Instance().Recycle(list_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* statement_primary { ResourcePool<SecondaryRow>::Instance().Allocate() };
        statement_primary->ReadJson(obj);

        list_.emplaceBack(statement_primary);
    }

    sort(std::to_underlying(SecondaryField::kIssuedTime), Qt::AscendingOrder);
    endResetModel();
}
}
