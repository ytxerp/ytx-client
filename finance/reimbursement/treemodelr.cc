#include "treemodelr.h"

#include <QJsonArray>

#include "component/constantwebsocket.h"
#include "global/resourcepool.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

namespace reimbursement {

TreeModelR::TreeModelR(const QStringList& header, QObject* parent)
    : QAbstractItemModel { parent }
    , header_ { header }
{
}

TreeModelR::~TreeModelR() { ResourcePool<PrimaryRow>::Instance().Recycle(list_); }

void TreeModelR::RInsertePrimaryRow(const reimbursement::PrimaryRow& row)
{
    auto* reimbursement { ResourcePool<PrimaryRow>::Instance().Allocate() };
    *reimbursement = row;

    const int count { rowCount() };

    beginInsertRows(QModelIndex(), count, count);
    list_.emplaceBack(reimbursement);
    endInsertRows();
}

void TreeModelR::RUpdatePrimaryRow(const reimbursement::PrimaryRow& row)
{
    const auto row_index { FindRow(row.id) };

    if (!row_index)
        return;

    auto* reimbursement { list_[*row_index] };

    Q_ASSERT(reimbursement != nullptr);

    *reimbursement = row;

    emit dataChanged(index(*row_index, std::to_underlying(PrimaryField::kIssuedTime)), index(*row_index, std::to_underlying(PrimaryField::kInitialTotal)));
}

QModelIndex TreeModelR::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QVariant TreeModelR::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const PrimaryField column { index.column() };
    auto* reimbursement { static_cast<PrimaryRow*>(index.internalPointer()) };

    switch (column) {
    case PrimaryField::kIssuedTime:
        return reimbursement->issued_time;
    case PrimaryField::kDescription:
        return reimbursement->description;
    case PrimaryField::kStatus:
        return std::to_underlying(reimbursement->status);
    case PrimaryField::kPlaceholder:
        return QVariant();
    case PrimaryField::kUsername:
        return reimbursement->username;
    case PrimaryField::kInitialTotal:
        return reimbursement->initial_total;
        break;
    }
}

QVariant TreeModelR::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    if (section < 0 || section >= header_.size())
        return {};

    return header_.at(section);
}

void TreeModelR::sort(int column, Qt::SortOrder order)
{
    const PrimaryField e_column { column };
    if (e_column == PrimaryField::kPlaceholder)
        return;

    auto Compare = [e_column, order](const PrimaryRow* lhs, const PrimaryRow* rhs) -> bool {
        switch (e_column) {
        case PrimaryField::kUsername:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::username, order);
        case PrimaryField::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::issued_time, order);
        case PrimaryField::kDescription:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::description, order);
        case PrimaryField::kStatus:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::status, order);
        case PrimaryField::kInitialTotal:
            return utils::CompareMember(lhs, rhs, &PrimaryRow::initial_total, order);
        case PrimaryField::kPlaceholder:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(list_, Compare);
    emit layoutChanged();
}

bool TreeModelR::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row);
    auto* reimbursement { list_.takeAt(row) };
    endRemoveRows();

    QJsonObject message { JsonGen::ReimbursementDelete(reimbursement->id, reimbursement->version) };
    WebSocket::Instance()->SendMessage(WsKey::kReimbursementDelete, message);

    ResourcePool<PrimaryRow>::Instance().Recycle(reimbursement);
    return true;
}

void TreeModelR::Rebuild(const QJsonArray& array)
{
    if (array.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Received empty array";
    }

    QList<PrimaryRow*> new_list {};
    new_list.reserve(array.size());

    for (const auto& value : array) {
        Q_ASSERT(value.isObject());

        auto* row { ResourcePool<PrimaryRow>::Instance().Allocate() };
        row->ReadJson(value.toObject());
        new_list.emplaceBack(row);
    }

    std::ranges::sort(new_list, [](const auto* lhs, const auto* rhs) { return utils::CompareMember(lhs, rhs, &PrimaryRow::issued_time, Qt::AscendingOrder); });

    beginResetModel();

    ResourcePool<PrimaryRow>::Instance().Recycle(list_);
    list_ = std::move(new_list);

    endResetModel();
}
}
