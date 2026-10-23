#include "workspacemodel.h"

#include <QJsonArray>

#include "component/constantwebsocket.h"
#include "global/resourcepool.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

namespace workspace {
Model::Model(const QStringList& header, QObject* parent)
    : QAbstractItemModel(parent)
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
    // Basic validation to prevent out-of-bounds access
    if (!index.isValid() || index.row() >= list_.size()) {
        return {};
    }

    // Only respond to display and edit roles
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return {};
    }

    const RowField column { index.column() };
    auto* row { static_cast<Row*>(index.internalPointer()) };

    switch (column) {
    case RowField::kEmail:
        return row->email;
    case RowField::kUsername:
        return row->username;
    case RowField::kName:
        return row->name;
    case RowField::kWorkspaceRole:
        return static_cast<int>(row->workspace_role);
    case RowField::kFinancePermissions:
        return row->finance_permissions;
    case RowField::kTaskPermissions:
        return row->task_permissions;
    case RowField::kInventoryPermissions:
        return row->inventory_permissions;
    case RowField::kPartnerPermissions:
        return row->partner_permissions;
    case RowField::kSalePermissions:
        return row->sale_permissions;
    case RowField::kPurchasePermissions:
        return row->purchase_permissions;
    case RowField::kCreatedTime:
        return row->created_time;
    case RowField::kPlaceholder:
        return QVariant();
    }
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role)
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
    auto* row { static_cast<Row*>(index.internalPointer()) };
    const QUuid id { row->id };

    if (id.isNull()) {
        return false;
    }

    auto& update { pending_updates_[id] };
    update.row = row;
    auto& changes { update.changes };
    const int raw { value.toInt() };

    // Handle updates based on the column index
    // Assuming MemberColumn is your enum for WorkspaceMember columns
    switch (static_cast<RowField>(index.column())) {
    case RowField::kWorkspaceRole:
        row->workspace_role = static_cast<workspace::Role>(raw);
        changes.insert(kWorkspaceRole, raw);
        break;
    case RowField::kFinancePermissions:
        row->finance_permissions = raw;
        changes.insert(kFinancePermissions, raw);
        break;
    case RowField::kTaskPermissions:
        row->task_permissions = raw;
        changes.insert(kTaskPermissions, raw);
        break;
    case RowField::kInventoryPermissions:
        row->inventory_permissions = raw;
        changes.insert(kInventoryPermissions, raw);
        break;
    case RowField::kPartnerPermissions:
        row->partner_permissions = raw;
        changes.insert(kPartnerPermissions, raw);
        break;
    case RowField::kSalePermissions:
        row->sale_permissions = raw;
        changes.insert(kSalePermissions, raw);
        break;
    case RowField::kPurchasePermissions:
        row->purchase_permissions = raw;
        changes.insert(kPurchasePermissions, raw);
        break;
    case RowField::kEmail:
    case RowField::kUsername:
    case RowField::kName:
    case RowField::kPlaceholder:
    case RowField::kCreatedTime:
        // These columns are read-only in this model
        return false;
    }

    // Mark as pending update and restart the debounce timer
    RestartTimer(id);

    // Notify views about the data change
    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    return true;
}

void Model::sort(int column, Qt::SortOrder order)
{
    // Convert integer column to the structured enum using brace initialization
    const RowField e_column { column };
    if (e_column == RowField::kPlaceholder)
        return;

    // Define a lambda for comparison based on the selected column and sort order
    auto Compare = [order, e_column](const Row* lhs, const Row* rhs) -> bool {
        switch (e_column) {
        case RowField::kEmail:
            return utils::CompareMember(lhs, rhs, &Row::email, order);
        case RowField::kUsername:
            return utils::CompareMember(lhs, rhs, &Row::username, order);
        case RowField::kName:
            return utils::CompareMember(lhs, rhs, &Row::name, order);
        case RowField::kWorkspaceRole:
            // Sorting by the underlying integer value of the enum
            return utils::CompareMember(lhs, rhs, &Row::workspace_role, order);
        case RowField::kFinancePermissions:
            return utils::CompareMember(lhs, rhs, &Row::finance_permissions, order);
        case RowField::kTaskPermissions:
            return utils::CompareMember(lhs, rhs, &Row::task_permissions, order);
        case RowField::kInventoryPermissions:
            return utils::CompareMember(lhs, rhs, &Row::inventory_permissions, order);
        case RowField::kPartnerPermissions:
            return utils::CompareMember(lhs, rhs, &Row::partner_permissions, order);
        case RowField::kSalePermissions:
            return utils::CompareMember(lhs, rhs, &Row::sale_permissions, order);
        case RowField::kPurchasePermissions:
            return utils::CompareMember(lhs, rhs, &Row::purchase_permissions, order);
        case RowField::kCreatedTime:
            return utils::CompareMember(lhs, rhs, &Row::created_time, order);
        case RowField::kPlaceholder:
            return false;
        }
    };

    // Notify the view that the layout is about to change
    emit layoutAboutToBeChanged();

    // Perform the sort on the underlying data list
    std::ranges::sort(list_, Compare);

    // Notify the view that the layout has been updated
    emit layoutChanged();
}

