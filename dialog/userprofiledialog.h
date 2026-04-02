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

#ifndef USERPROFILEDIALOG_H
#define USERPROFILEDIALOG_H

#include <QDialog>

namespace Ui {
class UserProfileDialog;
}

class UserProfileDialog : public QDialog {
    Q_OBJECT

public:
    explicit UserProfileDialog(QWidget* parent = nullptr);
    ~UserProfileDialog() override;

private slots:
    void on_pushButtonSave_clicked();

private:
    void InitDialog();
    void EditUsernameFinished();
    void EditNameFinished();

private:
    Ui::UserProfileDialog* ui;
};

#endif // USERPROFILEDIALOG_H
