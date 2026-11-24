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

#include <QButtonGroup>

#include "component/arg/orderwidgetarg.h"
#include "table/model/tablemodelo.h"
#include "tablewidget.h"

namespace Ui {
class TableWidgetO;
}

class TableWidgetO final : public TableWidget {
    Q_OBJECT

public:
    explicit TableWidgetO(COrderWidgetArg& arg, NodeO* node, int permission, QWidget* parent = nullptr);
    ~TableWidgetO() override;

signals:
    // send to MainWindow
    void SSyncPartner(QUuid node_id, QUuid partner_id);

public slots:
    // receive from TableModelOrder
    void RDataChanged();
    void RSyncDelta(QUuid node_id, const entry::Delta& delta);
    void on_pBtnSave_clicked();

public:
    TableModel* Model() const override { return table_model_; }
    QTableView* View() const override;

    void SyncSucceeded(int version);
    void PermissionDenied();

    void StatusSucceeded(int status, int version);
    bool HasPendingUpdate() const;

private slots:

    void on_comboPartner_currentIndexChanged(int index);
    void on_comboEmployee_currentIndexChanged(int index);
    void on_comboTemplate_currentIndexChanged(int index);

    void on_dateTimeEdit_dateTimeChanged(const QDateTime& date_time);
    void on_lineDescription_textChanged(const QString& arg1);

    void on_pBtnPreview_clicked();
    void on_pBtnPrint_clicked();

    void RRuleGroupClicked(int id);
    void RUnitGroupClicked(int id);

    void on_pBtnUnrelease_clicked();
    void on_pBtnRelease_clicked();

private:
    void InitWidget(QSortFilterProxyModel* pmodel, QSortFilterProxyModel* emodel);
    void InitConnect();
    void InitData(const NodeO* node);
    void UpdateButtons(OrderStatus value);
    void UpdateSaveReleaseButtons();
    void InitRuleGroup();
    void InitUnitGroup();
    void InitUiValue();

    bool PreparePrint();

    void BuildNodeInsert(QJsonObject& message);
    void BuildNodeUpdate(QJsonObject& message);
    QJsonObject BuildStatusMessage() const;

    bool ValidatePartner() const;
    bool ValidateSyncState() const;
    bool ValidateUnitPrice() const;

private:
    Ui::TableWidgetO* ui;
    NodeO* node_ {};
    NodeO original_node_ {};

    TableModelO* table_model_ {};
    CSectionConfig& section_config_ {};
    QButtonGroup* rule_group_ {};
    QButtonGroup* unit_group_ {};

    QJsonObject pending_update_ {};

    const QUuid node_id_ {};
    const Section section_ {};
    const int permission_ {};
};

inline const char* kTableWidgetO = "TableWidgetO";
