#include "reimbursementsecondarymodel.h"

#include <QJsonArray>

#include "component/constant.h"
#include "global/resourcepool.h"
#include "utils/templateutils.h"

namespace reimbursement {

SecondaryModel::SecondaryModel(const QStringList& header, Status status, QUuid primary_id, int finance_permission, QObject* parent)
    : QAbstractItemModel { parent }
    , header_ { header }
    , primary_id_ { primary_id }
    , finance_permission_ { finance_permission }
    , status_ { status }
{
}

SecondaryModel::~SecondaryModel() { ResourcePool<SecondaryRow>::Instance().Recycle(list_); }

QModelIndex SecondaryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QVariant SecondaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    const SecondaryField column { index.column() };
    auto* reimbursement { static_cast<SecondaryRow*>(index.internalPointer()) };

    switch (column) {
    case SecondaryField::kDescription:
        return reimbursement->description;
    case SecondaryField::kDebitNode:
        return reimbursement->debit_node;
    case SecondaryField::kCreditNode:
        return reimbursement->credit_node;
    case SecondaryField::kDocument:
        return reimbursement->document;
    case SecondaryField::kPlaceholder:
        return QVariant();
    case SecondaryField::kInitial:
        return reimbursement->initial;
        break;
    }
}

bool SecondaryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (!(flags(index) & Qt::ItemIsEditable))
        return false;

    if (data(index, role) == value)
        return false;

    const SecondaryField column { index.column() };
    auto* row { static_cast<SecondaryRow*>(index.internalPointer()) };

    const double old_initial { row->initial };

    switch (column) {
    case SecondaryField::kDescription:
        UpdateDescription(row, value.toString());
        break;
    case SecondaryField::kDebitNode:
        UpdateDebit(row, value.toUuid());
        break;
    case SecondaryField::kCreditNode:
        UpdateCredit(row, value.toUuid());
        break;
    case SecondaryField::kDocument:
        UpdateDocument(row, value.toStringList());
        break;
    case SecondaryField::kPlaceholder:
        return true;
    case SecondaryField::kInitial:
        UpdateInitial(row, value.toDouble());
        break;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });

    const double initial_delta { row->initial - old_initial };
    if (!qFuzzyIsNull(initial_delta))
        emit SSyncInitial(initial_delta);

    return true;
}

bool SecondaryModel::UpdateDescription(SecondaryRow* row, const QString& value)
{
    if (row->description == value)
        return false;

    row->description = value;

    if (row->sync_state == SyncState::kSynced)
        row->sync_state = SyncState::kUpdating;

    return true;
}

bool SecondaryModel::UpdateDocument(SecondaryRow* row, const QStringList& value)
{
    if (row->document == value)
        return false;

    row->document = value;

    if (row->sync_state == SyncState::kSynced)
        row->sync_state = SyncState::kUpdating;

    return true;
}

bool SecondaryModel::UpdateInitial(SecondaryRow* row, double value)
{
    if (qFuzzyCompare(row->initial, value))
        return false;

    row->initial = value;

    if (row->sync_state == SyncState::kSynced)
        row->sync_state = SyncState::kUpdating;

    return true;
}

bool SecondaryModel::UpdateDebit(SecondaryRow* row, QUuid value)
{
    if (row->debit_node == value)
        return false;

    row->debit_node = value;

    if (row->sync_state == SyncState::kSynced)
        row->sync_state = SyncState::kUpdating;

    return true;
}

bool SecondaryModel::UpdateCredit(SecondaryRow* row, QUuid value)
{
    if (row->credit_node == value)
        return false;

    row->credit_node = value;

    if (row->sync_state == SyncState::kSynced)
        row->sync_state = SyncState::kUpdating;

    return true;
}

QVariant SecondaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    if (section < 0 || section >= header_.size())
        return {};

    return header_.at(section);
}

