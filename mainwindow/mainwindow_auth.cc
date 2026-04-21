#include <QtCore/qstandardpaths.h>

#include <QDir>

#include "dialog/authdialog.h"
#include "global/logininfo.h"
#include "global/tablesstation.h"
#include "global/userprofile.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils/mainwindowutils.h"
#include "websocket/websocket.h"

void MainWindow::on_actionReconnect_triggered()
{
    qInfo() << "[UI]" << "on_actionReconnect_triggered";

    utils::SetConnectionStatus(connection_label_, ConnectionStatus::Connecting);
    WebSocket::Instance()->Connect();
}

void MainWindow::on_actionSignIn_triggered()
{
    qInfo() << "[UI]" << "on_actionSignIn_triggered";

    static QPointer<AuthDialog> dialog {};

    if (!dialog) {
        dialog = new AuthDialog(app_settings_, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setWindowModality(Qt::ApplicationModal);
    }

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void MainWindow::on_actionSignOut_triggered()
{
    qInfo() << "[UI]" << "on_actionSignOut_triggered";

    ResetMainwindow();

    WebSocket::Instance()->Reset();
    TableSStation::Instance()->Reset();
    UserProfile::Instance().Reset();

    SetAction(false);
    utils::SetLoginStatus(login_label_, LoginStatus::LoggedOut);
    utils::SetConnectionStatus(connection_label_, ConnectionStatus::Connecting);
}

void MainWindow::RDenyConnection()
{
    ui->actionSignIn->setEnabled(false);
    ui->actionSignOut->setEnabled(false);
    ui->actionReconnect->setEnabled(true);

    utils::SetConnectionStatus(connection_label_, ConnectionStatus::Disconnected);
    utils::ShowNotification(QMessageBox::Warning, tr("Connection Refused"), tr("Unable to connect to the server. Please try again."), TimeConst::kAutoCloseMs);
}

void MainWindow::RAllowConnection()
{
    ui->actionSignIn->setEnabled(true);
    ui->actionSignOut->setEnabled(false);
    ui->actionReconnect->setEnabled(false);

    on_actionSignIn_triggered();
    utils::SetConnectionStatus(connection_label_, ConnectionStatus::Connected);
}

void MainWindow::RRemoteHostClosed()
{
    utils::ShowNotification(
        QMessageBox::Warning, tr("Remote Host Closed"), tr("The server has closed the connection. Please try reconnecting."), TimeConst::kAutoCloseMs);

    on_actionSignOut_triggered();
    utils::SetConnectionStatus(connection_label_, ConnectionStatus::Disconnected);
}

void MainWindow::RAllowLogin(const QString& name, const QString& expire_date)
{
    {
        LoginInfo& login_info { LoginInfo::Instance() };
        UpdateAccountInfo(login_info.Workspace(), name, expire_date);

        const bool is_admin { UserProfile::Instance().GetWorkspaceRole() >= WorkspaceRole::kAdmin };
        ui->actionWorkspaceMember->setVisible(is_admin);
        ui->actionAuditLog->setVisible(is_admin);

        if (!section_settings_) {
            const QString ini_file { QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator()
                + utils::AccountIniFileName(login_info.Email(), login_info.Workspace()) + kDotSuffixINI };

            section_settings_ = QSharedPointer<QSettings>::create(ini_file, QSettings::IniFormat);
        }
    }

    {
        ReadSectionConfig(sc_f_.section_config, kFinance);
        ReadSectionConfig(sc_i_.section_config, kInventory);
        ReadSectionConfig(sc_t_.section_config, kTask);
        ReadSectionConfig(sc_p_.section_config, kPartner);
        ReadSectionConfig(sc_sale_.section_config, kSale);
        ReadSectionConfig(sc_purchase_.section_config, kPurchase);
    }

    {
        TreeDelegateF(sc_f_.tree_view, sc_f_.info, sc_f_.section_config);
        TreeDelegateI(sc_i_.tree_view, sc_i_.info, sc_i_.section_config);
        TreeDelegateP(sc_p_.tree_view, sc_p_.info, sc_p_.section_config);
        TreeDelegateT(sc_t_.tree_view, sc_t_.info, sc_t_.section_config);
        TreeDelegateO(sc_sale_.tree_view, sc_sale_.info, sc_sale_.section_config);
        TreeDelegateO(sc_purchase_.tree_view, sc_purchase_.info, sc_purchase_.section_config);
    }

    {
        SetTreeHeader(sc_f_.tree_view, Section::kFinance);
        SetTreeHeader(sc_i_.tree_view, Section::kInventory);
        SetTreeHeader(sc_p_.tree_view, Section::kPartner);
        SetTreeHeader(sc_t_.tree_view, Section::kTask);
        SetTreeHeader(sc_sale_.tree_view, Section::kSale);
        SetTreeHeader(sc_purchase_.tree_view, Section::kPurchase);
    }

    {
        ui->actionSignIn->setEnabled(false);
        ui->actionSignOut->setEnabled(true);
        ui->actionReconnect->setEnabled(false);
    }

    {
        utils::SetLoginStatus(login_label_, LoginStatus::LoggedIn);
    }
}

void MainWindow::RDenyLogin()
{
    ui->actionSignIn->setEnabled(true);
    ui->actionSignOut->setEnabled(false);
    ui->actionReconnect->setEnabled(true);
    utils::SetLoginStatus(login_label_, LoginStatus::LoggedOut);
}

void MainWindow::RFinishTreeSync()
{
    qDebug() << "RTreeSyncFinished";
    tabWidget_currentChanged();
}
