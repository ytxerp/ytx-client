#include "tablemodelr.h"

#include <QJsonArray>

#include "component/constantstring.h"
#include "enum/entryenum.h"
#include "global/entrypool.h"
#include "utils/templateutils.h"

TableModelR::TableModelR(CTableModelArg& arg, QObject* parent)
    : TableModelSingleDeferred { arg, parent }
    , d_node_ { static_cast<NodeR*>(arg.node) }
{
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
    case EntryEnumR::kStatus:
        return entry->status;
    }
}

bool TableModelR::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    const EntryEnumR column { index.column() };
    auto* row { static_cast<EntryR*>(index.internalPointer()) };

    if (column == EntryEnumR::kStatus) {
        row->status = value.toInt();

        emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
        return true;
    }

    if (d_node_->reimbursement_status != ReimbursementStatus::kDraft || d_node_->sync_state == SyncState::kUpdating)
        return false;

    const double old_initial { row->initial };

    switch (column) {
    case EntryEnumR::kDescription:
        UpdateDescription(row, value.toString());
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
    case EntryEnumR::kStatus:
        break;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    emit SDataChanged();

    const double initial_delta { row->initial - old_initial };
    if (!qFuzzyIsNull(initial_delta))
        emit SSyncDelta(node_id_, entry::Delta { .initial = initial_delta });

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

void TableModelR::sort(int column, Qt::SortOrder order)
{
    const EntryEnumR e_column { column };

    auto Compare = [e_column, order](const Entry* lhs, const Entry* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryR>(lhs) };
        auto* d_rhs { DerivedPtr<EntryR>(rhs) };

        switch (e_column) {
        case EntryEnumR::kDescription:
            return utils::CompareMember(lhs, rhs, &Entry::description, order);
        case EntryEnumR::kDocument:
            return utils::CompareMember(lhs, rhs, &Entry::document, order);
        case EntryEnumR::kInitial:
            return utils::CompareMember(d_lhs, d_rhs, &EntryR::initial, order);
        case EntryEnumR::kTag:
            return utils::CompareMember(lhs, rhs, &Entry::tag, order);
        case EntryEnumR::kIssuedTime:
            return utils::CompareMember(lhs, rhs, &Entry::issued_time, order);
        case EntryEnumR::kLhsNode:
            return false;
        case EntryEnumR::kStatus:
            return utils::CompareMember(lhs, rhs, &Entry::status, order);
        }
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(entry_list_, Compare);
    emit layoutChanged();
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

bool TableModelR::HasZeroValue() const
{
    for (const Entry* entry : entry_list_) {
        const auto* d_entry { static_cast<const EntryR*>(entry) };

        if (qFuzzyIsNull(d_entry->initial))
            return true;
    }

    return false;
}

Qt::ItemFlags TableModelR::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };

    if (d_node_->reimbursement_status != ReimbursementStatus::kDraft || d_node_->sync_state == SyncState::kUpdating)
        return flags;

    const EntryEnumR column { index.column() };

    switch (column) {
    case EntryEnumR::kDescription:
    case EntryEnumR::kInitial:
        flags |= Qt::ItemIsEditable;
        break;
    case EntryEnumR::kDocument:
    case EntryEnumR::kTag:
    case EntryEnumR::kLhsNode:
    case EntryEnumR::kIssuedTime:
    case EntryEnumR::kStatus:
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

    if (d_node_->reimbursement_status != ReimbursementStatus::kDraft || d_node_->sync_state == SyncState::kUpdating)
        return false;

    auto* entry { EntryPool::Instance().Allocate(section_) };

    entry->id = QUuid::createUuidV7();
    entry->lhs_node = node_id_;
    entry->sync_state = SyncState::kCreating;
    entry->issued_time = QDateTime::currentDateTime();

    beginInsertRows(parent, row, row);
    entry_list_.insert(row, entry);
    endInsertRows();

    emit SDataChanged();
    return true;
}

bool TableModelR::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent.isValid() || count != 1)
        return false;

    if (row < 0 || row >= entry_list_.size())
        return false;

    if (d_node_->reimbursement_status != ReimbursementStatus::kDraft || d_node_->sync_state == SyncState::kUpdating)
        return false;

    auto* entry = static_cast<EntryR*>(entry_list_.at(row));
    const QUuid entry_id { entry->id };
    const double initial_delta { -entry->initial };

    beginRemoveRows(parent, row, row);
    entry_list_.removeAt(row);
    endRemoveRows();

    if (!qFuzzyIsNull(initial_delta))
        emit SSyncDelta(node_id_, entry::Delta { .initial = initial_delta });

    if (entry->sync_state != SyncState::kCreating)
        pending_delete_.insert(entry_id);

    EntryPool::Instance().Recycle(entry, section_);
    emit SDataChanged();
    return true;
}
