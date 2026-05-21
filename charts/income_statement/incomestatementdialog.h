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

#ifndef INCOMESTATEMENTDIALOG_H
#define INCOMESTATEMENTDIALOG_H

#include <QDialog>
#include <QTreeView>

#include "charts/income_statement/incomestatementmodel.h"
#include "tree/model/treemodel.h"

namespace Ui {
class IncomeStatementDialog;
}

class IncomeStatementDialog final : public QDialog {
    Q_OBJECT

public:
    explicit IncomeStatementDialog(CTreeModel* tree_model, IncomeStatementModel* model, const QUuid& widget_id, QWidget* parent = nullptr);
    ~IncomeStatementDialog() override;

    QTreeView* View();
    IncomeStatementModel* Model() { return model_; }
    void ResetNetProfit(double value);

private slots:
    void on_dateTimeEditEnd_dateChanged(const QDate& date);
    void on_dateTimeEditStart_dateChanged(const QDate& date);
    void on_pushButtonFetch_clicked();

private:
    void InitDialog();
    void InitTimer();

private:
    Ui::IncomeStatementDialog* ui;

    QDateTime start_ {};
    QDateTime end_ {};
    const QUuid widget_id_ {};

    IncomeStatementModel* model_ {};
    CTreeModel* tree_model_ {};

    QTimer* cooldown_timer_ { nullptr };
};

#endif // INCOMESTATEMENTDIALOG_H
