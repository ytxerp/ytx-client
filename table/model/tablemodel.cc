#include "tablemodel.h"

#include <QtConcurrent>

#include "global/entrypool.h"
#include "global/resourcepool.h"
#include "utils/compareutils.h"
#include "utils/entryutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableModel::TableModel(CTableModelArg& arg, QObject* parent)
    : QAbstractItemModel(parent)
    , info_ { arg.info }
    , direction_rule_ { arg.direction_rule }
    , lhs_id_ { arg.node_id }
    , section_ { arg.info.section }
{
}

TableModel::~TableModel()
{
    FlushCaches();
    ResourcePool<EntryShadow>::Instance().Recycle(shadow_list_);
}

void TableModel::RDirectionRule(bool value)
{
    for (auto* entry_shadow : std::as_const(shadow_list_))
        entry_shadow->balance = -entry_shadow->balance;

    direction_rule_ = value;
}

void TableModel::RRefreshStatus()
{
    const int column { std::to_underlying(EntryEnum::kStatus) };
    emit dataChanged(index(0, column), index(rowCount() - 1, column));
}

void TableModel::RRefreshField(const QUuid& entry_id, int start, int end)
{
    auto idx { GetIndex(entry_id) };
    if (!idx.isValid())
        return;

    int row { idx.row() };

    if (start > end)
        std::swap(start, end);

    emit dataChanged(index(row, start), index(row, end));
}

void TableModel::RAppendOneEntry(Entry* entry)
{
    auto* entry_shadow { ResourcePool<EntryShadow>::Instance().Allocate() };
    entry_shadow->BindEntry(entry, lhs_id_ == entry->lhs_node);

    auto row { shadow_list_.size() };

    beginInsertRows(QModelIndex(), row, row);
    shadow_list_.emplaceBack(entry_shadow);
    endInsertRows();

    const double previous_balance { row >= 1 ? shadow_list_.at(row - 1)->balance : 0.0 };
    entry_shadow->balance = CalculateBalance(entry_shadow) + previous_balance;

    const int balance_column { Utils::BalanceColumn(section_) };
    emit dataChanged(index(row, balance_column), index(row, balance_column));
}

void TableModel::RRemoveOneEntry(const QUuid& entry_id)
{
    auto idx { GetIndex(entry_id) };
    if (!idx.isValid())
        return;

    int row { idx.row() };
    beginRemoveRows(QModelIndex(), row, row);
    ResourcePool<EntryShadow>::Instance().Recycle(shadow_list_.takeAt(row));
    endRemoveRows();

    AccumulateBalance(row);
}

void TableModel::RUpdateBalance(const QUuid& entry_id)
{
    const auto entry_index { GetIndex(entry_id) };
    const int row { entry_index.row() };

    const auto [debit, credit] = Utils::EntryNumericColumnRange(section_);
    emit dataChanged(index(row, debit), index(row, credit));

    if (entry_index.isValid())
        AccumulateBalance(row);
}

void TableModel::ActionEntry(EntryAction action)
{
    if (shadow_list_.isEmpty())
        return;

    QJsonObject message { JsonGen::EntryAction(section_, lhs_id_, std::to_underlying(action)) };
    WebSocket::Instance()->SendMessage(kEntryAction, message);

    auto Update = [action](EntryShadow* entry_shadow) {
        switch (action) {
        case EntryAction::kMarkAll:
            *entry_shadow->status = std::to_underlying(EntryStatus::kMarked);
            break;
        case EntryAction::kMarkNone:
            *entry_shadow->status = std::to_underlying(EntryStatus::kUnmarked);
            break;
        case EntryAction::kMarkToggle:
            *entry_shadow->status ^= std::to_underlying(EntryStatus::kMarked);
            break;
        default:
            break;
        }
    };

    auto future { QtConcurrent::map(shadow_list_, Update) };
    auto* watcher { new QFutureWatcher<void>(this) };

    connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]() {
        const int column { std::to_underlying(EntryEnum::kStatus) };
        emit dataChanged(index(0, column), index(rowCount() - 1, column));

        watcher->deleteLater();
    });

    watcher->setFuture(future);
}

