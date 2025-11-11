#include "tablemodel.h"

#include <QtConcurrent>

#include "global/entrypool.h"
#include "global/entryshadowpool.h"
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
    EntryShadowPool::Instance().Recycle(shadow_list_, section_);
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
    auto* entry_shadow { EntryShadowPool::Instance().Allocate(section_) };
    entry_shadow->BindEntry(entry, lhs_id_ == entry->lhs_node);

    auto row { shadow_list_.size() };

    beginInsertRows(QModelIndex(), row, row);
    shadow_list_.emplaceBack(entry_shadow);
    endInsertRows();

    const double previous_balance { row >= 1 ? shadow_list_.at(row - 1)->balance : 0.0 };
    entry_shadow->balance = CalculateBalance(entry_shadow) + previous_balance;

    const int balance_column { EntryUtils::BalanceColumn(section_) };
    emit dataChanged(index(row, balance_column), index(row, balance_column));
}

void TableModel::RRemoveOneEntry(const QUuid& entry_id)
{
    auto idx { GetIndex(entry_id) };
    if (!idx.isValid())
        return;

    int row { idx.row() };
    beginRemoveRows(QModelIndex(), row, row);
    EntryShadowPool::Instance().Recycle(shadow_list_.takeAt(row), section_);
    endRemoveRows();

    AccumulateBalance(row);
}

void TableModel::RUpdateBalance(const QUuid& entry_id)
{
    const auto entry_index { GetIndex(entry_id) };
    const int row { entry_index.row() };

    const auto [debit, credit] = EntryUtils::NumericColumnRange(section_);
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
            *entry_shadow->status = std::to_underlying(EntryStatus::kMarked) - *entry_shadow->status;
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

    const int balance_column { EntryUtils::BalanceColumn(section_) };
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

int TableModel::GetRhsRow(const QUuid& rhs_id) const
{
    int row { 0 };

    for (const auto* entry_shadow : shadow_list_) {
        if (*entry_shadow->rhs_node == rhs_id) {
            return row;
        }
        ++row;
    }
    return -1;
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

bool TableModel::insertRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent));

    auto* entry_shadow { InsertRowsImpl(row, parent) };
    *entry_shadow->issued_time = QDateTime::currentDateTimeUtc();

    emit SInsertEntry(entry_shadow->entry);
    return true;
}

EntryShadow* TableModel::InsertRowsImpl(int row, const QModelIndex& parent)
{
    auto* entry { EntryPool::Instance().Allocate(section_) };
    entry->id = QUuid::createUuidV7();

    auto* entry_shadow { EntryShadowPool::Instance().Allocate(section_) };
    entry_shadow->BindEntry(entry, true);

    *entry_shadow->lhs_node = lhs_id_;

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
            EntryShadowPool::Instance().Recycle(shadow_list_.takeAt(i), section_);
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
        auto* entry_shadow { EntryShadowPool::Instance().Allocate(section_) };
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
