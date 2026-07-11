#include "periodclosemodel.h"

#include "enum/entryenum.h"
#include "utils/templateutils.h"

namespace period_close {

Model::Model(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel(parent)
    , info_ { info }
{
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.full_entry_header.at(section);

    return QVariant();
}

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QModelIndex Model::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int Model::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return list_.size();
}

int Model::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_.full_entry_header.size();
}

void Model::sort(int column, Qt::SortOrder order)
{
    const FullEntryEnum e_column { column };

    auto Compare = [e_column, order](const Entry* lhs, const Entry* rhs) -> bool {
        switch (e_column) {
        case FullEntryEnum::kLhsNode:
            return utils::CompareMember(lhs, rhs, &Entry::lhs_node, order);
        case FullEntryEnum::kLhsRate:
            return utils::CompareMember(lhs, rhs, &Entry::lhs_rate, order);
        case FullEntryEnum::kLhsDebit:
            return utils::CompareMember(lhs, rhs, &Entry::lhs_debit, order);
        case FullEntryEnum::kLhsCredit:
            return utils::CompareMember(lhs, rhs, &Entry::lhs_credit, order);
        case FullEntryEnum::kRhsCredit:
            return utils::CompareMember(lhs, rhs, &Entry::rhs_credit, order);
        case FullEntryEnum::kRhsDebit:
            return utils::CompareMember(lhs, rhs, &Entry::rhs_debit, order);
        case FullEntryEnum::kRhsRate:
            return utils::CompareMember(lhs, rhs, &Entry::rhs_rate, order);
        case FullEntryEnum::kRhsNode:
            return utils::CompareMember(lhs, rhs, &Entry::rhs_node, order);
        case FullEntryEnum::kId:
        case FullEntryEnum::kVersion:
        case FullEntryEnum::kIssuedTime:
        case FullEntryEnum::kCode:
        case FullEntryEnum::kDescription:
        case FullEntryEnum::kDocument:
        case FullEntryEnum::kTag:
        case FullEntryEnum::kStatus:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

QVariant Model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const FullEntryEnum column { index.column() };
    auto* entry { static_cast<Entry*>(index.internalPointer()) };

    switch (column) {
    case FullEntryEnum::kId:
        return entry->id;
    case FullEntryEnum::kVersion:
        return entry->version;
    case FullEntryEnum::kIssuedTime:
        return entry->issued_time;
    case FullEntryEnum::kCode:
        return entry->code;
    case FullEntryEnum::kTag:
        return entry->tag;
    case FullEntryEnum::kLhsNode:
        return entry->lhs_node;
    case FullEntryEnum::kLhsRate:
        return entry->lhs_rate;
    case FullEntryEnum::kLhsDebit:
        return entry->lhs_debit;
    case FullEntryEnum::kLhsCredit:
        return entry->lhs_credit;
    case FullEntryEnum::kDescription:
        return entry->description;
    case FullEntryEnum::kDocument:
        return entry->document;
    case FullEntryEnum::kStatus:
        return entry->status;
    case FullEntryEnum::kRhsCredit:
        return entry->rhs_credit;
    case FullEntryEnum::kRhsDebit:
        return entry->rhs_debit;
    case FullEntryEnum::kRhsRate:
        return entry->rhs_rate;
    case FullEntryEnum::kRhsNode:
        return entry->rhs_node;
    }
}

void Model::Rebuild(const QList<Entry*>& list)
{
    beginResetModel();
    list_ = list;
    endResetModel();
}
}