void TableModel::AccumulateBalance(int start)
{
    if (start <= -1 || start >= shadow_list_.size() || shadow_list_.isEmpty())
        return;

    double previous_balance { start >= 1 ? shadow_list_.at(start - 1)->balance : 0.0 };

    std::accumulate(shadow_list_.begin() + start, shadow_list_.end(), previous_balance, [&](double current_balance, EntryShadow* entry_shadow) {
        if (!entry_shadow)
            return current_balance;

        entry_shadow->balance = CalculateBalance(entry_shadow) + current_balance;
        return entry_shadow->balance;
    });

    const int balance_column { Utils::BalanceColumn(section_) };
    emit dataChanged(index(start, balance_column), index(rowCount() - 1, balance_column));
}

void TableModel::RestartTimer(const QUuid& id)
{
    if (pending_timers_.contains(id)) {
        pending_timers_[id]->stop();
    } else {
        auto* timer = new QTimer(this);
        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout, this, [this, id]() {
            auto* expired_timer { pending_timers_.take(id) };

            if (auto it = pending_updates_.find(id); it != pending_updates_.end()) {
                const auto& update { it.value() };

                if (!update.isEmpty()) {
                    const QJsonObject message { JsonGen::EntryUpdate(section_, id, update) };
                    WebSocket::Instance()->SendMessage(kEntryUpdate, message);
                }

                pending_updates_.erase(it);
            }

            expired_timer->deleteLater();
        });
        pending_timers_[id] = timer;
    }

    pending_timers_[id]->start(kThreeThousand);
}

void TableModel::FlushCaches()
{
    for (auto* timer : std::as_const(pending_timers_)) {
        timer->stop();
        timer->deleteLater();
    }

    pending_timers_.clear();

    for (auto it = pending_updates_.cbegin(); it != pending_updates_.cend(); ++it) {
        if (!it.value().isEmpty()) {
            const auto message { JsonGen::EntryUpdate(section_, it.key(), it.value()) };
            WebSocket::Instance()->SendMessage(kEntryUpdate, message);
        }
    }

    pending_updates_.clear();
}

QModelIndex TableModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.entry_header.at(section);

    return QVariant();
}

QModelIndex TableModel::GetIndex(const QUuid& entry_id) const
{
    int row { 0 };

    for (const auto* entry_shadow : shadow_list_) {
        if (*entry_shadow->id == entry_id) {
            return index(row, 0);
        }
        ++row;
    }
    return QModelIndex();
}

QVariant TableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* shadow = shadow_list_.at(index.row());

    const EntryEnum column { index.column() };

    switch (column) {
    case EntryEnum::kId:
        return *shadow->id;
    case EntryEnum::kUserId:
        return *shadow->user_id;
    case EntryEnum::kCreateTime:
        return *shadow->created_time;
    case EntryEnum::kCreateBy:
        return *shadow->created_by;
    case EntryEnum::kUpdateTime:
        return *shadow->updated_time;
    case EntryEnum::kUpdateBy:
        return *shadow->updated_by;
    case EntryEnum::kVersion:
        return *shadow->version;
    case EntryEnum::kIssuedTime:
        return *shadow->issued_time;
    case EntryEnum::kLhsNode:
        return *shadow->lhs_node;
    case EntryEnum::kCode:
        return *shadow->code;
    case EntryEnum::kLhsRate:
        return *shadow->lhs_rate;
    case EntryEnum::kDescription:
        return *shadow->description;
    case EntryEnum::kRhsNode:
        return *shadow->rhs_node;
    case EntryEnum::kStatus:
        return *shadow->status;
    case EntryEnum::kDocument:
        return *shadow->document;
    case EntryEnum::kDebit:
        return *shadow->lhs_debit;
    case EntryEnum::kCredit:
        return *shadow->lhs_credit;
    case EntryEnum::kBalance:
        return shadow->balance;
    default:
        return QVariant();
    }
}

