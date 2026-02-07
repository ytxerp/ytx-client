#include <QtCore/qstandardpaths.h>

#include <QDir>
#include <QMessageBox>

#include "dialog/authdialog.h"
#include "global/logininfo.h"
#include "global/tablesstation.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils/mainwindowutils.h"
#include "websocket/websocket.h"

void MainWindow::on_actionReconnect_triggered()
{
    qInfo() << "[UI]" << "on_actionReconnect_triggered";

    Utils::SetConnectionStatus(connection_label_, ConnectionStatus::Connecting);
    WebSocket::Instance()->Connect();
}

void MainWindow::on_actionSignIn_triggered()
{
    qInfo() << "[UI]" << "on_actionSignIn_triggered";

    static QPointer<AuthDialog> dialog {};

    if (!dialog) {
        dialog = new AuthDialog(app_settings_, this);
        dialog->setModal(true);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
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

    SetAction(false);
    Utils::SetLoginStatus(login_label_, LoginStatus::LoggedOut);
    Utils::SetConnectionStatus(connection_label_, ConnectionStatus::Connecting);
}

void MainWindow::RConnectionRefused()
{
    ui->actionSignIn->setEnabled(false);
    ui->actionSignOut->setEnabled(false);
    ui->actionReconnect->setEnabled(true);

    Utils::SetConnectionStatus(connection_label_, ConnectionStatus::Disconnected);
    QMessageBox::warning(this, tr("Connection Refused"), tr("Unable to connect to the server. Please try again."));
}

void MainWindow::RConnectionSucceeded()
{
    ui->actionSignIn->setEnabled(true);
    ui->actionSignOut->setEnabled(false);
    ui->actionReconnect->setEnabled(true);

    on_actionSignIn_triggered();
    Utils::SetConnectionStatus(connection_label_, ConnectionStatus::Connected);
}

void MainWindow::RRemoteHostClosed()
{
    auto* msg_box { new QMessageBox(
        QMessageBox::Warning, tr("Remote Host Closed"), tr("The server has closed the connection. Please try reconnecting."), QMessageBox::Ok, this) };

    msg_box->setAttribute(Qt::WA_DeleteOnClose);
    msg_box->show();

    on_actionSignOut_triggered();
    Utils::SetConnectionStatus(connection_label_, ConnectionStatus::Disconnected);
}

void MainWindow::RLoginSucceeded(const QString& expire_date)
{
    {
        LoginInfo& login_info { LoginInfo::Instance() };
        UpdateAccountInfo(login_info.Email(), login_info.Workspace(), expire_date);

        if (!section_settings_) {
            const QString ini_file { QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator()
                + Utils::AccountIniFileName(login_info.Email(), login_info.Workspace()) + kDotSuffixINI };

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
        Utils::SetLoginStatus(login_label_, LoginStatus::LoggedIn);
    }
}

void MainWindow::RLoginFailed()
{
    ui->actionSignIn->setEnabled(true);
    ui->actionSignOut->setEnabled(false);
    ui->actionReconnect->setEnabled(true);
    Utils::SetLoginStatus(login_label_, LoginStatus::LoggedOut);
}

void MainWindow::RTreeSyncFinished()
{
    qDebug() << "RTreeSyncFinished";
    on_tabWidget_currentChanged(0);
}
