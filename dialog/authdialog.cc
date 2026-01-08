#include "authdialog.h"

#include <QMessageBox>

#include "component/signalblocker.h"
#include "enum/authenum.h"
#include "global/logininfo.h"
#include "ui_authdialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

AuthDialog::AuthDialog(const QSharedPointer<QSettings>& local_settings, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AuthDialog)
    , local_settings_ { local_settings }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->labelSignUp->setFocusPolicy(Qt::NoFocus);
    ui->labelSignIn->setFocusPolicy(Qt::NoFocus);
    ui->labelSignUp->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    ui->labelSignIn->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    CreateAction(ui->lineEditPassword);
    CreateAction(ui->lineEditPasswordConfirm);

    RLoginDialog();
    InitConnect();
}

AuthDialog::~AuthDialog() { delete ui; }

void AuthDialog::RLoginResult(bool result, int code)
{
    if (result) {
        close();
        return;
    }

    QString message {};
    QString title { tr("Login Failed") };

    switch (static_cast<LoginOutcome>(code)) {
    case LoginOutcome::EmptyEmail:
        message = tr("Please enter your email.");
        break;
    case LoginOutcome::EmptyPassword:
        message = tr("Please enter your password.");
        break;
    case LoginOutcome::EmailNotFound:
        message = tr("The email you entered was not found.");
        break;
    case LoginOutcome::PasswordIncorrect:
        message = tr("The password you entered is incorrect.");
        break;
    case LoginOutcome::WorkspaceNotFound:
        message = tr("The specified workspace does not exist.");
        break;
    case LoginOutcome::WorkspaceExpired:
        message = tr("The workspace subscription has expired.");
        break;
    case LoginOutcome::WorkspaceAccessPending:
        title = tr("Access Pending");
        message
            = tr("Your access to workspace \"%1\" for email \"%2\" is pending approval.").arg(LoginInfo::Instance().Workspace(), LoginInfo::Instance().Email());
        break;
    case LoginOutcome::AlreadyLoggedIn:
        message = tr("You are already logged in.");
        break;
    case LoginOutcome::ServerError:
        message = tr("Server error occurred. Please try again later.");
        break;
    default:
        message = tr("Unable to log in. Please contact the administrator for details.");
        break;
    }

    QMessageBox::critical(this, title, message);
}

void AuthDialog::RRegisterResult(bool result, int code)
{
    if (result) {
        SyncLoginInfo();
        LoginInfo::Instance().WriteConfig(local_settings_);

        RLoginDialog();
        QMessageBox::information(this, tr("Registration Successful"), tr("Your account has been registered successfully."));
        return;
    }

    {
        QString message {};
        switch (RegisterOutcome(code)) {
        case RegisterOutcome::EmptyEmail:
            message = tr("Please enter your email.");
            break;
        case RegisterOutcome::EmptyPassword:
            message = tr("Please enter your password.");
            break;
        case RegisterOutcome::InvalidEmail:
            message = tr("The email format is invalid.");
            break;
        case RegisterOutcome::EmailAlreadyExists:
            message = tr("This email is already registered.");
            break;
        case RegisterOutcome::ServerError:
            message = tr("Server error occurred. Please try again later.");
            break;
        default:
            message = tr("Unable to register. Please contact the administrator for details.");
            break;
        }

        QMessageBox::critical(this, tr("Registration Failed"), message);
    }
}

void AuthDialog::on_pushButtonLogin_clicked()
{
    const QString email { ui->lineEditEmail->text().trimmed() };
    const QString password { ui->lineEditPassword->text() };
    const QString workspace { ui->lineEditWorkspace->text().trimmed() };

    if (!ValidateEmail(email))
        return;

    if (!ValidatePassword(password)) {
        return;
    }

    if (workspace.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Workspace"), tr("Workspace cannot be empty"));
        return;
    }

    WebSocket::Instance()->SendMessage(kLogin, JsonGen::Login(email, password, workspace));

    SyncLoginInfo();
    LoginInfo::Instance().WriteConfig(local_settings_);
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
    ui->chkBoxPasswordRemembered->setHidden(true);
    ui->pushButtonLogin->setHidden(true);

    ui->labelSignIn->show();
    ui->pushButtonRegister->show();
    ui->lineEditPasswordConfirm->show();

    setWindowTitle(tr("Sign Up"));

    adjustSize();
}

