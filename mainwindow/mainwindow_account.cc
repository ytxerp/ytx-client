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

void MainWindow::RAccountName(const QString& name) { ui->actionName->setText(tr("Name", "Person") + ": " + name); }

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
    case AccountUsernameOutcome::kUserNotFound:
        message = tr("User account was not found.");
        break;
    }

    utils::ShowNotification(QMessageBox::Warning, tr("Update Failed"), message, time_const::kAutoCloseMs);
}

void MainWindow::InitHeader()
{
    header_info_.workspace = {
        tr("ID"),
        tr("Version"),
        tr("Email"),
        tr("Username"),
        tr("Name", "Person"),
        tr("Workspace Role"),
        tr("Database Role"),
        tr("Created Time"),
    };

    header_info_.balance_sheet = {
        tr("Name"),
        tr("ID"),
        tr("Code"),
        tr("Description"),
        tr("Direction Rule"),
        tr("Kind"),
        tr("Local Total"),
    };

    header_info_.income_statement = {
        tr("Name"),
        tr("ID"),
        tr("Code"),
        tr("Description"),
        tr("Direction Rule"),
        tr("Kind"),
        tr("Local Total"),
    };

    header_info_.inventory_heat = {
        tr("Name"),
        tr("Placeholder"),
        tr("Order Count"),
        tr("Partner Count"),
        tr("Active Months"),
        tr("Active Days"),
        tr("Total Quantity"),
        tr("Heat Score"),
    };

    header_info_.partner_heat = {
        tr("Name"),
        tr("Placeholder"),
        tr("Order Count"),
        tr("Inventory Diversity"),
        tr("Active Months"),
        tr("Active Days"),
        tr("Total Quantity"),
        tr("Heat Score"),
    };

    header_info_.tag = {
        tr("ID"),
        tr("Version"),
        tr("Name"),
        tr("Color"),
    };

    // Statement
    header_info_.statement = {
        tr("Partner"),
        tr("Previous Balance"),
        tr("Current Count"),
        tr("Current Measure"),
        tr("Current Amount"),
        tr("Description"),
        tr("Current Settlement"),
        tr("Current Balance"),
    };

    header_info_.statement_node = {
        tr("Issued Time"),
        tr("Code"),
        tr("Count"),
        tr("Measure"),
        tr("Amount"),
        tr("Description"),
        tr("Status"),
        tr("Employee"),
        tr("Settlement"),
    };

    header_info_.statement_entry = {
        tr("Issued Time"),
        tr("Code"),
        tr("Internal SKU"),
        tr("Count"),
        tr("Measure"),
        tr("Unit Price"),
        tr("Amount"),
        tr("Description"),
        tr("Status"),
        tr("External SKU"),
    };

    // Settlement
    header_info_.settlement = {
        tr("ID"),
        tr("Version"),
        tr("Partner"),
        tr("Issued Time"),
        tr("Description"),
        tr("Status"),
        tr("Amount"),
    };

    header_info_.settlement_item = {
        tr("ID"),
        tr("Issued Time"),
        tr("Amount"),
        tr("Status"),
        tr("Description"),
        tr("Employee"),
    };
}

void MainWindow::on_actionWorkspaceMember_triggered()
{
    qInfo() << Q_FUNC_INFO;

    static QPointer<WorkspaceMemberDialog> dialog {};

    if (!dialog) {
        dialog = new WorkspaceMemberDialog(header_info_.workspace, this);

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
        tr("Your account role has been updated. Please restart the application to enable new permissions."), time_const::kAutoCloseMs);
}