#include "tablemodelr.h"

#include <QJsonArray>

#include "component/constant.h"
#include "component/constantstring.h"
#include "global/entrypool.h"
#include "utils/templateutils.h"

TableModelR::TableModelR(CTableModelArg& arg, const NodeR* node, int finance_permission, QObject* parent)
    : TableModel { arg, parent }
    , d_node_ { node }
    , finance_permission_ { finance_permission }
{
}

TableModelR::~TableModelR() { EntryPool::Instance().Recycle(entry_list_, section_); }

QModelIndex TableModelR::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, entry_list_.at(row));
}

QVariant TableModelR::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const EntryEnumR column { index.column() };
    auto* entry { static_cast<EntryR*>(index.internalPointer()) };

    if (role == Qt::ToolTipRole && column == EntryEnumR::kIssuedTime)
        return entry->issued_time.toString(datetime_format::kDateTime);

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    switch (column) {
    case EntryEnumR::kDescription:
        return entry->description;
    case EntryEnumR::kDebitNode:
        return entry->debit_node;
    case EntryEnumR::kCreditNode:
        return entry->credit_node;
    case EntryEnumR::kDocument:
        return entry->document;
    case EntryEnumR::kInitial:
        return entry->initial;
    case EntryEnumR::kLhsNode:
        return entry->lhs_node;
    case EntryEnumR::kTag:
        return entry->tag;
    case EntryEnumR::kIssuedTime:
        return entry->issued_time;
    }
}

bool TableModelR::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (!(flags(index) & Qt::ItemIsEditable))
        return false;

    if (data(index, role) == value)
        return false;

    const EntryEnumR column { index.column() };
    auto* row { static_cast<EntryR*>(index.internalPointer()) };

    const double old_initial { row->initial };

    switch (column) {
    case EntryEnumR::kDescription:
        UpdateDescription(row, value.toString());
        break;
    case EntryEnumR::kDebitNode:
        UpdateDebit(row, value.toUuid());
        break;
    case EntryEnumR::kCreditNode:
        UpdateCredit(row, value.toUuid());
        break;
    case EntryEnumR::kDocument:
        UpdateDocument(row, value.toStringList());
        break;
    case EntryEnumR::kInitial:
        UpdateInitial(row, value.toDouble());
        break;
    case EntryEnumR::kTag:
        UpdateTag(row, value.toStringList());
        break;
    case EntryEnumR::kLhsNode:
    case EntryEnumR::kIssuedTime:
        break;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });

    const double initial_delta { row->initial - old_initial };
    if (!qFuzzyIsNull(initial_delta))
        emit SSyncInitial(row->id, initial_delta);

    return true;
}

bool TableModelR::UpdateDescription(EntryR* entry, const QString& value)
{
    if (entry->description == value)
        return false;

    entry->description = value;

    if (entry->sync_state == SyncState::kSynced)
        entry->sync_state = SyncState::kUpdating;

    return true;
}

bool TableModelR::UpdateDocument(EntryR* entry, const QStringList& value)
{
    if (entry->document == value)
        return false;

    entry->document = value;

    if (entry->sync_state == SyncState::kSynced)
        entry->sync_state = SyncState::kUpdating;

    return true;
}

bool TableModelR::UpdateInitial(EntryR* entry, double value)
{
    if (qFuzzyCompare(entry->initial, value))
        return false;

    entry->initial = value;

    if (entry->sync_state == SyncState::kSynced)
        entry->sync_state = SyncState::kUpdating;

    return true;
}

bool TableModelR::UpdateDebit(EntryR* entry, QUuid value)
{
    if (entry->debit_node == value)
        return false;

    entry->debit_node = value;

    if (entry->sync_state == SyncState::kSynced)
        entry->sync_state = SyncState::kUpdating;

    return true;
}

bool TableModelR::UpdateCredit(EntryR* entry, QUuid value)
{
    if (entry->credit_node == value)
        return false;

    entry->credit_node = value;

    if (entry->sync_state == SyncState::kSynced)
        entry->sync_state = SyncState::kUpdating;

    return true;
}

bool TableModelR::UpdateTag(EntryR* entry, const QStringList& value)
{
    if (entry->tag == value)
        return false;

    entry->tag = value;

    if (entry->sync_state == SyncState::kSynced)
        entry->sync_state = SyncState::kUpdating;

    return true;
}

