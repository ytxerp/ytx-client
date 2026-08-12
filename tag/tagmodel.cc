#include "tagmodel.h"

#include <QJsonObject>
#include <QRandomGenerator>

#include "component/constantwebsocket.h"
#include "global/resourcepool.h"
#include "tag/tagenum.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

namespace tag {

Model::Model(Section section, const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
    , section_ { section }
    , header_ { header }
{
}

Model::~Model() { FlushTimers(); }

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

    auto* tag { static_cast<Row*>(index.internalPointer()) };
    if (!tag)
        return false;

    pending_updates_[tag->id].tag = tag;

    const RowField column { index.column() };

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
    const int version { tag->version };

    // Remove pending update to prevent delayed flush after deletion
    // Stop its timer to avoid accessing recycled member.
    if (auto* timer = pending_updates_.take(tag_id).timer; timer) {
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
        const QJsonObject message { JsonGen::TagDelete(section_, tag_id, version) };
        WebSocket::Instance()->SendMessage(WsKey::kTagDelete, message);
    } else {
        ResourcePool<Row>::Instance().Recycle(tag);
    }

    return true;
}

void Model::Rebuild(const QHash<QUuid, Row*>& tag_hash)
{
    QList<Row*> new_list {};
    QSet<QString> new_names {};

    new_list.reserve(tag_hash.size());
    new_names.reserve(tag_hash.size());

    for (auto it = tag_hash.cbegin(); it != tag_hash.cend(); ++it) {
        Row* tag { it.value() };

        if (!tag) {
            continue;
        }

        new_list.append(tag);
        new_names.insert(tag->name);
    }

    std::ranges::sort(new_list, [](const Row* lhs, const Row* rhs) { return utils::CompareString(lhs->name, rhs->name, Qt::AscendingOrder); });

    beginResetModel();

    list_ = std::move(new_list);
    names_ = std::move(new_names);

    endResetModel();
}

bool Model::UpdateName(Row* tag, const QString& name)
{
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

    if (tag->sync_state == SyncState::kCreating) {
        TryInsert(tag);
        return true;
    }

    pending_updates_[tag->id].changes.insert(kName, new_name);
    RestartTimer(tag->id);

    return true;
}

bool Model::UpdateColor(Row* tag, const QString& new_color)
{
    tag->color = new_color;

    if (tag->sync_state == SyncState::kCreating) {
        return true;
    }

    pending_updates_[tag->id].changes.insert(kColor, new_color);
    RestartTimer(tag->id);

    return true;
}

void Model::RestartTimer(const QUuid& id)
{
    auto*& timer { pending_updates_[id].timer };

    if (!timer) {
        timer = new QTimer { this };
        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout, this, [this, id]() { FlushTimer(id); });
    }

    timer->start(time_const::kAutoCloseMs);
}

void Model::FlushTimer(const QUuid& id)
{
    auto update { pending_updates_.take(id) };

    if (update.tag && !update.changes.isEmpty()) {
        const QJsonObject message { JsonGen::TagUpdate(section_, id, update.changes, update.tag->version) };
        WebSocket::Instance()->SendMessage(WsKey::kTagUpdate, message);
    }

    if (update.timer) {
        update.timer->stop();
        update.timer->deleteLater();
    }
}

void Model::FlushTimers()
{
    const auto ids { pending_updates_.keys() };

    for (const auto& id : ids) {
        FlushTimer(id);
    }
}

void Model::TryInsert(Row* tag)
{
    if (tag->sync_state != SyncState::kCreating)
        return;

    tag->sync_state = SyncState::kSynced;
    tag->version = 1;

    const QJsonObject message { JsonGen::TagInsert(section_, tag) };
    WebSocket::Instance()->SendMessage(WsKey::kTagInsert, message);

    emit SInsertLocalTag(section_, tag);
}
}