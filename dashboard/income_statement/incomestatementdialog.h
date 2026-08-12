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
#include <QTreeView>

#include "incomestatementmodel.h"
#include "tree/model/treemodel.h"
#include "utils/daterange.h"

namespace Ui {
class IncomeStatementDialog;
}

class IncomeStatementDialog final : public QDialog {
    Q_OBJECT

public:
    explicit IncomeStatementDialog(CTreeModel* tree_model, income_statement::Model* model, const QUuid& widget_id, QWidget* parent = nullptr);
    ~IncomeStatementDialog() override;

    QTreeView* View();
    income_statement::Model* Model() { return model_; }

private slots:
    void on_dateEditStart_dateChanged(const QDate& date);
    void on_dateEditEnd_dateChanged(const QDate& date);
    void on_pushButtonFetch_clicked();

private:
    void InitDialog();
    void InitTimer();
    static utils::DateRange DefaultRange()
    {
        const auto today { QDate::currentDate() };
        return { QDate(today.year(), today.month(), 1), today };
    }

private:
    Ui::IncomeStatementDialog* ui;

    utils::DateRange range_ {};
    const QUuid widget_id_ {};

    income_statement::Model* model_ {};
    CTreeModel* tree_model_ {};

    QTimer* cooldown_timer_ { nullptr };
};
