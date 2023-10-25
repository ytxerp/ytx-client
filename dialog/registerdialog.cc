#include "registerdialog.h"

#include <QMessageBox>
#include <QRegularExpression>

#include "global/websocket.h"
#include "ui_registerdialog.h"
#include "utils/jsongen.h"

RegisterDialog::RegisterDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    connect(&WebSocket::Instance(), &WebSocket::SRegisterResult, this, &RegisterDialog::RRegisterResult);
}

RegisterDialog::~RegisterDialog() { delete ui; }

void RegisterDialog::RRegisterResult(bool result)
{
    if (result) {
        this->close();
    } else {
        QMessageBox::critical(this, tr("Registration Failed"), tr("Unable to register. Please contact the administrator for details."));
    }
}

void RegisterDialog::on_pushButtonSubmit_clicked()
{
    const QString email = ui->lineEditEmail->text().trimmed();
    const QString password = ui->lineEditPassword->text();
    const QString confirm_pwd = ui->lineEditConfirmPwd->text();

    if (email.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Information"), tr("Email cannot be empty"));
        return;
    }

    static QRegularExpression email_regex(R"(^[^\s@]+@[^\s@]+\.[^\s@]+$)");
    if (!email_regex.match(email).hasMatch()) {
        QMessageBox::warning(this, tr("Invalid Information"), tr("Invalid email format"));
        return;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Information"), tr("Password cannot be empty"));
        return;
    }

    if (password != confirm_pwd) {
        QMessageBox::warning(this, tr("Invalid Information"), tr("Passwords do not match"));
        return;
    }

    WebSocket::Instance().SendMessage(kRegister, JsonGen::Register(email, password));
}