void SecondaryModel::sort(int column, Qt::SortOrder order)
{
    const SecondaryField e_column { column };
    if (e_column == SecondaryField::kPlaceholder)
        return;

    auto Compare = [e_column, order](const SecondaryRow* lhs, const SecondaryRow* rhs) -> bool {
        switch (e_column) {
        case SecondaryField::kDescription:
            return utils::CompareMember(lhs, rhs, &SecondaryRow::description, order);
        case SecondaryField::kDebitNode:
            return utils::CompareMember(lhs, rhs, &SecondaryRow::debit_node, order);
        case SecondaryField::kCreditNode:
            return utils::CompareMember(lhs, rhs, &SecondaryRow::credit_node, order);
        case SecondaryField::kDocument:
            return utils::CompareMember(lhs, rhs, &SecondaryRow::document, order);
        case SecondaryField::kPlaceholder:
            return false;
        case SecondaryField::kInitial:
            return utils::CompareMember(lhs, rhs, &SecondaryRow::initial, order);
        }
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(list_, Compare);
    emit layoutChanged();
}

void SecondaryModel::Rebuild(const QJsonArray& array)
{
    if (array.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Received empty array";
    }

    QList<SecondaryRow*> new_list {};
    new_list.reserve(array.size());

    for (const auto& value : array) {
        Q_ASSERT(value.isObject());

        auto* row { ResourcePool<SecondaryRow>::Instance().Allocate() };
        row->ReadJson(value.toObject());

        new_list.emplaceBack(row);
    }

    std::ranges::sort(
        new_list, [](const auto* lhs, const auto* rhs) { return utils::CompareMember(lhs, rhs, &SecondaryRow::description, Qt::AscendingOrder); });

    beginResetModel();

    ResourcePool<SecondaryRow>::Instance().Recycle(list_);
    list_ = std::move(new_list);

    endResetModel();
}

void SecondaryModel::UpdateStatus(Status status)
{
    if (status_ == status)
        return;

    status_ = status;
}

void SecondaryModel::Finalize(QJsonObject& message)
{
    // deleted
    {
        QJsonArray deleted_entry_array {};
        for (const auto& id : std::as_const(pending_delete_)) {
            deleted_entry_array.append(id.toString(QUuid::WithoutBraces));
        }
        message.insert(kDeletedEntryArray, deleted_entry_array);
    }

    // insert and update
    {
        QJsonArray inserted_entry_array {};
        QJsonArray updated_entry_array {};

        for (auto* entry : std::as_const(list_)) {
            switch (entry->sync_state) {
            case SyncState::kCreating:
                inserted_entry_array.append(entry->WriteJson());
                break;
            case SyncState::kUpdating:
                updated_entry_array.append(entry->WriteJson());
                break;
            case SyncState::kSynced:
            case SyncState::kDeleting:
            case SyncState::kError:
                break;
            }
        }

        message.insert(kInsertedEntryArray, inserted_entry_array);
        message.insert(kUpdatedEntryArray, updated_entry_array);
    }
}

void SecondaryModel::Purify()
{
    for (auto i = list_.size() - 1; i >= 0; --i) {
        auto* row { list_[i] };

        if (!row->description.isEmpty())
            continue;

        beginRemoveRows(QModelIndex(), i, i);
        ResourcePool<SecondaryRow>::Instance().Recycle(list_.takeAt(i));
        endRemoveRows();
    }
}

void SecondaryModel::SyncSucceeded()
{
    for (auto* entry : std::as_const(list_)) {
        switch (entry->sync_state) {
        case SyncState::kCreating:
        case SyncState::kUpdating:
            entry->sync_state = SyncState::kSynced;
            break;

        case SyncState::kSynced:
        case SyncState::kDeleting:
        case SyncState::kError:
            break;
        }
    }

    pending_delete_.clear();
}

bool SecondaryModel::HasPendingUpdate() const
{
    if (!pending_delete_.isEmpty())
        return true;

    for (auto* row : std::as_const(list_)) {
        if (row->sync_state == SyncState::kCreating || row->sync_state == SyncState::kUpdating)
            return true;
    }

    return false;
}

Qt::ItemFlags SecondaryModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };

    if (status_ != Status::kDraft)
        return flags;

    const SecondaryField column { index.column() };

    switch (column) {
    case SecondaryField::kDebitNode:
    case SecondaryField::kCreditNode:
        if (finance_permission_ & (finance_permission::kApprove | finance_permission::kUnapprove))
            flags |= Qt::ItemIsEditable;
        break;
    case SecondaryField::kDescription:
    case SecondaryField::kDocument:
    case SecondaryField::kInitial:
        flags |= Qt::ItemIsEditable;
        break;
    case SecondaryField::kPlaceholder:
        break;
    }

    return flags;
}

bool SecondaryModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (parent.isValid() || count != 1)
        return false;

    if (row < 0 || row > list_.size())
        return false;

    if (status_ != Status::kDraft)
        return false;

    auto* entry { ResourcePool<SecondaryRow>::Instance().Allocate() };

    entry->id = QUuid::createUuidV7();
    entry->lhs_node = primary_id_;
    entry->sync_state = SyncState::kCreating;

    beginInsertRows(parent, row, row);
    list_.insert(row, entry);
    endInsertRows();

    return true;
}

bool SecondaryModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent.isValid() || count != 1)
        return false;

    if (row < 0 || row >= list_.size())
        return false;

    if (status_ != Status::kDraft)
        return false;

    auto* entry = list_.at(row);
    const QUuid entry_id { entry->id };
    const double initial_delta { -entry->initial };

    beginRemoveRows(parent, row, row);
    list_.removeAt(row);
    endRemoveRows();

    if (!qFuzzyIsNull(initial_delta))
        emit SSyncInitial(initial_delta);

    if (entry->sync_state != SyncState::kCreating)
        pending_delete_.insert(entry_id);

    ResourcePool<SecondaryRow>::Instance().Recycle(entry);
    return true;
}

}
