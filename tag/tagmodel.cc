#include "tagmodel.h"

#include <QJsonObject>

#include "component/constant.h"
#include "enum/tagenum.h"
#include "global/resourcepool.h"
#include "utils/compareutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TagModel::TagModel(Section section, const QHash<QUuid, Tag>& raw_tags, QObject* parent)
    : QAbstractItemModel(parent)
    , raw_tags_ { raw_tags }
    , section_ { section }
{
    for (auto it = raw_tags.cbegin(); it != raw_tags.cend(); ++it) {
        const QString& name { it.value().name };

        auto* tag { ResourcePool<Tag>::Instance().Allocate() };
        tag->name = name;
        tag->color = it.value().color;
        tag->id = it.key();

        tags_.append(tag);
        names_.insert(name);
    }
}

TagModel::~TagModel()
{
    ResourcePool<Tag>::Instance().Recycle(tags_);
    tags_.clear();
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

    return createIndex(row, column, tags_.at(row));
}

QVariant TagModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto* tag { tags_.at(index.row()) };
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

    auto* tag { tags_.at(index.row()) };

    switch (static_cast<TagEnum>(index.column())) {
    case TagEnum::kName:
        UpdateName(tag, value.toString(), pending_updates_[tag->id]);
        break;
    case TagEnum::kColor:
        UpdateColor(tag, value.toString(), pending_updates_[tag->id]);
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
    std::sort(tags_.begin(), tags_.end(), Compare);
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
    if (count != 1 || row < 0 || row > tags_.size())
        return false;

    auto* tag { ResourcePool<Tag>::Instance().Allocate() };
    tag->is_new = true;

    beginInsertRows(parent, row, row);
    tags_.insert(row, tag);
    endInsertRows();

    return true;
}

bool TagModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (count != 1 || row < 0 || row >= tags_.size())
        return false;

    beginRemoveRows(parent, row, row);
    auto* tag = tags_.takeAt(row);
    endRemoveRows();

    if (!tag->is_new) {
        QJsonObject message {};
        message.insert(kSection, std::to_underlying(section_));
        message.insert(kId, tag->id.toString(QUuid::WithoutBraces));
        message.insert(kSessionId, QString());

        WebSocket::Instance()->SendMessage(kTagDelete, message);
    }

    names_.remove(tag->name);
    ResourcePool<Tag>::Instance().Recycle(tag);

    return true;
}

bool TagModel::UpdateName(Tag* tag, const QString& new_name, QJsonObject& update)
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

    if (tag->is_new) {
        if (!tag->color.isEmpty()) {
            WebSocket::Instance()->SendMessage(kTagInsert, tag->WriteJson());
            tag->is_new = false;
        }
    } else {
        update.insert(kName, new_name);
        names_.remove(old_name);

        RestartTimer(tag->id);
    }

    return true;
}

bool TagModel::UpdateColor(Tag* tag, const QString& new_color, QJsonObject& update)
{
    if (!tag || tag->color == new_color)
        return false;

    tag->color = new_color;

    if (tag->is_new) {
        if (!tag->name.isEmpty()) {
            WebSocket::Instance()->SendMessage(kTagInsert, tag->WriteJson());
            tag->is_new = false;
        }
    } else {
        update.insert(kColor, new_color);
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

            if (auto it = pending_updates_.find(id); it != pending_updates_.end()) {
                const auto& update { it.value() };

                if (!update.isEmpty()) {
                    const QJsonObject message { JsonGen::TagUpdate(section_, id, update, raw_tags_.value(id).version) };
                    WebSocket::Instance()->SendMessage(kTagUpdate, message);
                }

                pending_updates_.erase(it);
            }

            expired_timer->deleteLater();
        });
        pending_timers_[id] = timer;
    }

    pending_timers_[id]->start(kThreeThousand);
}
