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

#pragma once

#include <QDialog>
#include <QTableView>

#include "tagmodel.h"

namespace Ui {
class TagDialog;
}

class TagDialog final : public QDialog {
    Q_OBJECT

public:
    explicit TagDialog(tag::Model* model, QWidget* parent = nullptr);
    ~TagDialog() override;

    QTableView* View();

private slots:
    void on_pBtnDelete_clicked();
    void on_pBtnInsert_clicked();

private:
    Ui::TagDialog* ui;
    tag::Model* model_ {};
};
