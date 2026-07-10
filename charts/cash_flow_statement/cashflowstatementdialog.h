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

#ifndef CASHFLOWSTATEMENTDIALOG_H
#define CASHFLOWSTATEMENTDIALOG_H

#include <QDateTime>
#include <QDialog>
#include <QTableView>
#include <QTimer>
#include <QTreeView>
#include <QUuid>

#include "cashflowcarriermodel.h"
#include "cashflowspecialmodel.h"
#include "cashflowstatementmodel.h"
#include "cashflowwrongmodel.h"

namespace Ui {
class CashFlowStatementDialog;
}

class CashFlowStatementDialog final : public QDialog {
    Q_OBJECT

public:
    explicit CashFlowStatementDialog(cash_flow::Model* model, cash_flow::CarrierModel* carrier, cash_flow::SpecialModel* special, cash_flow::WrongModel* wrong,
        const QUuid& widget_id, QWidget* parent = nullptr);
    ~CashFlowStatementDialog() override;

    QTreeView* View();
    QTreeView* CarrierView();
    QTreeView* SpecialView();
    QTableView* WrongView();

    cash_flow::Model* Model() { return model_; }
    cash_flow::CarrierModel* CarrierModel() { return carrier_; }
    cash_flow::SpecialModel* SpecialModel() { return special_; }
    cash_flow::WrongModel* WrongModel() { return wrong_; }

private slots:
    void on_dateTimeEditEnd_dateChanged(const QDate& date);
    void on_dateTimeEditStart_dateChanged(const QDate& date);
    void on_pushButtonFetch_clicked();

private:
    void InitDialog();
    void InitTimer();

private:
    Ui::CashFlowStatementDialog* ui;

    QDateTime start_ {};
    QDateTime end_ {};
    const QUuid widget_id_ {};

    cash_flow::Model* model_ {};
    cash_flow::CarrierModel* carrier_ {};
    cash_flow::SpecialModel* special_ {};
    cash_flow::WrongModel* wrong_ {};

    QTimer* cooldown_timer_ { nullptr };
};

#endif // CASHFLOWSTATEMENTDIALOG_H
