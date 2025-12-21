#include "searchentrymodel.h"

#include "websocket/jsongen.h"
#include "websocket/websocket.h"

SearchEntryModel::SearchEntryModel(CSectionInfo& info, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
{
}

void SearchEntryModel::RSearchEntry(const EntryList& entry_list)
{
    beginResetModel();
    entry_list_ = entry_list;
    endResetModel();
}

QModelIndex SearchEntryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex SearchEntryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SearchEntryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return entry_list_.size();
}

int SearchEntryModel::columnCount(const QModelIndex& /*parent*/) const { return info_.full_entry_header.size(); }

QVariant SearchEntryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.full_entry_header.at(section);

    return QVariant();
}

void SearchEntryModel::Search(const QString& text)
{
    if (text.isEmpty()) {
        beginResetModel();
        entry_list_.clear();
        endResetModel();
        return;
    }

    WebSocket::Instance()->SendMessage(kEntrySearch, JsonGen::EntrySearch(info_.section, text));
}

QVariant SearchEntryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* entry { entry_list_.at(index.row()) };
    const FullEntryEnum column { index.column() };

    switch (column) {
    case FullEntryEnum::kId:
        return entry->id;
    case FullEntryEnum::kIssuedTime:
        return entry->issued_time;
    case FullEntryEnum::kCode:
        return entry->code;
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
    default:
        return QVariant();
    }
}

void SearchEntryModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.entry_header.size() - 1)
        return;

    auto Compare = [column, order](const Entry* lhs, const Entry* rhs) -> bool {
        const FullEntryEnum e_column { column };

        switch (e_column) {
        case FullEntryEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case FullEntryEnum::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case FullEntryEnum::kLhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_node < rhs->lhs_node) : (lhs->lhs_node > rhs->lhs_node);
        case FullEntryEnum::kLhsRate:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_rate < rhs->lhs_rate) : (lhs->lhs_rate > rhs->lhs_rate);
        case FullEntryEnum::kLhsDebit:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_debit < rhs->lhs_debit) : (lhs->lhs_debit > rhs->lhs_debit);
        case FullEntryEnum::kLhsCredit:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_credit < rhs->lhs_credit) : (lhs->lhs_credit > rhs->lhs_credit);
        case FullEntryEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case FullEntryEnum::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document.size() < rhs->document.size()) : (lhs->document.size() > rhs->document.size());
        case FullEntryEnum::kStatus:
            return (order == Qt::AscendingOrder) ? (lhs->status < rhs->status) : (lhs->status > rhs->status);
        case FullEntryEnum::kRhsCredit:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_credit < rhs->rhs_credit) : (lhs->rhs_credit > rhs->rhs_credit);
        case FullEntryEnum::kRhsDebit:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_debit < rhs->rhs_debit) : (lhs->rhs_debit > rhs->rhs_debit);
        case FullEntryEnum::kRhsRate:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_rate < rhs->rhs_rate) : (lhs->rhs_rate > rhs->rhs_rate);
        case FullEntryEnum::kRhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_node < rhs->rhs_node) : (lhs->rhs_node > rhs->rhs_node);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(entry_list_.begin(), entry_list_.end(), Compare);
    emit layoutChanged();
}
