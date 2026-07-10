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

#ifndef BALANCESHEETDIALOG_H
#define BALANCESHEETDIALOG_H

#include <QDateTime>
#include <QDialog>
#include <QTreeView>
#include <QUuid>

#include "balancesheetmodel.h"
#include "tree/model/treemodel.h"

namespace Ui {
class BalanceSheetDialog;
}

class BalanceSheetDialog final : public QDialog {
    Q_OBJECT

public:
    explicit BalanceSheetDialog(CTreeModel* tree_model, balance_sheet::Model* model, const QUuid& widget_id, QWidget* parent = nullptr);
    ~BalanceSheetDialog() override;

    QTreeView* View();
    balance_sheet::Model* Model() { return model_; }

private slots:
    void on_dateTimeEditEnd_dateChanged(const QDate& date);
    void on_pushButtonFetch_clicked();
    void on_dateTimeEditStart_dateChanged(const QDate& date);

private:
    void InitDialog();
    void InitTimer();

private:
    Ui::BalanceSheetDialog* ui;

    QDateTime start_ {};
    QDateTime end_ {};

    const QUuid widget_id_ {};

    balance_sheet::Model* model_ {};
    CTreeModel* tree_model_ {};

    QTimer* cooldown_timer_ { nullptr };
};

#endif // BALANCESHEETDIALOG_H
