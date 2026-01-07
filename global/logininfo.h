/*
 * Copyright (C) 2023 YTX
 *
 * This file is part of YTX.
 *
 * YTX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * YTX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YTX. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LOGININFO_H
#define LOGININFO_H

#include <QSettings>
#include <QSharedPointer>
#include <QString>
#include <QUuid>

class LoginInfo {
public:
    static LoginInfo& Instance();

    void WriteConfig(QSharedPointer<QSettings> local_settings);
    void ReadConfig(QSharedPointer<QSettings> local_settings);

    // Getter
    const QString& Email() const { return email_; }
    const QString& Password() const { return password_; }
    const QString& Workspace() const { return workspace_; }
    bool PasswordRemembered() const { return password_remembered_; }

    // Setter
    void SetEmail(const QString& value) { email_ = value; }
    void SetPassword(const QString& value) { password_ = value; }
    void SetWorkspace(const QString& value) { workspace_ = value; }
    void SetPasswordRemembered(bool value) { password_remembered_ = value; }

    // Remove
    void Clear();

    LoginInfo(const LoginInfo&) = delete;
    LoginInfo& operator=(const LoginInfo&) = delete;
    LoginInfo(LoginInfo&&) = delete;
    LoginInfo& operator=(LoginInfo&&) = delete;

private:
    LoginInfo();
    ~LoginInfo() = default;

private:
    QString email_ {};
    QString password_ {};
    QString workspace_ {};
    bool password_remembered_ { false };

    const QByteArray machine_key_ {};
};

#endif // LOGININFO_H
