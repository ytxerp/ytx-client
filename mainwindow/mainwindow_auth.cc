#include <QtCore/qstandardpaths.h>

#include <QDir>

#include "dialog/authdialog.h"
#include "global/logininfo.h"
#include "global/partner_inventory_registry.h"
#include "global/tablesstation.h"
#include "global/userprofile.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils/mainwindowutils.h"
#include "websocket/websocket.h"

void MainWindow::on_actionReconnect_triggered()
{
    qInfo() << Q_FUNC_INFO;

    utils::SetConnectionStatus(connection_label_, ConnectionState::Connecting);
    WebSocket::Instance()->Connect();
}

void MainWindow::on_actionSignIn_triggered()
{
    qInfo() << Q_FUNC_INFO;

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
    qInfo() << Q_FUNC_INFO;

    ResetMainwindow();

    WebSocket::Instance()->Reset();
    TableSStation::Instance()->Reset();
    UserProfile::Instance().Reset();
    PartnerInventoryRegistry::Instance().Reset();

    DisableActions();
    SetLoginActions(false);
    UpdateSessionActions(ConnectionState::Connecting, LoginState::LoggedOut);

    utils::SetLoginStatus(login_label_, LoginState::LoggedOut);
    utils::SetConnectionStatus(connection_label_, ConnectionState::Connecting);
}

void MainWindow::RDenyConnection()
{
    UpdateSessionActions(ConnectionState::Disconnected, LoginState::LoggedOut);

    utils::SetConnectionStatus(connection_label_, ConnectionState::Disconnected);
    utils::ShowMessage(QMessageBox::Warning, tr("Connection Refused"), tr("Unable to connect to the server. Please try again."), time_const::kAutoCloseMs);
}

void MainWindow::RAllowConnection()
{
    UpdateSessionActions(ConnectionState::Connected, LoginState::LoggedOut);

    utils::SetConnectionStatus(connection_label_, ConnectionState::Connected);
    on_actionSignIn_triggered();
}

void MainWindow::UpdateSessionActions(ConnectionState connection, LoginState login) const
{
    const bool connected { connection == ConnectionState::Connected };
    const bool logged_in { login == LoginState::LoggedIn };

    ui->actionSignIn->setEnabled(connected && !logged_in);
    ui->actionSignOut->setEnabled(connected && logged_in);
    ui->actionReconnect->setEnabled(connection == ConnectionState::Disconnected);
}

void MainWindow::RRemoteHostClosed()
{
    utils::ShowMessage(
        QMessageBox::Warning, tr("Remote Host Closed"), tr("The server has closed the connection. Please try reconnecting."), time_const::kAutoCloseMs);

    on_actionSignOut_triggered();
    utils::SetConnectionStatus(connection_label_, ConnectionState::Disconnected);
    ui->actionReconnect->setEnabled(true);
}

void MainWindow::RAllowLogin(const QString& name, const QString& expire_date)
{
    {
        LoginInfo& login_info { LoginInfo::Instance() };
        UpdateAccountInfo(login_info.Workspace(), name, expire_date);

        if (!section_settings_) {
            const QDir config_dir { QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) };
            const QString file_name { utils::AccountIniFileName(login_info.Email(), login_info.Workspace()) + kDotSuffixINI };

            section_settings_ = QSharedPointer<QSettings>::create(config_dir.filePath(file_name), QSettings::IniFormat);
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
        UpdateSessionActions(ConnectionState::Connected, LoginState::LoggedIn);
    }

    {
        utils::SetLoginStatus(login_label_, LoginState::LoggedIn);
        SetLoginActions(true);
    }
}

void MainWindow::RDenyLogin()
{
    UpdateSessionActions(ConnectionState::Connected, LoginState::LoggedOut);
    utils::SetLoginStatus(login_label_, LoginState::LoggedOut);
}

void MainWindow::RFinishTreeSync()
{
    qDebug() << Q_FUNC_INFO;

    sc_p_.tree_model->SyncFilterModels();
    sc_i_.tree_model->SyncFilterModels();

    tabWidget_currentChanged();
}
