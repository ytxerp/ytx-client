#include "statementprimarymodel.h"

#include <QJsonArray>
#include <QJsonObject>

#include "global/resourcepool.h"
#include "statementenum.h"
#include "utils/templateutils.h"

namespace statement {

PrimaryModel::PrimaryModel(const QStringList& header, QObject* parent)
    : QAbstractItemModel { parent }
    , header_ { header }
{
}

PrimaryModel::~PrimaryModel() { ResourcePool<PrimaryRow>::Instance().Recycle(list_); }

QModelIndex PrimaryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex PrimaryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int PrimaryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int PrimaryModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return header_.size();
}

QVariant PrimaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const PrimaryField column { index.column() };
    auto* statement { static_cast<PrimaryRow*>(index.internalPointer()) };

    switch (column) {
    case PrimaryField::kPartner:
        return statement->partner_id;
    case PrimaryField::kPBalance:
        return statement->pbalance;
    case PrimaryField::kCAmount:
        return statement->camount;
    case PrimaryField::kCSettlement:
        return statement->csettlement;
    case PrimaryField::kCBalance:
        return statement->cbalance;
    case PrimaryField::kCCount:
        return statement->ccount;
    case PrimaryField::kCMeasure:
        return statement->cmeasure;
    case PrimaryField::kPlaceholder:
        return QVariant();
    }
}

QVariant PrimaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

void PrimaryModel::sort(int column, Qt::SortOrder order)
{
    const PrimaryField e_column { column };

    auto Compare = [e_column, order](const PrimaryRow* lhs, const PrimaryRow* rhs) -> bool {
        switch (e_column) {
        case PrimaryField::kPartner:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::partner_id, order);
        case PrimaryField::kPBalance:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::pbalance, order);
        case PrimaryField::kCAmount:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::camount, order);
        case PrimaryField::kCSettlement:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::csettlement, order);
        case PrimaryField::kCBalance:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::cbalance, order);
        case PrimaryField::kCCount:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::ccount, order);
        case PrimaryField::kCMeasure:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::cmeasure, order);
        case PrimaryField::kPlaceholder:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void PrimaryModel::ResetModel(const QJsonArray& array)
{
    beginResetModel();

    ResourcePool<PrimaryRow>::Instance().Recycle(list_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* statement { ResourcePool<PrimaryRow>::Instance().Allocate() };
        statement->ReadJson(obj);

        list_.emplaceBack(statement);
    }

    sort(std::to_underlying(PrimaryField::kPartner), Qt::AscendingOrder);
    endResetModel();
}

}
