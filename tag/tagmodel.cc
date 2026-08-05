#include "tagmodel.h"

#include <QJsonObject>
#include <QRandomGenerator>

#include "component/constantwebsocket.h"
#include "global/resourcepool.h"
#include "tag/tagenum.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

namespace tag {

Model::Model(Section section, const QHash<QUuid, Row*>& tag_hash, const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
    , section_ { section }
    , header_ { header }
{
    for (auto it = tag_hash.cbegin(); it != tag_hash.cend(); ++it) {
        Row* tag { it.value() };

        if (!tag)
            continue;

        names_.insert(tag->name);
        list_.append(tag);
    }

    std::sort(list_.begin(), list_.end(), [](const Row* a, const Row* b) { return a->name < b->name; });
}

Model::~Model() { FlushCaches(); }

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QVariant Model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    const RowField column { index.column() };
    auto* tag { static_cast<Row*>(index.internalPointer()) };

    switch (column) {
    case RowField::kName:
        return tag->name;
    case RowField::kColor:
        return tag->color;
    }
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return false;

    const RowField column { index.column() };
    auto* tag { static_cast<Row*>(index.internalPointer()) };

    switch (column) {
    case RowField::kName:
        UpdateName(tag, value.toString());
        break;
    case RowField::kColor:
        UpdateColor(tag, value.toString());
        break;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    return true;
}

void Model::sort(int column, Qt::SortOrder order)
{
    const RowField e_column { column };

    auto Compare = [order, e_column](const Row* lhs, const Row* rhs) -> bool {
        switch (e_column) {
        case RowField::kName:
            return utils::CompareMember(lhs, rhs, &Row::name, order);
        case RowField::kColor:
            return utils::CompareMember(lhs, rhs, &Row::color, order);
        }
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(list_, Compare);
    emit layoutChanged();
}

Qt::ItemFlags Model::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags { QAbstractItemModel::flags(index) };

    switch (static_cast<RowField>(index.column())) {
    case RowField::kName:
        flags |= Qt::ItemIsEditable;
        break;
    case RowField::kColor:
        flags &= ~Qt::ItemIsEditable;
        break;
    }

    return flags;
}

bool Model::insertRows(int row, int count, const QModelIndex& parent)
{
    if (count != 1 || row < 0 || row > list_.size())
        return false;

    auto* tag { ResourcePool<Row>::Instance().Allocate() };

    tag->id = QUuid::createUuidV7();
    tag->sync_state = SyncState::kCreating;

    const QColor color { QColor::fromHsv(
        QRandomGenerator::global()->bounded(360), QRandomGenerator::global()->bounded(128, 256), QRandomGenerator::global()->bounded(180, 256)) };
    tag->color = color.name(QColor::HexArgb);

    beginInsertRows(parent, row, row);
    list_.insert(row, tag);
    endInsertRows();

    return true;
}

bool Model::removeRows(int row, int count, const QModelIndex& parent)
{
    // Basic validation
    if (count != 1 || row < 0 || row >= list_.size()) {
        return false;
    }

    // Capture the pointer and necessary values using {} initialization
    Row* tag { list_.at(row) };
    const QUuid tag_id { tag->id };
    const QString tag_name { tag->name };

    // Clean up the pending timer first (Safety)
    // Prevent the timer from firing after the tag is recycled.
    if (auto* timer { pending_timers_.take(tag_id) }) {
        timer->stop();
        timer->deleteLater();
    }

    pending_updates_.remove(tag_id);
    names_.remove(tag_name);

    // Notify views that rows are about to be removed
    beginRemoveRows(parent, row, row);

    list_.removeAt(row);

    endRemoveRows();

    // Handle synchronization and network notification
    if (tag->sync_state == SyncState::kSynced) {
        const QJsonObject message { JsonGen::TagDelete(section_, tag_id) };
        WebSocket::Instance()->SendMessage(WsKey::kTagDelete, message);
    } else {
        ResourcePool<Row>::Instance().Recycle(tag);
    }

    return true;
}

bool Model::UpdateName(Row* tag, const QString& name)
{
    if (!tag)
        return false;

    const QString new_name { name.simplified() };
    if (new_name.isEmpty())
        return false;

    const QString old_name { tag->name };
    if (old_name == new_name)
        return true;

    if (names_.contains(new_name))
        return false;

    names_.remove(old_name);
    names_.insert(new_name);

    tag->name = new_name;

    switch (tag->sync_state) {
    case SyncState::kCreating:
        TryInsert(tag);
        return true;
    case SyncState::kSynced:
        pending_updates_[tag->id].insert(kName, new_name);
        RestartTimer(tag->id, tag);
        break;
    case SyncState::kError:
    case SyncState::kUpdating:
    case SyncState::kDeleting:
        break;
    }

    return true;
}

bool Model::UpdateColor(Row* tag, const QString& new_color)
{
    if (!tag || tag->color == new_color)
        return false;

    tag->color = new_color;

    if (tag->sync_state == SyncState::kCreating && !tag->name.isEmpty()) {
        TryInsert(tag);
    } else if (tag->sync_state == SyncState::kSynced) {
        pending_updates_[tag->id].insert(kColor, new_color);
        RestartTimer(tag->id, tag);
    }

    return true;
}

void Model::RestartTimer(const QUuid& id, Row* tag)
{
    // Try to retrieve the existing timer
    QTimer* timer { pending_timers_.value(id, nullptr) };

    if (!timer) {
        // Create and configure a new timer if it does not exist
        timer = new QTimer { this };
        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout, this, [this, id, tag]() {
            auto* expired_timer { pending_timers_.take(id) };
            auto update { pending_updates_.take(id) };

            if (!update.isEmpty()) {
                update.insert(kVersion, tag->version);

                const QJsonObject message { JsonGen::TagUpdate(section_, id, update) };
                WebSocket::Instance()->SendMessage(WsKey::kTagUpdate, message);
            }

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

void Model::FlushCaches()
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

void Model::TryInsert(Row* tag)
{
    if (!tag || tag->sync_state != SyncState::kCreating)
        return;

    tag->sync_state = SyncState::kSynced;

    const QJsonObject message { JsonGen::TagInsert(section_, tag) };
    WebSocket::Instance()->SendMessage(WsKey::kTagInsert, message);

    emit SInsertLocalTag(section_, tag);
}
}