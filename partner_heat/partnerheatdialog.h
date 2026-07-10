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

#ifndef PARTNERHEATDIALOG_H
#define PARTNERHEATDIALOG_H

#include <QDialog>
#include <QTableView>

#include "partnerheatmodel.h"

namespace Ui {
class PartnerHeatDialog;
}

class PartnerHeatDialog final : public QDialog {
    Q_OBJECT

public:
    explicit PartnerHeatDialog(partner_heat::Model* model, const QUuid& widget_id, QWidget* parent = nullptr);
    ~PartnerHeatDialog() override;

    QTableView* View();
    partner_heat::Model* Model() { return model_; }

private slots:
    void on_pushButtonFetch_clicked();
    void on_dateTimeEditStart_dateChanged(const QDate& date);
    void on_dateTimeEditEnd_dateChanged(const QDate& date);

private:
    void InitDialog();
    void InitTimer();

private:
    Ui::PartnerHeatDialog* ui;

    QDateTime start_ {};
    QDateTime end_ {};

    QTimer* cooldown_timer_ { nullptr };

    partner_heat::Model* model_ {};
    const QUuid widget_id_ {};
};

#endif // PARTNERHEATDIALOG_H
