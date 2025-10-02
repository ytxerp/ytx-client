#include "leafmodel.h"

#include <QtConcurrent>

#include "global/entryshadowpool.h"
#include "global/websocket.h"
#include "utils/jsongen.h"

LeafModel::LeafModel(CLeafModelArg& arg, QObject* parent)
    : QAbstractItemModel(parent)
    , entry_hub_ { arg.entry_hub }
    , info_ { arg.info }
    , direction_rule_ { arg.direction_rule }
    , lhs_id_ { arg.node_id }
    , section_ { arg.info.section }
{
}

LeafModel::~LeafModel()
{
    FlushCaches();
    EntryShadowPool::Instance().Recycle(shadow_list_, section_);
}

void LeafModel::RSyncRule(bool value)
{
    for (auto* entry_shadow : std::as_const(shadow_list_))
        entry_shadow->balance = -entry_shadow->balance;

    direction_rule_ = value;
}

void LeafModel::RRefreshStatus()
{
    const int column { std::to_underlying(EntryEnum::kStatus) };
    emit dataChanged(index(0, column), index(rowCount() - 1, column));
}

void LeafModel::RRefreshField(const QUuid& entry_id, int start, int end)
{
    auto idx = GetIndex(entry_id);
    if (!idx.isValid())
        return;

    int row = idx.row();

    if (start > end)
        std::swap(start, end);

    emit dataChanged(index(row, start), index(row, end));
}

void LeafModel::RAppendOneEntry(Entry* entry)
{
    auto* entry_shadow { EntryShadowPool::Instance().Allocate(section_) };
    entry_shadow->BindEntry(entry, lhs_id_ == entry->lhs_node);

    auto row { shadow_list_.size() };

    beginInsertRows(QModelIndex(), row, row);
    shadow_list_.emplaceBack(entry_shadow);
    endInsertRows();

    const double previous_balance { row >= 1 ? shadow_list_.at(row - 1)->balance : 0.0 };
    entry_shadow->balance = CalculateBalance(entry_shadow) + previous_balance;
}

void LeafModel::RRemoveOneEntry(const QUuid& entry_id)
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

void LeafModel::RUpdateBalance(const QUuid& entry_id)
{
    auto index { GetIndex(entry_id) };
    if (index.isValid())
        AccumulateBalance(index.row());
}

void LeafModel::ActionEntry(EntryAction action)
{
    QJsonObject message = JsonGen::EntryAction(info_.section_str, lhs_id_, std::to_underlying(action));
    WebSocket::Instance()->SendMessage(kEntryAction, message);

    auto Update = [action](EntryShadow* entry_shadow) {
        switch (action) {
        case EntryAction::kMarkAll:
            *entry_shadow->status = true;
            break;
        case EntryAction::kMarkNone:
            *entry_shadow->status = false;
            break;
        case EntryAction::kMarkToggle:
            *entry_shadow->status = !*entry_shadow->status;
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

void LeafModel::AccumulateBalance(int start)
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
}

void LeafModel::RestartTimer(const QUuid& id)
{
    if (timers_.contains(id)) {
        timers_[id]->stop();
    } else {
        auto* timer = new QTimer(this);
        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout, this, [this, id, timer]() {
            timers_.remove(id);

            const auto cache { caches_.take(id) };
            if (cache.isEmpty()) {
                timer->deleteLater();
                return;
            }

            QJsonObject message { JsonGen::Update(info_.section_str, id, cache) };
            WebSocket::Instance()->SendMessage(kEntryUpdate, message);
            timer->deleteLater();
        });
        timers_[id] = timer;
    }

    timers_[id]->start(kThreeThousand);
}

void LeafModel::FlushCaches()
{
    for (auto* timer : std::as_const(timers_)) {
        timer->stop();
        timer->deleteLater();
    }

    timers_.clear();

    for (auto it = caches_.cbegin(); it != caches_.cend(); ++it) {
        if (!it.value().isEmpty()) {
            const auto message = JsonGen::Update(info_.section_str, it.key(), it.value());
            WebSocket::Instance()->SendMessage(kNodeUpdate, message);
        }
    }

    caches_.clear();
}

QModelIndex LeafModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QVariant LeafModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.entry_header.at(section);

    return QVariant();
}

int LeafModel::GetRhsRow(const QUuid& rhs_id) const
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

QModelIndex LeafModel::GetIndex(const QUuid& entry_id) const
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

bool LeafModel::insertRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent));

    auto* entry_shadow { entry_hub_->AllocateEntryShadow() };

    *entry_shadow->lhs_node = lhs_id_;

    beginInsertRows(parent, row, row);
    shadow_list_.emplaceBack(entry_shadow);
    endInsertRows();

    return true;
}

void LeafModel::RRemoveMultiEntry(const QSet<QUuid>& entry_id_set)
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

void LeafModel::RAppendMultiEntry(const EntryList& entry_list)
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

    sort(std::to_underlying(EntryEnum::kIssuedTime));
    AccumulateBalance(row);
}
