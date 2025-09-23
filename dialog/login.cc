#include "login.h"

#include <QMessageBox>

#include "component/signalblocker.h"
#include "global/logininfo.h"
#include "global/websocket.h"
#include "ui_login.h"
#include "utils/jsongen.h"

Login::Login(QSharedPointer<QSettings> local_settings, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Login)
    , local_settings_ { local_settings }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniDialog();
    connect(WebSocket::Instance(), &WebSocket::SLoginResult, this, &Login::RLoginResult, Qt::UniqueConnection);
    connect(WebSocket::Instance(), &WebSocket::SWorkspaceAccessPending, this, &Login::RWorkspaceAccessPending, Qt::UniqueConnection);
}

Login::~Login() { delete ui; }

void Login::RLoginResult(bool result)
{
    if (result) {
        SaveLoginConfig();
        this->close();
    } else {
        QMessageBox::critical(
            this, tr("Login Failed"), tr("Unable to log in. Please verify your email, password, workspace access, or check if your account has expired."));
    }
}

void Login::RWorkspaceAccessPending(const QString& email, const QString& workspace)
{
    const QString message
        = tr("Your access to workspace \"%1\" for email \"%2\" is pending approval. Please contact the administrator if needed.").arg(workspace, email);

    QMessageBox::information(this, tr("Workspace Access Pending"), message);
}

void Login::on_pushButtonLogin_clicked()
{
    LoginInfo& login_info { LoginInfo::Instance() };

    login_info.SetEmail(ui->lineEditEmail->text());
    login_info.SetPassword(ui->lineEditPassword->text());
    login_info.SetWorkspace(ui->lineEditWorkspace->text());

    WebSocket::Instance()->SendMessage(kLogin, JsonGen::Login());
}

void Login::IniDialog()
{
    LoginInfo& login_info { LoginInfo::Instance() };

    ui->lineEditEmail->setText(login_info.Email());
    ui->lineEditWorkspace->setText(login_info.Workspace());
    ui->lineEditPassword->setText(login_info.Password());
    ui->chkBoxSave->setChecked(login_info.IsSaved());
}

void Login::SaveLoginConfig()
{
    const bool is_saved { ui->chkBoxSave->isChecked() };

    LoginInfo& login_info { LoginInfo::Instance() };
    login_info.SetIsSaved(is_saved);

    if (!is_saved)
        login_info.SetPassword({});

    login_info.WriteConfig(local_settings_);
}