void AuthDialog::RLoginDialog()
{
    LoginInfo& login_info { LoginInfo::Instance() };

    ui->lineEditEmail->setText(login_info.Email());
    ui->lineEditWorkspace->setText(login_info.Workspace());
    ui->lineEditPassword->setText(login_info.Password());
    ui->chkBoxPasswordRemembered->setChecked(login_info.PasswordRemembered());

    ui->labelHeader->setText(tr("Log in to YTX account"));
    ui->labelTail->setText(tr("Don't have YTX account?"));

    ui->pushButtonRegister->setHidden(true);
    ui->labelSignIn->setHidden(true);
    ui->lineEditPasswordConfirm->setHidden(true);

    ui->labelSignUp->show();
    ui->chkBoxPasswordRemembered->show();
    ui->pushButtonLogin->show();
    ui->lineEditWorkspace->show();

    setWindowTitle(tr("Sign In"));

    adjustSize();
}

void AuthDialog::InitConnect()
{
    connect(WebSocket::Instance(), &WebSocket::SLoginResult, this, &AuthDialog::RLoginResult);
    connect(WebSocket::Instance(), &WebSocket::SRegisterResult, this, &AuthDialog::RRegisterResult);

    connect(ui->labelSignUp, &QLabel::linkActivated, this, &AuthDialog::RRegisterDialog);
    connect(ui->labelSignIn, &QLabel::linkActivated, this, &AuthDialog::RLoginDialog);
}

void AuthDialog::SyncLoginInfo()
{
    LoginInfo& login_info { LoginInfo::Instance() };

    login_info.SetEmail(ui->lineEditEmail->text().trimmed());
    login_info.SetWorkspace(ui->lineEditWorkspace->text().trimmed());

    const bool remember { ui->chkBoxPasswordRemembered->isChecked() };

    login_info.SetPasswordRemembered(remember);
    login_info.SetPassword(remember ? ui->lineEditPassword->text() : QString());
}

void AuthDialog::CreateAction(QLineEdit* lineEdit)
{
    QAction* action { new QAction(lineEdit) };
    action->setCheckable(true);
    lineEdit->addAction(action, QLineEdit::TrailingPosition);

    action->setIcon(QIcon(":/solarized_dark/solarized_dark/eye_closed.png"));

    connect(action, &QAction::toggled, this, [=](bool checked) {
        if (checked) {
            lineEdit->setEchoMode(QLineEdit::Normal);
            action->setIcon(QIcon(":/solarized_dark/solarized_dark/eye_open.png"));
        } else {
            lineEdit->setEchoMode(QLineEdit::Password);
            action->setIcon(QIcon(":/solarized_dark/solarized_dark/eye_closed.png"));
        }
    });
}

bool AuthDialog::ValidateEmail(const QString& email)
{
    if (email.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Email"), tr("Email cannot be empty"));
        return false;
    }

    static QRegularExpression re(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");

    if (!re.match(email).hasMatch()) {
        QMessageBox::warning(this, tr("Invalid Email"), tr("Please enter a valid email address."));
        return false;
    }

    return true;
}

bool AuthDialog::ValidatePassword(const QString& password)
{
    if (password.length() < 8) {
        QMessageBox::warning(this, tr("Invalid Password"), tr("Password must be at least 8 characters long."));
        return false;
    }

    {
        static QRegularExpression re(R"([A-Za-z])");

        if (!password.contains(re)) {
            QMessageBox::warning(this, tr("Invalid Password"), tr("Password must include at least one letter."));
            return false;
        }
    }

    {
        static QRegularExpression re(R"(\d)");

        if (!password.contains(re)) {
            QMessageBox::warning(this, tr("Invalid Password"), tr("Password must include at least one digit."));
            return false;
        }
    }

    {
        static QRegularExpression re(R"([^\w\s])");

        if (!password.contains(re)) {
            QMessageBox::warning(this, tr("Invalid Password"), tr("Password must include at least one special character."));
            return false;
        }
    }

    return true;
}

void AuthDialog::on_pushButtonRegister_clicked()
{
    const QString email { ui->lineEditEmail->text().trimmed() };
    const QString password { ui->lineEditPassword->text() };
    const QString confirm_pwd { ui->lineEditPasswordConfirm->text() };

    if (!ValidateEmail(email))
        return;

    if (!ValidatePassword(password)) {
        return;
    }

    if (password != confirm_pwd) {
        QMessageBox::warning(this, tr("Password Mismatch"), tr("The passwords do not match."));
        return;
    }

    WebSocket::Instance()->SendMessage(kRegister, JsonGen::Register(email, password));
}