bool TableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const EntryEnum column { index.column() };
    const int row { index.row() };

    auto* shadow { shadow_list_.at(index.row()) };

    if (IsFinished(lhs_id_, *shadow->rhs_node))
        return false;

    const QUuid id { *shadow->id };

    switch (column) {
    case EntryEnum::kIssuedTime: {
        const QDateTime new_time { value.toDateTime() };
        Utils::UpdateShadowIssuedTime(pending_updates_[id], shadow, kIssuedTime, new_time, &EntryShadow::issued_time, [id, this]() { RestartTimer(id); });
        last_issued_ = new_time;
        break;
    }
    case EntryEnum::kCode:
        Utils::UpdateShadowField(pending_updates_[id], shadow, kCode, value.toString(), &EntryShadow::code, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnum::kStatus:
        Utils::UpdateShadowField(pending_updates_[id], shadow, kStatus, value.toInt(), &EntryShadow::status, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnum::kDescription:
        Utils::UpdateShadowField(pending_updates_[id], shadow, kDescription, value.toString(), &EntryShadow::description, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnum::kDocument:
        Utils::UpdateShadowDocument(pending_updates_[id], shadow, kDocument, value.toStringList(), &EntryShadow::document, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnum::kLhsRate:
        UpdateRate(shadow, value.toDouble());
        break;
    case EntryEnum::kRhsNode:
        UpdateLinkedNode(shadow, value.toUuid(), row);
        break;
    case EntryEnum::kDebit:
        UpdateNumeric(shadow, value.toDouble(), row, true);
        break;
    case EntryEnum::kCredit:
        UpdateNumeric(shadow, value.toDouble(), row, false);
        break;
    case EntryEnum::kId:
    case EntryEnum::kUpdateBy:
    case EntryEnum::kUpdateTime:
    case EntryEnum::kCreateTime:
    case EntryEnum::kCreateBy:
    case EntryEnum::kVersion:
    case EntryEnum::kUserId:
    case EntryEnum::kLhsNode:
    case EntryEnum::kBalance:
        return false;
    }

    emit SResizeColumnToContents(index.column());
    return true;
}

void TableModel::sort(int column, Qt::SortOrder order)
{
    Q_ASSERT(column >= 0 && column < info_.entry_header.size());

    const EntryEnum e_column { column };
    if (e_column == EntryEnum::kBalance)
        return;

    auto Compare = [order, e_column](const EntryShadow* lhs, const EntryShadow* rhs) -> bool {
        switch (e_column) {
        case EntryEnum::kCode:
            return Utils::CompareShadowMember(lhs, rhs, &EntryShadow::code, order);
        case EntryEnum::kDescription:
            return Utils::CompareShadowMember(lhs, rhs, &EntryShadow::description, order);
        case EntryEnum::kIssuedTime:
            return Utils::CompareShadowMember(lhs, rhs, &EntryShadow::issued_time, order);
        case EntryEnum::kLhsRate:
            return Utils::CompareShadowMember(lhs, rhs, &EntryShadow::lhs_rate, order);
        case EntryEnum::kRhsNode:
            return Utils::CompareShadowMember(lhs, rhs, &EntryShadow::rhs_node, order);
        case EntryEnum::kStatus:
            return Utils::CompareShadowMember(lhs, rhs, &EntryShadow::status, order);
        case EntryEnum::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document->size() < rhs->document->size()) : (lhs->document->size() > rhs->document->size());
        case EntryEnum::kDebit:
            return Utils::CompareShadowMember(lhs, rhs, &EntryShadow::lhs_debit, order);
        case EntryEnum::kCredit:
            return Utils::CompareShadowMember(lhs, rhs, &EntryShadow::lhs_credit, order);
        case EntryEnum::kId:
        case EntryEnum::kUpdateBy:
        case EntryEnum::kUpdateTime:
        case EntryEnum::kCreateTime:
        case EntryEnum::kCreateBy:
        case EntryEnum::kVersion:
        case EntryEnum::kUserId:
        case EntryEnum::kLhsNode:
        case EntryEnum::kBalance:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(shadow_list_.begin(), shadow_list_.end(), Compare);
    emit layoutChanged();

    AccumulateBalance(0);
}

Qt::ItemFlags TableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const EntryEnum column { index.column() };

    switch (column) {
    case EntryEnum::kId:
    case EntryEnum::kBalance:
    case EntryEnum::kDocument:
    case EntryEnum::kStatus:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    const auto rhs_id { index.siblingAtColumn(std::to_underlying(EntryEnum::kRhsNode)).data().toUuid() };
    if (IsFinished(lhs_id_, rhs_id))
        flags &= ~Qt::ItemIsEditable;

    return flags;
}

bool TableModel::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    Q_ASSERT(row >= 0 && row <= rowCount(parent) - 1);

    auto* shadow = shadow_list_.at(row);
    const auto rhs_node_id { *shadow->rhs_node };

    if (IsFinished(lhs_id_, rhs_node_id))
        return false;

    beginRemoveRows(parent, row, row);
    shadow_list_.removeAt(row);
    endRemoveRows();

    const auto entry_id { *shadow->id };

    if (!rhs_node_id.isNull()) {
        QJsonObject message { JsonGen::EntryRemove(section_, entry_id) };
        WebSocket::Instance()->SendMessage(kEntryRemove, message);

        const double lhs_initial_delta { *shadow->lhs_credit - *shadow->lhs_debit };
        const bool has_leaf_delta { std::abs(lhs_initial_delta) > kTolerance };

        if (has_leaf_delta) {
            AccumulateBalance(row);
        }

        emit SRemoveOneEntry(rhs_node_id, entry_id);
    } else {
        EntryPool::Instance().Recycle(shadow->entry, section_);
    }

    ResourcePool<EntryShadow>::Instance().Recycle(shadow);
    return true;
}

EntryShadow* TableModel::InsertRowsImpl(int row, const QModelIndex& parent)
{
    auto* entry { EntryPool::Instance().Allocate(section_) };
    entry->id = QUuid::createUuidV7();

    auto* entry_shadow { ResourcePool<EntryShadow>::Instance().Allocate() };
    entry_shadow->BindEntry(entry, true);

    {
        *entry_shadow->lhs_node = lhs_id_;
        last_issued_ = last_issued_.isValid() ? last_issued_.addSecs(1) : QDateTime::currentDateTimeUtc();
        *entry_shadow->issued_time = last_issued_;

        const auto size { shadow_list_.size() };
        entry_shadow->balance = size >= 1 ? shadow_list_.at(size - 1)->balance : 0.0;
    }

    beginInsertRows(parent, row, row);
    shadow_list_.emplaceBack(entry_shadow);
    endInsertRows();

    return entry_shadow;
}

void TableModel::RRemoveMultiEntry(const QSet<QUuid>& entry_id_set)
{
    int min_row { std::numeric_limits<int>::max() };

    for (int i = shadow_list_.size() - 1; i >= 0; --i) {
        const auto entry_id { *shadow_list_[i]->id };

        if (entry_id_set.contains(entry_id)) {
            min_row = i;

            beginRemoveRows(QModelIndex(), i, i);
            ResourcePool<EntryShadow>::Instance().Recycle(shadow_list_.takeAt(i));
            endRemoveRows();
        }
    }

    if (min_row != std::numeric_limits<int>::max())
        AccumulateBalance(min_row);
}

void TableModel::RAppendMultiEntry(const EntryList& entry_list)
{
    EntryShadowList shadow_list {};
    for (auto* entry : entry_list) {
        auto* entry_shadow { ResourcePool<EntryShadow>::Instance().Allocate() };
        entry_shadow->BindEntry(entry, lhs_id_ == entry->lhs_node);
        shadow_list.emplaceBack(entry_shadow);
    }

    auto row { shadow_list_.size() };
    beginInsertRows(QModelIndex(), row, row + entry_list.size() - 1);
    shadow_list_.append(shadow_list);
    endInsertRows();

    sort(std::to_underlying(EntryEnum::kIssuedTime), Qt::AscendingOrder);
    AccumulateBalance(row);
}
