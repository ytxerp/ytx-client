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

#include "inventoryheatmodel.h"
#include "utils/daterange.h"

namespace Ui {
class InventoryHeatDialog;
}

class InventoryHeatDialog final : public QDialog {
    Q_OBJECT

public:
    explicit InventoryHeatDialog(inventory_heat::Model* model, const QUuid& widget_id, QWidget* parent = nullptr);
    ~InventoryHeatDialog() override;

    QTableView* View();
    inventory_heat::Model* Model() { return model_; }

private slots:
    void on_pushButtonFetch_clicked();
    void on_dateEditStart_dateChanged(const QDate& date);
    void on_dateEditEnd_dateChanged(const QDate& date);

private:
    void InitDialog();
    void InitTimer();
    static utils::DateRange DefaultRange()
    {
        const auto today { QDate::currentDate() };
        return { today.addYears(-2), today };
    }

private:
    Ui::InventoryHeatDialog* ui;

    utils::DateRange range_ {};

    QTimer* cooldown_timer_ { nullptr };

    inventory_heat::Model* model_ {};
    const QUuid widget_id_ {};
};
