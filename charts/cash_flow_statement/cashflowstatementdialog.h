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
    explicit CashFlowStatementDialog(CashFlowStatementModel* model, CashFlowCarrierModel* carrier, CashFlowSpecialModel* special, CashFlowWrongModel* wrong,
        const QUuid& widget_id, QWidget* parent = nullptr);
    ~CashFlowStatementDialog() override;

    QTreeView* View();
    QTreeView* CarrierView();
    QTreeView* SpecialView();
    QTableView* WrongView();

    CashFlowStatementModel* Model() { return model_; }
    CashFlowCarrierModel* CarrierModel() { return carrier_; }
    CashFlowSpecialModel* SpecialModel() { return special_; }
    CashFlowWrongModel* WrongModel() { return wrong_; }

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

    CashFlowStatementModel* model_ {};
    CashFlowCarrierModel* carrier_ {};
    CashFlowSpecialModel* special_ {};
    CashFlowWrongModel* wrong_ {};

    QTimer* cooldown_timer_ { nullptr };
};

#endif // CASHFLOWSTATEMENTDIALOG_H