void TableModelR::sort(int column, Qt::SortOrder order)
{
    const EntryEnumR e_column { column };

    auto Compare = [e_column, order](const Entry* lhs, const Entry* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryR>(lhs) };
        auto* d_rhs { DerivedPtr<EntryR>(rhs) };

        switch (e_column) {
        case EntryEnumR::kDescription:
            return utils::CompareMember(lhs, rhs, &Entry::description, order);
        case EntryEnumR::kDebitNode:
            return utils::CompareMember(d_lhs, d_rhs, &EntryR::debit_node, order);
        case EntryEnumR::kCreditNode:
            return utils::CompareMember(d_lhs, d_rhs, &EntryR::credit_node, order);
        case EntryEnumR::kDocument:
            return utils::CompareMember(lhs, rhs, &Entry::document, order);
        case EntryEnumR::kInitial:
            return utils::CompareMember(d_lhs, d_rhs, &EntryR::initial, order);
        case EntryEnumR::kTag:
            return utils::CompareMember(lhs, rhs, &EntryR::tag, order);
        case EntryEnumR::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &EntryR::issued_time, order);
        case EntryEnumR::kLhsNode:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(entry_list_, Compare);
    emit layoutChanged();
}

void TableModelR::Finalize(QJsonObject& message)
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

        for (auto* entry : std::as_const(entry_list_)) {
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

void TableModelR::Purify()
{
    for (auto i = entry_list_.size() - 1; i >= 0; --i) {
        auto* row { entry_list_[i] };

        if (!row->description.isEmpty())
            continue;

        beginRemoveRows(QModelIndex(), i, i);
        EntryPool::Instance().Recycle(entry_list_.takeAt(i), section_);
        endRemoveRows();
    }
}

void TableModelR::SyncSucceeded()
{
    for (auto* entry : std::as_const(entry_list_)) {
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

bool TableModelR::HasPendingUpdate() const
{
    if (!pending_delete_.isEmpty())
        return true;

    for (auto* row : std::as_const(entry_list_)) {
        if (row->sync_state == SyncState::kCreating || row->sync_state == SyncState::kUpdating)
            return true;
    }

    return false;
}

Qt::ItemFlags TableModelR::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };

    if (d_node_->reimbursement_status != ReimbursementStatus::kDraft)
        return flags;

    const EntryEnumR column { index.column() };

    switch (column) {
    case EntryEnumR::kDebitNode:
    case EntryEnumR::kCreditNode:
        if (finance_permission_ & (finance_permission::kApprove | finance_permission::kUnapprove))
            flags |= Qt::ItemIsEditable;
        break;
    case EntryEnumR::kDescription:
    case EntryEnumR::kInitial:
        flags |= Qt::ItemIsEditable;
        break;
    case EntryEnumR::kDocument:
    case EntryEnumR::kTag:
    case EntryEnumR::kLhsNode:
    case EntryEnumR::kIssuedTime:
        flags &= ~Qt::ItemIsEditable;
        break;
    }

    return flags;
}

bool TableModelR::insertRows(int row, int count, const QModelIndex& parent)
{
    if (parent.isValid() || count != 1)
        return false;

    if (row < 0 || row > entry_list_.size())
        return false;

    if (d_node_->reimbursement_status != ReimbursementStatus::kDraft)
        return false;

    auto* entry { EntryPool::Instance().Allocate(section_) };

    entry->id = QUuid::createUuidV7();
    entry->lhs_node = node_id_;
    entry->sync_state = SyncState::kCreating;
    entry->issued_time = QDateTime::currentDateTime();

    beginInsertRows(parent, row, row);
    entry_list_.insert(row, entry);
    endInsertRows();

    return true;
}

bool TableModelR::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent.isValid() || count != 1)
        return false;

    if (row < 0 || row >= entry_list_.size())
        return false;

    if (d_node_->reimbursement_status != ReimbursementStatus::kDraft)
        return false;

    auto* entry = static_cast<EntryR*>(entry_list_.at(row));
    const QUuid entry_id { entry->id };
    const double initial_delta { -entry->initial };

    beginRemoveRows(parent, row, row);
    entry_list_.removeAt(row);
    endRemoveRows();

    if (!qFuzzyIsNull(initial_delta))
        emit SSyncInitial(entry_id, initial_delta);

    if (entry->sync_state != SyncState::kCreating)
        pending_delete_.insert(entry_id);

    EntryPool::Instance().Recycle(entry, section_);
    return true;
}
