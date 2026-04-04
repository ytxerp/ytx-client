#include "tagmodel.h"

#include <QJsonObject>
#include <QRandomGenerator>

#include "component/constantwebsocket.h"
#include "enum/tagenum.h"
#include "global/resourcepool.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

static const QStringList kHeaders = {
    QObject::tr("Id"),
    QObject::tr("Name"),
    QObject::tr("Color"),
};

TagModel::TagModel(Section section, const QHash<QUuid, Tag*>& tag_hash, QObject* parent)
    : QAbstractItemModel(parent)
    , section_ { section }
{
    for (auto it = tag_hash.cbegin(); it != tag_hash.cend(); ++it) {
        tag_list_.append(it.value());
        names_.insert(it.value()->name);
    }

    std::sort(tag_list_.begin(), tag_list_.end(), [](const Tag* a, const Tag* b) { return a->name < b->name; });
}

QVariant TagModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return {};

    if (role != Qt::DisplayRole)
        return {};

    if (section < 0 || section >= kHeaders.size())
        return {};

    return kHeaders.at(section);
}

QModelIndex TagModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, tag_list_.at(row));
}

int TagModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return kHeaders.size();
}

QVariant TagModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* tag { tag_list_.at(index.row()) };
    if (tag->id.isNull())
        return {};

    const TagEnum column { index.column() };

    switch (column) {
    case TagEnum::kId:
        return tag->id;
    case TagEnum::kName:
        return tag->name;
    case TagEnum::kColor:
        return tag->color;
    }
}

bool TagModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    auto* tag { tag_list_.at(index.row()) };
    if (tag->id.isNull())
        return {};

    switch (static_cast<TagEnum>(index.column())) {
    case TagEnum::kName:
        UpdateName(tag, value.toString());
        break;
    case TagEnum::kColor:
        UpdateColor(tag, value.toString());
        break;
    case TagEnum::kId:
        return false;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    return true;
}

void TagModel::sort(int column, Qt::SortOrder order)
{
    const TagEnum e_column { column };

    auto Compare = [order, e_column](const Tag* lhs, const Tag* rhs) -> bool {
        switch (e_column) {
        case TagEnum::kName:
            return Utils::CompareMember(lhs, rhs, &Tag::name, order);
        case TagEnum::kColor:
            return Utils::CompareMember(lhs, rhs, &Tag::color, order);
        case TagEnum::kId:
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

    switch (static_cast<TagEnum>(index.column())) {
    case TagEnum::kName:
        flags |= Qt::ItemIsEditable;
        break;
    case TagEnum::kId:
    case TagEnum::kColor:
        flags &= ~Qt::ItemIsEditable;
        break;
    }

    return flags;
}

bool TagModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (count != 1 || row < 0 || row > tag_list_.size())
        return false;

    auto* tag { ResourcePool<Tag>::Instance().Allocate() };

    tag->id = QUuid::createUuidV7();
    tag->state = SyncState::kNew;

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
    Tag* tag { tag_list_.at(row) };
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
    if (tag->state == SyncState::kSynced) {
        const QJsonObject message { JsonGen::TagDelete(section_, tag_id) };
        WebSocket::Instance()->SendMessage(WsKey::kTagDelete, message);
    } else {
        ResourcePool<Tag>::Instance().Recycle(tag);
    }

    return true;
}

bool TagModel::UpdateName(Tag* tag, const QString& new_name)
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

    if (tag->state == SyncState::kNew && !tag->color.isEmpty()) {
        TryInsert(tag);
    } else if (tag->state == SyncState::kSynced) {
        pending_updates_[tag->id].insert(kName, new_name);
        RestartTimer(tag->id);
    }

    return true;
}

bool TagModel::UpdateColor(Tag* tag, const QString& new_color)
{
    if (!tag || tag->color == new_color)
        return false;

    tag->color = new_color;

    if (tag->state == SyncState::kNew && !tag->name.isEmpty()) {
        TryInsert(tag);
    } else if (tag->state == SyncState::kSynced) {
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
    timer->start(TimeConst::kAutoCloseMs);
}

void TagModel::TryInsert(Tag* tag)
{
    if (!tag || tag->state != SyncState::kNew)
        return;

    tag->state = SyncState::kDirty;

    const QJsonObject message { JsonGen::TagInsert(section_, tag) };
    WebSocket::Instance()->SendMessage(WsKey::kTagInsert, message);

    emit SInsertingTag(tag);
}
