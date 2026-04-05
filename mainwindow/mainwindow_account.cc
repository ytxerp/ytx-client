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
    qInfo() << "[UI]" << "on_actionProfile_triggered";

    static QPointer<UserProfileDialog> dialog {};

    if (!dialog) {
        dialog = new UserProfileDialog(this);
        Utils::ManageDialog(sc_->widget_hash, dialog);
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

    Utils::ShowNotification(QMessageBox::Warning, tr("Update Failed"), message, TimeConst::kAutoCloseMs);
}

void MainWindow::on_actionWorkspaceMember_triggered()
{
    qInfo() << "[UI]" << "on_actionWorkspaceMember_triggered";

    auto* dialog = new WorkspaceMemberDialog(this);
    const auto widget_id { Utils::ManageDialog(widget_hash_, dialog) };

    const auto message { JsonGen::WorkspaceMemberAck(widget_id, LoginInfo::Instance().Workspace()) };
    WebSocket::Instance()->SendMessage(WsKey::kWorkspaceMemberAck, message);

    auto* view { dialog->View() };
    InitTableView(view, std::to_underlying(WorkspaceMemberEnum::kId), std::to_underlying(WorkspaceMemberEnum::kName));
    DelegateWorkspaceMember(view);

    dialog->show();
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