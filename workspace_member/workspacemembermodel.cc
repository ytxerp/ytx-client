#include "workspacemembermodel.h"

#include "component/constantwebsocket.h"
#include "global/resourcepool.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"
#include "workspacememberenum.h"

WorkspaceMemberModel::WorkspaceMemberModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

QVariant WorkspaceMemberModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    static const QStringList kHeaders = {
        tr("Id"),
        tr("Email"),
        tr("Username"),
        tr("Name"),
        tr("WorkspaceRole"),
        tr("DatabaseRole"),
        tr("CreatedTime"),
    };

    if (section < 0 || section >= kHeaders.size())
        return {};

    return kHeaders.at(section);
}

QModelIndex WorkspaceMemberModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, member_list_.at(row));
}

QVariant WorkspaceMemberModel::data(const QModelIndex& index, int role) const
{
    // Basic validation to prevent out-of-bounds access
    if (!index.isValid() || index.row() >= member_list_.size()) {
        return {};
    }

    // Only respond to display and edit roles
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return {};
    }

    const auto* member { member_list_.at(index.row()) };
    const auto column { static_cast<WorkspaceMemberEnum>(index.column()) };

    switch (column) {
    case WorkspaceMemberEnum::kId:
        return member->id;
    case WorkspaceMemberEnum::kEmail:
        return member->email;
    case WorkspaceMemberEnum::kUsername:
        return member->username;
    case WorkspaceMemberEnum::kName:
        return member->name;
    case WorkspaceMemberEnum::kWorkspaceRole:
        return static_cast<int>(member->workspace_role);
    case WorkspaceMemberEnum::kDatabaseRole:
        return member->database_role;
    case WorkspaceMemberEnum::kCreatedTime:
        return member->created_time;
    }
}

bool WorkspaceMemberModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    // Basic validation for index and role
    if (!index.isValid() || role != Qt::EditRole) {
        return false;
    }

    // Return if value is the same as the current data
    if (data(index, role) == value) {
        return false;
    }

    // Get the member pointer using {} initialization
    auto* member { member_list_.at(index.row()) };
    const QUuid id { member->id };

    if (id.isNull()) {
        return false;
    }

    // Handle updates based on the column index
    // Assuming MemberColumn is your enum for WorkspaceMember columns
    switch (static_cast<WorkspaceMemberEnum>(index.column())) {
    case WorkspaceMemberEnum::kWorkspaceRole:
        member->workspace_role = static_cast<WorkspaceRole>(value.toInt());
        pending_updates_[id].insert(kWorkspaceRole, static_cast<int>(member->workspace_role));
        break;
    case WorkspaceMemberEnum::kDatabaseRole:
        member->database_role = value.toString();
        pending_updates_[id].insert(kDatabaseRole, member->database_role);
        break;
    case WorkspaceMemberEnum::kEmail:
    case WorkspaceMemberEnum::kUsername:
    case WorkspaceMemberEnum::kName:
    case WorkspaceMemberEnum::kId:
    case WorkspaceMemberEnum::kCreatedTime:
        // These columns are read-only in this model
        return false;
    }

    // Mark as pending update and restart the debounce timer
    RestartTimer(id);

    // Notify views about the data change
    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    return true;
}

void WorkspaceMemberModel::sort(int column, Qt::SortOrder order)
{
    // Convert integer column to the structured enum using brace initialization
    const WorkspaceMemberEnum e_column { static_cast<WorkspaceMemberEnum>(column) };

    // Define a lambda for comparison based on the selected column and sort order
    auto Compare = [order, e_column](const WorkspaceMember* lhs, const WorkspaceMember* rhs) -> bool {
        switch (e_column) {
        case WorkspaceMemberEnum::kEmail:
            return Utils::CompareMember(lhs, rhs, &WorkspaceMember::email, order);
        case WorkspaceMemberEnum::kUsername:
            return Utils::CompareMember(lhs, rhs, &WorkspaceMember::username, order);
        case WorkspaceMemberEnum::kName:
            return Utils::CompareMember(lhs, rhs, &WorkspaceMember::name, order);
        case WorkspaceMemberEnum::kWorkspaceRole:
            // Sorting by the underlying integer value of the enum
            return Utils::CompareMember(lhs, rhs, &WorkspaceMember::workspace_role, order);
        case WorkspaceMemberEnum::kDatabaseRole:
            return Utils::CompareMember(lhs, rhs, &WorkspaceMember::database_role, order);
        case WorkspaceMemberEnum::kCreatedTime:
            return Utils::CompareMember(lhs, rhs, &WorkspaceMember::created_time, order);
        case WorkspaceMemberEnum::kId:
            return false;
        }
    };

    // Notify the view that the layout is about to change
    emit layoutAboutToBeChanged();

    // Perform the sort on the underlying data list
    std::sort(member_list_.begin(), member_list_.end(), Compare);

    // Notify the view that the layout has been updated
    emit layoutChanged();
}

