#include <QMessageBox>

#include "dialog/authdialog.h"
#include "global/tablesstation.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "websocket/websocket.h"

void MainWindow::on_actionReconnect_triggered() { WebSocket::Instance()->Connect(); }

void MainWindow::on_actionLogin_triggered()
{
    static AuthDialog* dialog = nullptr;

    if (!dialog) {
        dialog = new AuthDialog(app_settings_, this);
        dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(dialog, &QDialog::destroyed, this, [=]() { dialog = nullptr; });
    }

    dialog->setModal(true);
    dialog->show();
    dialog->activateWindow();
}

void MainWindow::on_actionLogout_triggered()
{
    ClearMainwindow();
    ClearAccountInfo();

    WebSocket::Instance()->Close();
    TableSStation::Instance()->Clear();

    SetAction(false);

    ui->actionReconnect->setEnabled(true);
    ui->actionLogin->setEnabled(false);
    ui->actionLogout->setEnabled(false);
}

void MainWindow::ClearAccountInfo()
{
    ui->actionEmail->setText(tr("Email"));
    ui->actionWorkspace->setText(tr("Workspace"));
    ui->actionExpireDate->setText(tr("Expire Date"));
}

void MainWindow::RLoginResult(bool result)
{
    ui->actionLogin->setEnabled(!result);
    ui->actionLogout->setEnabled(result);
}

void MainWindow::RConnectionRefused() { QMessageBox::warning(this, tr("Connection Refused"), tr("Unable to connect to the server. Please try again.")); }

void MainWindow::RRemoteHostClosed()
{
    auto* msg_box { new QMessageBox(
        QMessageBox::Warning, tr("Remote Host Closed"), tr("The server has closed the connection. Please try reconnecting."), QMessageBox::Ok, this) };

    msg_box->setAttribute(Qt::WA_DeleteOnClose);
    msg_box->show();

    on_actionLogout_triggered();
}

void MainWindow::RConnectionAccepted()
{
    ui->actionLogin->setEnabled(true);
    ui->actionReconnect->setEnabled(false);
    ui->actionLogout->setEnabled(false);

    on_actionLogin_triggered();
}
