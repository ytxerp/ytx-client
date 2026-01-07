#include "logininfo.h"

#include "component/constant.h"
#include "global/password_encryption.h"

LoginInfo& LoginInfo::Instance()
{
    static LoginInfo instance {};
    return instance;
}

void LoginInfo::WriteConfig(QSharedPointer<QSettings> local_settings)
{
    if (!local_settings)
        return;

    local_settings->beginGroup(kLogin);

    local_settings->setValue(kUser, email_);

    if (password_remembered_ && !password_.isEmpty()) {
        const QString encrypted_pwd { PasswordEncryption::Encrypt(password_, machine_key_) };
        local_settings->setValue(kPassword, encrypted_pwd);
    } else {
        local_settings->remove(kPassword);
    }

    local_settings->setValue(kWorkspace, workspace_);
    local_settings->setValue(kPasswordRemembered, password_remembered_);

    local_settings->endGroup();
}

void LoginInfo::ReadConfig(QSharedPointer<QSettings> local_settings)
{
    if (!local_settings)
        return;

    local_settings->beginGroup(kLogin);

    email_ = local_settings->value(kUser, {}).toString();
    password_remembered_ = local_settings->value(kPasswordRemembered, false).toBool();
    workspace_ = local_settings->value(kWorkspace, {}).toString();

    const QString encrypted_pwd { local_settings->value(kPassword, {}).toString() };

    if (password_remembered_ && !encrypted_pwd.isEmpty()) {
        password_ = PasswordEncryption::Decrypt(encrypted_pwd, machine_key_);
    } else {
        password_.clear();
    }

    local_settings->endGroup();
}

void LoginInfo::Clear()
{
    email_.clear();
    password_.clear();
    workspace_.clear();
    password_remembered_ = false;
}

LoginInfo::LoginInfo()
    : machine_key_ { PasswordEncryption::GetMachineKey() }
{
}
