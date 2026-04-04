#include "authdialog.h"

#include "about.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "enum/authenum.h"
#include "global/logininfo.h"
#include "ui_authdialog.h"
#include "utils/mainwindowutils.h"
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

    InitDialog();
    RLoginDialog();
    InitConnect();
}

AuthDialog::~AuthDialog() { delete ui; }

void AuthDialog::RAllowLogin()
{
    qInfo() << "[Auth]" << "Login succeeded" << ui->lineEditEmail->text().trimmed() << ui->lineEditWorkspace->text().trimmed();
    close();
}

void AuthDialog::RDenyLogin(int code)
{
    QString message {};
    QString title { tr("Login Failed") };

    switch (LoginOutcome(code)) {
    case LoginOutcome::EmptyEmail:
        message = tr("Please enter your email.");
        break;
    case LoginOutcome::EmptyPassword:
        message = tr("Please enter your password.");
        break;
    case LoginOutcome::EmailNotFound:
        message = tr("No account found for this email.");
        break;
    case LoginOutcome::PasswordIncorrect:
        message = tr("Incorrect password. Please try again.");
        break;
    case LoginOutcome::WorkspaceNotFound:
        message = tr("Workspace not found. Please check the name and try again.");
        break;
    case LoginOutcome::WorkspaceExpired:
        message = tr("This workspace's subscription has expired.");
        break;
    case LoginOutcome::WorkspaceAccessPending:
        title = tr("Access Pending");
        message = tr("Your request to join workspace %1 is awaiting approval.").arg(LoginInfo::Instance().Workspace());
        break;
    case LoginOutcome::AlreadyLoggedIn:
        message = tr("You're already logged in.");
        break;
    case LoginOutcome::ServerError:
        message = tr("Something went wrong on our end. Please try again later.");
        break;
    case LoginOutcome::Success:
        return;
    default:
        message = tr("Unable to sign in. Please contact support for help.");
        break;
    }

    qWarning() << "[Auth] Login failed"
               << "| email:" << ui->lineEditEmail->text().trimmed() << "| workspace:" << ui->lineEditWorkspace->text().trimmed() << "| reason:" << message;
    Utils::ShowNotification(QMessageBox::Critical, title, message, TimeConst::kAutoCloseMs);
}

void AuthDialog::RRegisterResult(bool result, int code)
{
    if (result) {
        SyncLoginInfo();
        LoginInfo::Instance().WriteConfig(local_settings_);
        RLoginDialog();
        qInfo() << "[Auth] Registration successful"
                << "| email:" << ui->lineEditEmail->text().trimmed();
        Utils::ShowNotification(
            QMessageBox::Information, tr("Registration Successful"), tr("Your account has been created. Welcome aboard!"), TimeConst::kAutoCloseMs);
        return;
    }

    QString message {};
    switch (RegisterOutcome(code)) {
    case RegisterOutcome::EmptyEmail:
        message = tr("Please enter your email.");
        break;
    case RegisterOutcome::EmptyPassword:
        message = tr("Please enter your password.");
        break;
    case RegisterOutcome::InvalidEmail:
        message = tr("Please enter a valid email address.");
        break;
    case RegisterOutcome::EmailAlreadyExists:
        message = tr("This email is already registered. Try signing in instead.");
        break;
    case RegisterOutcome::ServerError:
        message = tr("Something went wrong on our end. Please try again later.");
        break;
    case RegisterOutcome::Success:
        return;
    case RegisterOutcome::UsernameGenerationFailed:
        message = tr("Failed to generate a username. Please try again.");
        break;
    default:
        message = tr("Unable to register. Please contact support for help.");
        break;
    }

    qWarning() << "[Auth] Registration failed"
               << "| email:" << ui->lineEditEmail->text().trimmed() << "| reason:" << message;

    Utils::ShowNotification(QMessageBox::Critical, tr("Registration Failed"), message, TimeConst::kAutoCloseMs);
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
        Utils::ShowNotification(QMessageBox::Warning, tr("Invalid Workspace"), tr("Workspace cannot be empty"), TimeConst::kAutoCloseMs);
        return;
    }

    WebSocket::Instance()->SendMessage(WsKey::kLogin, JsonGen::Login(email, password, workspace));

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
    ui->checkBoxPrivacy->setVisible(true);
    ui->checkBoxTerms->setVisible(true);
    ui->labelPrivacy->setVisible(true);
    ui->labelTerms->setVisible(true);

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
    ui->checkBoxPrivacy->setHidden(true);
    ui->checkBoxTerms->setHidden(true);
    ui->labelPrivacy->setHidden(true);
    ui->labelTerms->setHidden(true);

    ui->labelSignUp->show();
    ui->chkBoxPasswordRemembered->show();
    ui->pushButtonLogin->show();
    ui->lineEditWorkspace->show();

    setWindowTitle(tr("Sign In"));

    adjustSize();
}

