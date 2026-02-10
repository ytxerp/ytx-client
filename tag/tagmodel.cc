#include "tagmodel.h"

#include <QJsonObject>

#include "component/constant.h"
#include "enum/tagenum.h"
#include "global/resourcepool.h"
#include "utils/compareutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TagModel::TagModel(Section section, const QHash<QUuid, Tag*>& tag_hash, QObject* parent)
    : QAbstractItemModel(parent)
    , section_ { section }
    , tag_hash_ { tag_hash }
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

    switch (static_cast<TagEnum>(section)) {
    case TagEnum::kId:
        return tr("Id");
    case TagEnum::kName:
        return tr("Name");
    case TagEnum::kColor:
        return tr("Color");
    case TagEnum::kVersion:
        return tr("Version");
    }
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
    case TagEnum::kVersion:
        return tag->version;
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
    case TagEnum::kVersion:
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
        case TagEnum::kVersion:
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
    case TagEnum::kVersion:
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
    tag->state = Tag::State::NEW;

    beginInsertRows(parent, row, row);

    tag_list_.insert(row, tag);
    tag_hash_.insert(tag->id, tag);

    endInsertRows();

    return true;
}

bool TagModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (count != 1 || row < 0 || row >= tag_list_.size())
        return false;

    Tag* tag = tag_list_.at(row);

    beginRemoveRows(parent, row, row);

    tag_list_.removeAt(row);
    tag_hash_.remove(tag->id);
    names_.remove(tag->name);
    pending_updates_.remove(tag->id);

    endRemoveRows();

    if (tag->state == Tag::State::SYNCED) {
        const QJsonObject message { JsonGen::TagDelete(section_, tag->id) };
        WebSocket::Instance()->SendMessage(kTagDelete, message);
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

    if (tag->state == Tag::State::NEW && !tag->color.isEmpty()) {
        TryInsert(tag);
    } else if (tag->state == Tag::State::SYNCED) {
        pending_updates_.insert(tag->id);
        RestartTimer(tag->id);
    }

    return true;
}

bool TagModel::UpdateColor(Tag* tag, const QString& new_color)
{
    if (!tag || tag->color == new_color)
        return false;

    tag->color = new_color;

    if (tag->state == Tag::State::NEW && !tag->name.isEmpty()) {
        TryInsert(tag);
    } else if (tag->state == Tag::State::SYNCED) {
        pending_updates_.insert(tag->id);
        RestartTimer(tag->id);
    }

    return true;
}

void TagModel::RestartTimer(const QUuid& id)
{
    if (pending_timers_.contains(id)) {
        pending_timers_[id]->stop();
    } else {
        auto* timer = new QTimer(this);
        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout, this, [this, id]() {
            auto* expired_timer { pending_timers_.take(id) };

            // auto it = std::find_if(tag_list_.begin(), tag_list_.end(), [id](const Tag* tag) { return tag && tag->id == id; });

            // if (it != tag_list_.end()) {
            //     const auto* tag = *it;
            //     qDebug() << "update tag" << tag->name;

            //     const QJsonObject message = JsonGen::TagUpdate(section_, tag);
            //     WebSocket::Instance()->SendMessage(kTagUpdate, message);

            //     pending_updates_.remove(tag->id);
            // } else {
            //     qDebug() << "tag not found in tag_list_, id:" << id;
            // }

            if (auto it = tag_hash_.find(id); it != tag_hash_.end()) {
                const auto* tag { it.value() };
                const QJsonObject message { JsonGen::TagUpdate(section_, tag) };
                WebSocket::Instance()->SendMessage(kTagUpdate, message);
                pending_updates_.remove(tag->id);
            }

            expired_timer->deleteLater();
        });
        pending_timers_[id] = timer;
    }

    pending_timers_[id]->start(kThreeThousand);
}

void TagModel::TryInsert(Tag* tag)
{
    if (!tag || tag->state != Tag::State::NEW)
        return;

    tag->state = Tag::State::INSERTING;

    const QJsonObject message { JsonGen::TagInsert(section_, tag) };
    WebSocket::Instance()->SendMessage(kTagInsert, message);

    emit SInsertingTag(tag);
}
