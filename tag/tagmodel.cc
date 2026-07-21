#include "tagmodel.h"

#include <QJsonObject>
#include <QRandomGenerator>

#include "component/constantwebsocket.h"
#include "global/resourcepool.h"
#include "tag/tagenum.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TagModel::TagModel(Section section, const QHash<QUuid, TagRow*>& tag_hash, const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
    , section_ { section }
    , header_ { header }
{
    for (auto it = tag_hash.cbegin(); it != tag_hash.cend(); ++it) {
        tag_list_.append(it.value());
        names_.insert(it.value()->name);
    }

    std::sort(tag_list_.begin(), tag_list_.end(), [](const TagRow* a, const TagRow* b) { return a->name < b->name; });
}

TagModel::~TagModel() { FlushCaches(); }

QVariant TagModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

QModelIndex TagModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, tag_list_.at(row));
}

QVariant TagModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    const TagRowField column { index.column() };
    auto* tag { static_cast<TagRow*>(index.internalPointer()) };

    switch (column) {
    case TagRowField::kId:
        return tag->id;
    case TagRowField::kName:
        return tag->name;
    case TagRowField::kColor:
        return tag->color;
    }
}

bool TagModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    const TagRowField column { index.column() };
    auto* tag { static_cast<TagRow*>(index.internalPointer()) };

    switch (column) {
    case TagRowField::kName:
        UpdateName(tag, value.toString());
        break;
    case TagRowField::kColor:
        UpdateColor(tag, value.toString());
        break;
    case TagRowField::kId:
        return false;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    return true;
}

void TagModel::sort(int column, Qt::SortOrder order)
{
    const TagRowField e_column { column };

    auto Compare = [order, e_column](const TagRow* lhs, const TagRow* rhs) -> bool {
        switch (e_column) {
        case TagRowField::kName:
            return utils::CompareMember(lhs, rhs, &TagRow::name, order);
        case TagRowField::kColor:
            return utils::CompareMember(lhs, rhs, &TagRow::color, order);
        case TagRowField::kId:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(tag_list_.begin(), tag_list_.end(), Compare);
    emit layoutChanged();
}

Qt::ItemFlags TagModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags { QAbstractItemModel::flags(index) };

    switch (static_cast<TagRowField>(index.column())) {
    case TagRowField::kName:
        flags |= Qt::ItemIsEditable;
        break;
    case TagRowField::kId:
    case TagRowField::kColor:
        flags &= ~Qt::ItemIsEditable;
        break;
    }

    return flags;
}

bool TagModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (count != 1 || row < 0 || row > tag_list_.size())
        return false;

    auto* tag { ResourcePool<TagRow>::Instance().Allocate() };

    tag->id = QUuid::createUuidV7();
    tag->sync_state = SyncState::kCreating;

    const QColor color { QColor::fromHsv(
        QRandomGenerator::global()->bounded(360), QRandomGenerator::global()->bounded(128, 256), QRandomGenerator::global()->bounded(180, 256)) };
    tag->color = color.name(QColor::HexArgb);

    beginInsertRows(parent, row, row);

    tag_list_.insert(row, tag);

    endInsertRows();

    return true;
}

bool TagModel::removeRows(int row, int count, const QModelIndex& parent)
{
    // Basic validation
    if (count != 1 || row < 0 || row >= tag_list_.size()) {
        return false;
    }

    // Capture the pointer and necessary values using {} initialization
    TagRow* tag { tag_list_.at(row) };
    const QUuid tag_id { tag->id };
    const QString tag_name { tag->name };

    // Clean up the pending timer first (Safety)
    // Prevent the timer from firing after the tag is recycled.
    if (pending_timers_.contains(tag_id)) {
        auto* timer { pending_timers_.take(tag_id) };
        timer->stop();
        timer->deleteLater();
    }
    pending_updates_.remove(tag_id);

    // Notify views that rows are about to be removed
    beginRemoveRows(parent, row, row);

    // Remove from all internal containers
    tag_list_.removeAt(row);
    names_.remove(tag_name);

    endRemoveRows();

    // Handle synchronization and network notification
    if (tag->sync_state == SyncState::kSynced) {
        const QJsonObject message { JsonGen::TagDelete(section_, tag_id) };
        WebSocket::Instance()->SendMessage(WsKey::kTagDelete, message);
    } else {
        ResourcePool<TagRow>::Instance().Recycle(tag);
    }

    return true;
}

bool TagModel::UpdateName(TagRow* tag, const QString& new_name)
{
    if (!tag)
        return false;

    const QString old_name { tag->name };

    if (old_name == new_name || new_name.isEmpty())
        return false;

    if (names_.contains(new_name))
        return false;

    tag->name = new_name;

    names_.insert(new_name);
    names_.remove(old_name);

    if (tag->sync_state == SyncState::kCreating) {
        TryInsert(tag);
    } else if (tag->sync_state == SyncState::kSynced) {
        pending_updates_[tag->id].insert(kName, new_name);
        RestartTimer(tag->id);
    }

    return true;
}

bool TagModel::UpdateColor(TagRow* tag, const QString& new_color)
{
    if (!tag || tag->color == new_color)
        return false;

    tag->color = new_color;

    if (tag->sync_state == SyncState::kCreating && !tag->name.isEmpty()) {
        TryInsert(tag);
    } else if (tag->sync_state == SyncState::kSynced) {
        pending_updates_[tag->id].insert(kColor, new_color);
        RestartTimer(tag->id);
    }

    return true;
}

void TagModel::RestartTimer(const QUuid& id)
{
    // Try to retrieve the existing timer
    QTimer* timer { pending_timers_.value(id, nullptr) };

    if (!timer) {
        // Create and configure a new timer if it does not exist
        timer = new QTimer { this };
        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout, this, [this, id]() {
            // Manage lifecycle by taking the timer from the hash
            auto* expired_timer { pending_timers_.take(id) };

            // Retrieve and remove the pending update content in one go
            const auto update { pending_updates_.take(id) };

            // Only send the message if there are actual changes
            if (!update.isEmpty()) {
                const QJsonObject message { JsonGen::TagUpdate(section_, id, update) };
                WebSocket::Instance()->SendMessage(WsKey::kTagUpdate, message);
            }

            // Always clear the update flag
            pending_updates_.remove(id);

            if (expired_timer) {
                expired_timer->deleteLater();
            }
        });

        pending_timers_[id] = timer;
    }

    // Start or restart the timer
    timer->start(time_const::kAutoCloseMs);
}

void TagModel::FlushCaches()
{
    if (pending_updates_.isEmpty())
        return;

    for (auto* timer : std::as_const(pending_timers_)) {
        timer->stop();
        timer->deleteLater();
    }

    pending_timers_.clear();

    for (auto it = pending_updates_.cbegin(); it != pending_updates_.cend(); ++it) {
        if (!it.value().isEmpty()) {
            const QJsonObject message { JsonGen::TagUpdate(section_, it.key(), it.value()) };
            WebSocket::Instance()->SendMessage(WsKey::kTagUpdate, message);
        }
    }

    pending_updates_.clear();
}

void TagModel::TryInsert(TagRow* tag)
{
    if (!tag || tag->sync_state != SyncState::kCreating)
        return;

    tag->sync_state = SyncState::kUpdating;

    const QJsonObject message { JsonGen::TagInsert(section_, tag) };
    WebSocket::Instance()->SendMessage(WsKey::kTagInsert, message);

    emit SInsertingTag(tag);
}
