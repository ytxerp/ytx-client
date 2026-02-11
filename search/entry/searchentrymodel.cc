#include "searchentrymodel.h"

#include "utils/compareutils.h"
#include "utils/tagutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

SearchEntryModel::SearchEntryModel(CSectionInfo& info, const QHash<QUuid, Tag*>& tag_hash, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
    , tag_hash_ { tag_hash }
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
    // 1. Handle empty search input: clear the model if it has data
    if (text.trimmed().isEmpty()) {
        ClearModel();
        return;
    }

    // 2. Parse the search input into text and tag set
    const SearchQuery query { Utils::ParseSearchQuery(text, tag_hash_) };

    // 3. Perform the search (tag search has higher priority)
    if (!query.tags.isEmpty()) {
        WebSocket::Instance()->SendMessage(kEntryTagSearch, JsonGen::EntryTagSearch(info_.section, query.tags));
        return;
    }

    // 4.Send description text search request to server
    if (!query.text.isEmpty()) {
        WebSocket::Instance()->SendMessage(kEntryDescriptionSearch, JsonGen::EntryDescriptionSearch(info_.section, query.text));
        return;
    }

    // 5.Both tags and text are empty → clear the model
    ClearModel();
}

void SearchEntryModel::ClearModel()
{
    if (!entry_list_.isEmpty()) {
        beginResetModel();
        entry_list_.clear();
        endResetModel();
    }
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
    case FullEntryEnum::kUpdateBy:
        return entry->updated_by;
    case FullEntryEnum::kUpdateTime:
        return entry->updated_time;
    case FullEntryEnum::kCreateTime:
        return entry->created_time;
    case FullEntryEnum::kCreateBy:
        return entry->created_by;
    case FullEntryEnum::kVersion:
        return entry->version;
    case FullEntryEnum::kUserId:
        return entry->user_id;
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
    default:
        return QVariant();
    }
}

void SearchEntryModel::sort(int column, Qt::SortOrder order)
{
    const FullEntryEnum e_column { column };

    auto Compare = [e_column, order](const Entry* lhs, const Entry* rhs) -> bool {
        switch (e_column) {
        case FullEntryEnum::kIssuedTime:
            return Utils::CompareMember(lhs, rhs, &Entry::issued_time, order);
        case FullEntryEnum::kCode:
            return Utils::CompareMember(lhs, rhs, &Entry::code, order);
        case FullEntryEnum::kLhsNode:
            return Utils::CompareMember(lhs, rhs, &Entry::lhs_node, order);
        case FullEntryEnum::kLhsRate:
            return Utils::CompareMember(lhs, rhs, &Entry::lhs_rate, order);
        case FullEntryEnum::kLhsDebit:
            return Utils::CompareMember(lhs, rhs, &Entry::lhs_debit, order);
        case FullEntryEnum::kLhsCredit:
            return Utils::CompareMember(lhs, rhs, &Entry::lhs_credit, order);
        case FullEntryEnum::kDescription:
            return Utils::CompareMember(lhs, rhs, &Entry::description, order);
        case FullEntryEnum::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document.size() < rhs->document.size()) : (lhs->document.size() > rhs->document.size());
        case FullEntryEnum::kTag:
            return Utils::CompareMember(lhs, rhs, &Entry::tag, order);
        case FullEntryEnum::kStatus:
            return Utils::CompareMember(lhs, rhs, &Entry::status, order);
        case FullEntryEnum::kRhsCredit:
            return Utils::CompareMember(lhs, rhs, &Entry::rhs_credit, order);
        case FullEntryEnum::kRhsDebit:
            return Utils::CompareMember(lhs, rhs, &Entry::rhs_debit, order);
        case FullEntryEnum::kRhsRate:
            return Utils::CompareMember(lhs, rhs, &Entry::rhs_rate, order);
        case FullEntryEnum::kRhsNode:
            return Utils::CompareMember(lhs, rhs, &Entry::rhs_node, order);
        case FullEntryEnum::kId:
        case FullEntryEnum::kUpdateBy:
        case FullEntryEnum::kUpdateTime:
        case FullEntryEnum::kCreateTime:
        case FullEntryEnum::kCreateBy:
        case FullEntryEnum::kVersion:
        case FullEntryEnum::kUserId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(entry_list_.begin(), entry_list_.end(), Compare);
    emit layoutChanged();
}
