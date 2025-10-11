#include "authdialog.h"

#include <QMessageBox>

#include "component/signalblocker.h"
#include "global/logininfo.h"
#include "ui_authdialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

AuthDialog::AuthDialog(QSharedPointer<QSettings> local_settings, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AuthDialog)
    , local_settings_ { local_settings }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    setWindowTitle(tr("Authentication"));

    ui->labelSignUp->setFocusPolicy(Qt::NoFocus);
    ui->labelLogin->setFocusPolicy(Qt::NoFocus);
    ui->labelSignUp->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    ui->labelLogin->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    action_password_ = CreateAction(ui->lineEditPassword);
    action_confirm_ = CreateAction(ui->lineEditPasswordConfirm);

    RLoginDialog();
    InitConnect();
}

AuthDialog::~AuthDialog() { delete ui; }

void AuthDialog::RLoginResult(bool result)
{
    if (result) {
        SyncLoginInfo(ui->lineEditWorkspace->text());
        SaveLoginConfig();
        this->close();
    } else {
        QMessageBox::critical(
            this, tr("Login Failed"), tr("Unable to log in. Please verify your email, password, workspace access, or check if your account has expired."));
    }
}

void AuthDialog::RRegisterResult(bool result)
{
    if (result) {
        SyncLoginInfo();
        RLoginDialog();
    } else {
        QMessageBox::critical(this, tr("Registration Failed"), tr("Unable to register. Please contact the administrator for details."));
    }
}

void AuthDialog::RWorkspaceAccessPending(const QString& email, const QString& workspace)
{
    const QString message
        = tr("Your access to workspace \"%1\" for email \"%2\" is pending approval. Please contact the administrator if needed.").arg(workspace, email);

    QMessageBox::information(this, tr("Workspace Access Pending"), message);
}

void AuthDialog::on_pushButtonLogin_clicked()
{
    const QString email { ui->lineEditEmail->text().trimmed() };
    const QString password { ui->lineEditPassword->text() };
    const QString workspace { ui->lineEditWorkspace->text() };
    WebSocket::Instance()->SendMessage(kLogin, JsonGen::Login(email, password, workspace));
}

void AuthDialog::RRegisterDialog()
{
    ui->lineEditEmail->setText(QString());
    ui->lineEditPassword->setText(QString());
    ui->lineEditPasswordConfirm->setText(QString());

    ui->labelHeader->setText(tr("Need a YTX account?"));
    ui->labelTail->setText(tr("Have YTX account?"));

    ui->labelSignUp->setHidden(true);
    ui->lineEditWorkspace->setHidden(true);
    ui->chkBoxSave->setHidden(true);
    ui->pushButtonLogin->setHidden(true);

    ui->labelLogin->show();
    ui->pushButtonRegister->show();
    ui->lineEditPasswordConfirm->show();

    adjustSize();
}

void AuthDialog::RLoginDialog()
{
    LoginInfo& login_info { LoginInfo::Instance() };

    ui->lineEditEmail->setText(login_info.Email());
    ui->lineEditWorkspace->setText(login_info.Workspace());
    ui->lineEditPassword->setText(login_info.Password());
    ui->chkBoxSave->setChecked(login_info.IsSaved());

    ui->labelHeader->setText(tr("Log in to YTX account"));
    ui->labelTail->setText(tr("Don't have YTX account?"));

    ui->pushButtonRegister->setHidden(true);
    ui->labelLogin->setHidden(true);
    ui->lineEditPasswordConfirm->setHidden(true);

    ui->labelSignUp->show();
    ui->chkBoxSave->show();
    ui->pushButtonLogin->show();
    ui->lineEditWorkspace->show();

    adjustSize();
}

void AuthDialog::SaveLoginConfig()
{
    const bool is_saved { ui->chkBoxSave->isChecked() };

    LoginInfo& login_info { LoginInfo::Instance() };
    login_info.SetIsSaved(is_saved);

    if (!is_saved)
        login_info.SetPassword({});

    login_info.WriteConfig(local_settings_);
}

void AuthDialog::InitConnect()
{
    connect(WebSocket::Instance(), &WebSocket::SLoginResult, this, &AuthDialog::RLoginResult);
    connect(WebSocket::Instance(), &WebSocket::SRegisterResult, this, &AuthDialog::RRegisterResult);

    connect(WebSocket::Instance(), &WebSocket::SWorkspaceAccessPending, this, &AuthDialog::RWorkspaceAccessPending);

    connect(ui->labelSignUp, &QLabel::linkActivated, this, &AuthDialog::RRegisterDialog);
    connect(ui->labelLogin, &QLabel::linkActivated, this, &AuthDialog::RLoginDialog);
}

void AuthDialog::SyncLoginInfo(const QString& workspace)
{
    LoginInfo& login_info { LoginInfo::Instance() };

    login_info.SetEmail(ui->lineEditEmail->text());
    login_info.SetPassword(ui->lineEditPassword->text());
    login_info.SetWorkspace(workspace);
    login_info.SetIsSaved(ui->chkBoxSave->isChecked());
}

QAction* AuthDialog::CreateAction(QLineEdit* lineEdit)
{
    QAction* toggle_action { new QAction(lineEdit) };
    toggle_action->setCheckable(true);
    lineEdit->addAction(toggle_action, QLineEdit::TrailingPosition);

    connect(toggle_action, &QAction::toggled, this, [=](bool checked) {
        if (checked) {
            lineEdit->setEchoMode(QLineEdit::Normal);
        } else {
            lineEdit->setEchoMode(QLineEdit::Password);
        }
    });

    return toggle_action;
}

void AuthDialog::on_pushButtonRegister_clicked()
{
    const QString email { ui->lineEditEmail->text().trimmed() };
    const QString password { ui->lineEditPassword->text() };
    const QString confirm_pwd { ui->lineEditWorkspace->text() };

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

    WebSocket::Instance()->SendMessage(kRegister, JsonGen::Register(email, password));
}
