#include "userprofiledialog.h"

#include "global/logininfo.h"
#include "global/userprofile.h"
#include "ui_userprofiledialog.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

UserProfileDialog::UserProfileDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::UserProfileDialog)
{
    ui->setupUi(this);
    InitDialog();
}

UserProfileDialog::~UserProfileDialog() { delete ui; }

void UserProfileDialog::InitDialog()
{
    const auto& profile { UserProfile::Instance() };
    const auto& login_info { LoginInfo::Instance() };

    ui->lineEditEmail->setText(login_info.Email());
    ui->lineEditEmail->setReadOnly(true);

    ui->lineEditUsername->setText(profile.Username());
    ui->lineEditName->setText(profile.Name());

    ui->pushButtonSave->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
}

void UserProfileDialog::EditUsernameFinished()
{
    const auto username { ui->lineEditUsername->text().trimmed() };
    if (username == UserProfile::Instance().Username() || username.isEmpty()) {
        return;
    }

    // Validate username format
    static const QRegularExpression kUsernameRegex("^[a-z][a-z0-9_]{2,31}$");
    if (!kUsernameRegex.match(username).hasMatch() || username.contains("__") || username.endsWith('_')) {
        Utils::ShowNotification(QMessageBox::Warning, tr("Invalid Username"),
            tr("Username must be 3-32 characters, start with a letter, and contain only lowercase letters, digits, or underscores. No consecutive or trailing "
               "underscores."),
            TimeConst::kAutoCloseMs);
        return;
    }

    UserProfile::Instance().SetUsername(username);

    const auto message { JsonGen::AccountUsername(LoginInfo::Instance().Email(), username) };
    WebSocket::Instance()->SendMessage(WsKey::kAccountUsernameUpdate, message);
}

void UserProfileDialog::EditNameFinished()
{
    const auto name { ui->lineEditName->text().trimmed() };
    if (name == UserProfile::Instance().Name()) {
        return;
    }

    UserProfile::Instance().SetName(name);

    const auto message { JsonGen::AccountName(LoginInfo::Instance().Email(), name) };
    WebSocket::Instance()->SendMessage(WsKey::kAccountNameUpdate, message);
}

void UserProfileDialog::on_pushButtonSave_clicked()
{
    EditNameFinished();
    EditUsernameFinished();
}