Qt::ItemFlags Model::flags(const QModelIndex& index) const
{
    // Return early if index is invalid
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    // Initialize flags with the base class flags using brace initialization
    Qt::ItemFlags flags { QAbstractItemModel::flags(index) };

    // Convert column to enum for safe processing
    const RowField column { index.column() };

    switch (column) {
    case RowField::kWorkspaceRole:
    case RowField::kFinancePermissions:
    case RowField::kTaskPermissions:
    case RowField::kInventoryPermissions:
    case RowField::kPartnerPermissions:
    case RowField::kSalePermissions:
    case RowField::kPurchasePermissions:
        // Enable editing for specific roles
        flags |= Qt::ItemIsEditable;
        break;

    case RowField::kEmail:
    case RowField::kUsername:
    case RowField::kName:
    case RowField::kCreatedTime:
    case RowField::kPlaceholder:
        // Disable editing for all other columns
        flags &= ~Qt::ItemIsEditable;
        break;
    }

    return flags;
}

bool Model::removeRows(int row, int count, const QModelIndex& parent)
{
    // Basic validation
    if (count != 1 || row < 0 || row >= list_.size()) {
        return false;
    }

    // Capture the pointer before removal
    auto* member { list_.at(row) };
    const QUuid member_id { member->id };

    // Remove pending update to prevent delayed flush after deletion
    // Stop its timer to avoid accessing recycled member.
    if (auto* timer = pending_updates_.take(member_id).timer; timer) {
        timer->stop();
        timer->deleteLater();
    }

    // Notify views that rows are about to be removed
    beginRemoveRows(parent, row, row);
    list_.removeAt(row);
    endRemoveRows();

    // Send the WebSocket message using the captured ID
    const QJsonObject message { JsonGen::WorkspaceMemberDelete(member_id) };
    WebSocket::Instance()->SendMessage(WsKey::kWorkspaceMemberDelete, message);

    // Recycle the member back to the ResourcePool
    ResourcePool<Row>::Instance().Recycle(member);

    return true;
}

void Model::Rebuild(const QJsonArray& array)
{
    if (array.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Received empty member array";
    }

    // Parse outside the reset block
    QList<Row*> new_list {};
    new_list.reserve(array.size());

    for (const auto& value : array) {
        Q_ASSERT(value.isObject());

        auto* member { ResourcePool<Row>::Instance().Allocate() };
        member->ReadJson(value.toObject());
        new_list.emplaceBack(member);
    }

    std::ranges::sort(new_list, [](const auto* lhs, const auto* rhs) { return utils::CompareMember(lhs, rhs, &Row::workspace_role, Qt::DescendingOrder); });

    // Keep reset block as short as possible
    beginResetModel();

    ResourcePool<Row>::Instance().Recycle(list_);
    list_ = std::move(new_list);

    endResetModel();
}

void Model::RestartTimer(QUuid id)
{
    auto*& timer { pending_updates_[id].timer };

    if (!timer) {
        timer = new QTimer { this };
        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout, this, [this, id]() { FlushTimer(id); });
    }

    timer->start(time_const::kAutoCloseMs);
}

void Model::FlushTimer(QUuid id)
{
    auto update { pending_updates_.take(id) };

    if (update.row && !update.changes.isEmpty()) {
        const int version { update.row->version };

        const QJsonObject message { JsonGen::WorkspaceMemberUpdate(id, update.changes, version) };
        WebSocket::Instance()->SendMessage(WsKey::kWorkspaceMemberUpdate, message);

        update.row->version = version + 1;
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

}