void AuthDialog::InitConnect()
{
    connect(WebSocket::Instance(), &WebSocket::SLoginAllow, this, &AuthDialog::RAllowLogin);
    connect(WebSocket::Instance(), &WebSocket::SLoginDeny, this, &AuthDialog::RDenyLogin);
    connect(WebSocket::Instance(), &WebSocket::SRegisterResult, this, &AuthDialog::RRegisterResult);

    connect(ui->labelSignUp, &QLabel::linkActivated, this, &AuthDialog::RRegisterDialog);
    connect(ui->labelSignIn, &QLabel::linkActivated, this, &AuthDialog::RLoginDialog);

    connect(ui->labelPrivacy, &QLabel::linkActivated, this, &AuthDialog::ROpenPrivacyHtml);
    connect(ui->labelTerms, &QLabel::linkActivated, this, &AuthDialog::ROpenTermsHtml);
}

void AuthDialog::InitDialog()
{
    ui->labelSignIn->setText(QString("<a href='signin'>%1</a>").arg(tr("Sign In")));
    ui->labelSignUp->setText(QString("<a href='signup'>%1</a>").arg(tr("Sign Up")));
    ui->labelPrivacy->setText(QString("<a href='privacy'>%1</a>").arg(tr("Privacy Policy")));
    ui->labelTerms->setText(QString("<a href='terms'>%1</a>").arg(tr("Terms of Service")));
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

void AuthDialog::CreateAction(QLineEdit* line_edit)
{
    QAction* action { new QAction(line_edit) };
    action->setCheckable(true);
    line_edit->addAction(action, QLineEdit::TrailingPosition);

    action->setIcon(QIcon(":/solarized_dark/solarized_dark/eye_closed.png"));

    connect(action, &QAction::toggled, this, [line_edit, action](bool checked) {
        if (checked) {
            line_edit->setEchoMode(QLineEdit::Normal);
            action->setIcon(QIcon(":/solarized_dark/solarized_dark/eye_open.png"));
        } else {
            line_edit->setEchoMode(QLineEdit::Password);
            action->setIcon(QIcon(":/solarized_dark/solarized_dark/eye_closed.png"));
        }
    });
}

bool AuthDialog::ValidateEmail(const QString& email)
{
    if (email.isEmpty()) {
        Utils::ShowNotification(QMessageBox::Warning, tr("Invalid Email"), tr("Email cannot be empty"), TimeConst::kAutoCloseMs);
        return false;
    }

    static QRegularExpression re(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");

    if (!re.match(email).hasMatch()) {
        Utils::ShowNotification(QMessageBox::Warning, tr("Invalid Email"), tr("Please enter a valid email address."), TimeConst::kAutoCloseMs);
        return false;
    }

    return true;
}

bool AuthDialog::ValidatePassword(const QString& password)
{
    if (password.length() < 8) {
        Utils::ShowNotification(QMessageBox::Warning, tr("Invalid Password"), tr("Password must be at least 8 characters long."), TimeConst::kAutoCloseMs);
        return false;
    }

    {
        static QRegularExpression re(R"([A-Za-z])");

        if (!password.contains(re)) {
            Utils::ShowNotification(QMessageBox::Warning, tr("Invalid Password"), tr("Password must include at least one letter."), TimeConst::kAutoCloseMs);
            return false;
        }
    }

    {
        static QRegularExpression re(R"(\d)");

        if (!password.contains(re)) {
            Utils::ShowNotification(QMessageBox::Warning, tr("Invalid Password"), tr("Password must include at least one digit."), TimeConst::kAutoCloseMs);
            return false;
        }
    }

    {
        static QRegularExpression re(R"([^\w\s])");

        if (!password.contains(re)) {
            Utils::ShowNotification(
                QMessageBox::Warning, tr("Invalid Password"), tr("Password must include at least one special character."), TimeConst::kAutoCloseMs);
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
        Utils::ShowNotification(QMessageBox::Warning, tr("Password Mismatch"), tr("The passwords do not match."), TimeConst::kAutoCloseMs);
        return;
    }

    WebSocket::Instance()->SendMessage(WsKey::kRegister, JsonGen::Register(email, password));
}

void AuthDialog::on_checkBoxPrivacy_checkStateChanged(const Qt::CheckState& arg1)
{
    const bool privacy_checked { arg1 == Qt::Checked };
    const bool terms_checked { ui->checkBoxTerms->isChecked() };
    ui->pushButtonRegister->setEnabled(terms_checked && privacy_checked);
}

void AuthDialog::on_checkBoxTerms_checkStateChanged(const Qt::CheckState& arg1)
{
    const bool terms_checked { arg1 == Qt::Checked };
    const bool privacy_checked { ui->checkBoxPrivacy->isChecked() };
    ui->pushButtonRegister->setEnabled(terms_checked && privacy_checked);
}

void AuthDialog::ROpenPrivacyHtml() { About::OpenResourceHtml("privacy_policy.html"); }

void AuthDialog::ROpenTermsHtml() { About::OpenResourceHtml("terms_of_service.html"); }
