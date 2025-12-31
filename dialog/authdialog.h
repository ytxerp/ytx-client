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

#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSettings>

namespace Ui {
class AuthDialog;
}

class AuthDialog final : public QDialog {
    Q_OBJECT

public:
    explicit AuthDialog(QSharedPointer<QSettings> local_settings, QWidget* parent = nullptr);
    ~AuthDialog();

public slots:
    void RLoginResult(bool result, int code);
    void RRegisterResult(bool result, int code);

private slots:
    void on_pushButtonLogin_clicked();
    void RRegisterDialog();
    void RLoginDialog();

    void on_pushButtonRegister_clicked();

private:
    void InitConnect();
    void SyncLoginInfo();
    QAction* CreateAction(QLineEdit* lineEdit);

    bool ValidateEmail(const QString& email);
    bool ValidatePassword(const QString& password);

private:
    Ui::AuthDialog* ui;
    QSharedPointer<QSettings> local_settings_ {};
    QAction* action_password_ {};
    QAction* action_confirm_ {};
};

#endif // AUTHDIALOG_H