Qt::ItemFlags WorkspaceMemberModel::flags(const QModelIndex& index) const
{
    // Return early if index is invalid
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    // Initialize flags with the base class flags using brace initialization
    Qt::ItemFlags flags { QAbstractItemModel::flags(index) };

    // Convert column to enum for safe processing
    const auto column { static_cast<WorkspaceMemberEnum>(index.column()) };

    switch (column) {
    case WorkspaceMemberEnum::kWorkspaceRole:
    case WorkspaceMemberEnum::kDatabaseRole:
        // Enable editing for specific roles
        flags |= Qt::ItemIsEditable;
        break;

    case WorkspaceMemberEnum::kEmail:
    case WorkspaceMemberEnum::kUsername:
    case WorkspaceMemberEnum::kName:
    case WorkspaceMemberEnum::kCreatedTime:
    case WorkspaceMemberEnum::kId:
    default:
        // Disable editing for all other columns
        flags &= ~Qt::ItemIsEditable;
        break;
    }

    return flags;
}

bool WorkspaceMemberModel::removeRows(int row, int count, const QModelIndex& parent)
{
    // Basic validation
    if (count != 1 || row < 0 || row >= member_list_.size()) {
        return false;
    }

    // Capture the pointer before removal
    auto* member { member_list_.at(row) };
    const QUuid member_id { member->id };

    // IMPORTANT: Clean up the pending timer if it exists
    // If a timer is running for this member, it must be stopped and removed
    // to prevent it from firing and accessing a recycled object.
    if (pending_timers_.contains(member_id)) {
        auto* timer { pending_timers_.take(member_id) };
        timer->stop();
        timer->deleteLater();
    }

    pending_updates_.remove(member_id);

    // Notify views that rows are about to be removed
    beginRemoveRows(parent, row, row);
    member_list_.removeAt(row);
    endRemoveRows();

    // Send the WebSocket message using the captured ID
    const QJsonObject message { JsonGen::WorkspaceMemberDelete(member_id) };
    WebSocket::Instance()->SendMessage(WsKey::kWorkspaceMemberDelete, message);

    // Recycle the member back to the ResourcePool
    ResourcePool<WorkspaceMember>::Instance().Recycle(member);

    return true;
}

void WorkspaceMemberModel::ResetModel(const QJsonArray& array)
{
    if (array.isEmpty()) {
        qWarning() << "[WorkspaceMemberModel]" << "Received empty member array";
    }

    // Parse outside the reset block
    QList<WorkspaceMember*> new_list {};
    new_list.reserve(array.size());

    for (const auto& value : array) {
        if (!value.isObject()) {
            qWarning() << "[WorkspaceMemberModel]" << "Invalid member data, expected object:" << value;
            continue;
        }

        auto* member { ResourcePool<WorkspaceMember>::Instance().Allocate() };
        member->ReadJson(value.toObject());
        new_list.emplaceBack(member);
    }

    // Keep reset block as short as possible
    beginResetModel();
    ResourcePool<WorkspaceMember>::Instance().Recycle(member_list_);
    member_list_ = std::move(new_list);
    sort(std::to_underlying(WorkspaceMemberEnum::kWorkspaceRole), Qt::DescendingOrder);
    endResetModel();
}

void WorkspaceMemberModel::RestartTimer(const QUuid& id)
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
                const QJsonObject message { JsonGen::WorkspaceMemberUpdate(id, update) };
                WebSocket::Instance()->SendMessage(WsKey::kWorkspaceMemberUpdate, message);
            }

            if (expired_timer) {
                expired_timer->deleteLater();
            }
        });

        pending_timers_[id] = timer;
    }

    // Start or restart the timer
    timer->start(TimeConst::kAutoCloseMs);
}
