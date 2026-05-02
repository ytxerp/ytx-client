#include "component/constantwebsocket.h"
#include "dialog/userprofiledialog.h"
#include "enum/authenum.h"
#include "global/logininfo.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"
#include "workspace_member/workspacememberdialog.h"
#include "workspace_member/workspacememberenum.h"

void MainWindow::on_actionProfile_triggered()
{
    qInfo() << Q_FUNC_INFO;

    static QPointer<UserProfileDialog> dialog {};

    if (!dialog) {
        dialog = new UserProfileDialog(this);
        utils::ManageDialog(sc_->widget_hash, dialog);
    }

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void MainWindow::RAccountName(const QString& name) { ui->actionEmail->setText(tr("Name") + ": " + name); }

void MainWindow::RAccountUsername(const QJsonObject& obj)
{
    const auto code { static_cast<AccountUsernameOutcome>(obj[kCode].toInt()) };

    QString message {};
    switch (code) {
    case AccountUsernameOutcome::kSuccess:
        return;
    case AccountUsernameOutcome::kInvalidFormat:
        message = tr("Username format is invalid. Use 3-32 lowercase letters, digits, or underscores. No consecutive or trailing underscores.");
        break;
    case AccountUsernameOutcome::kAlreadyExists:
        message = tr("This username is already taken. Please choose another one.");
        break;
    }

    utils::ShowNotification(QMessageBox::Warning, tr("Update Failed"), message, TimeConst::kAutoCloseMs);
}

void MainWindow::InitWorkspaceInfo()
{
    workspace_info_.role_list = {
        { static_cast<int>(WorkspaceRole::kGuest), tr("Guest") },
        { static_cast<int>(WorkspaceRole::kMember), tr("Member") },
        { static_cast<int>(WorkspaceRole::kAdmin), tr("Admin") },
        { static_cast<int>(WorkspaceRole::kOwner), tr("Owner") },
    };

    workspace_info_.role_hash = QHash<int, QString>(workspace_info_.role_list.cbegin(), workspace_info_.role_list.cend());

    workspace_info_.database_role_list = {
        { PermissionBits::FINANCE_READONLY, tr("Finance R") },
        { PermissionBits::FINANCE_READWRITE, tr("Finance W") },
        { PermissionBits::TASK_READONLY, tr("Task R") },
        { PermissionBits::TASK_READWRITE, tr("Task W") },
        { PermissionBits::INVENTORY_READONLY, tr("Inventory R") },
        { PermissionBits::INVENTORY_READWRITE, tr("Inventory W") },
        { PermissionBits::PARTNER_READONLY, tr("Partner R") },
        { PermissionBits::PARTNER_READWRITE, tr("Partner W") },
        { PermissionBits::SALE_READONLY, tr("Sale R") },
        { PermissionBits::SALE_READWRITE, tr("Sale W") },
        { PermissionBits::PURCHASE_READONLY, tr("Purchase R") },
        { PermissionBits::PURCHASE_READWRITE, tr("Purchase W") },
    };

    workspace_info_.header = {
        tr("Id"),
        tr("Version"),
        tr("Email"),
        tr("Username"),
        tr("Name"),
        tr("WorkspaceRole"),
        tr("DatabaseRole"),
        tr("CreatedTime"),
    };
}

void MainWindow::on_actionWorkspaceMember_triggered()
{
    qInfo() << Q_FUNC_INFO;

    static QPointer<WorkspaceMemberDialog> dialog {};

    if (!dialog) {
        dialog = new WorkspaceMemberDialog(workspace_info_.header, this);

        const auto widget_id { utils::ManageDialog(widget_hash_, dialog) };
        const auto message { JsonGen::WorkspaceMemberAck(widget_id, LoginInfo::Instance().Workspace()) };

        WebSocket::Instance()->SendMessage(WsKey::kWorkspaceMemberAck, message);

        auto* view { dialog->View() };
        InitTableView(view, std::to_underlying(WorkspaceMemberEnum::kId), std::to_underlying(WorkspaceMemberEnum::kVersion),
            std::to_underlying(WorkspaceMemberEnum::kDatabaseRole));
        DelegateWorkspaceMember(view);
    }

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void MainWindow::RWorkspaceMemberAck(const QUuid& widget_id, const QJsonArray& array)
{
    auto widget { widget_hash_.value(widget_id).widget };
    if (!widget)
        return;

    auto* ptr { widget.data() };
    Q_ASSERT(qobject_cast<WorkspaceMemberDialog*>(ptr));

    auto* d_widget { static_cast<WorkspaceMemberDialog*>(ptr) };

    auto* model { d_widget->Model() };
    model->ResetModel(array);
}

void MainWindow::RAccountRoleUpdate()
{
    utils::ShowNotification(QMessageBox::Information, tr("Role Updated"),
        tr("Your account role has been updated. Please restart the application to enable new permissions."), TimeConst::kAutoCloseMs);
}