/*
 * Copyright (C) 2026 YTX
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

#include <QTableView>
#include <QWidget>

#include "component/config.h"
#include "finance/reimbursement/reimbursementrow.h"
#include "finance/reimbursement/tablemodelr.h"

namespace Ui {
class TableWidgetR;
}

class TableWidgetR final : public QWidget {
    Q_OBJECT

public:
    explicit TableWidgetR(reimbursement::TableModelR* model, CSectionConfig& config, const reimbursement::PrimaryRow& row, QUuid widget_id,
        int finance_permission, QWidget* parent = nullptr);
    ~TableWidgetR() override;

    QTableView* View() const;
    reimbursement::TableModelR* Model() const { return model_; }

    void ApproveSucceeded(int version);
    void UnapproveSucceeded(int version);
    void SaveSucceeded(int version);
    void PostSucceeded(int version);
    void UnpostSucceeded(int version);

signals:
    void SInsertPrimaryRow(const reimbursement::PrimaryRow& row);
    void SUpdatePrimaryRow(const reimbursement::PrimaryRow& row);

private slots:
    void on_pushButtonDelete_clicked();
    void on_pushButtonInsert_clicked();
    void on_pushButtonSave_clicked();
    void on_pushButtonApprove_clicked();
    void on_pushButtonPost_clicked();
    void on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime);
    void on_lineEditDescription_textEdited(const QString& arg1);
    void on_pushButtonUnapprove_clicked();
    void on_pushButtonUnpost_clicked();

private:
    void InitWidget();
    void InitData();
    void FetchNode();
    void UpdateButtons();

private:
    Ui::TableWidgetR* ui;
    reimbursement::TableModelR* model_ {};

    reimbursement::PrimaryRow primary_row_ {};
    QJsonObject pending_update_ {};

    const QUuid widget_id_ {};
    const int finance_permission_ {};

    CSectionConfig& config_ {};
};
