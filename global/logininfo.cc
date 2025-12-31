#include "logininfo.h"

#include "component/constant.h"

void LoginInfo::WriteConfig(QSharedPointer<QSettings> local_settings)
{
    if (!local_settings)
        return;

    local_settings->beginGroup(kLogin);
    local_settings->setValue(kUser, email_);
    local_settings->setValue(kPassword, password_);
    local_settings->setValue(kWorkspace, workspace_);
    local_settings->setValue(kIsSaved, password_remembered_);
    local_settings->endGroup();
}

void LoginInfo::ReadConfig(QSharedPointer<QSettings> local_settings)
{
    if (!local_settings)
        return;

    local_settings->beginGroup(kLogin);
    email_ = local_settings->value(kUser, {}).toString();
    password_ = local_settings->value(kPassword, {}).toString();
    workspace_ = local_settings->value(kWorkspace, {}).toString();
    password_remembered_ = local_settings->value(kIsSaved, false).toBool();
    local_settings->endGroup();
}

void LoginInfo::Clear()
{
    email_.clear();
    password_.clear();
    workspace_.clear();
    password_remembered_ = false;
}
