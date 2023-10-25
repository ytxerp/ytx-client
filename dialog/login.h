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

#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class Login;
}

class Login final : public QDialog {
    Q_OBJECT

public:
    explicit Login(QSharedPointer<QSettings> local_settings, QWidget* parent = nullptr);
    ~Login();

public slots:
    void RLoginResult(bool result);
    void RWorkspaceAccessPending(const QString& email, const QString& workspace);

private slots:
    void on_pushButtonLogin_clicked();

private:
    void IniDialog();
    void SaveLoginConfig();

private:
    Ui::Login* ui;
    QSharedPointer<QSettings> local_settings_ {};
};

#endif // LOGIN_H
