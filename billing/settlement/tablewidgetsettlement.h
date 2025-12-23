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

#ifndef TABLEWIDGETSETTLEMENT_H
#define TABLEWIDGETSETTLEMENT_H

#include <QTableView>
#include <QWidget>

#include "component/using.h"
#include "enum/section.h"
#include "settlement.h"
#include "tablemodelsettlement.h"
#include "tree/model/treemodel.h"

namespace Ui {
class TableWidgetSettlement;
}

class TableWidgetSettlement final : public QWidget {
    Q_OBJECT

signals:
    void SUpdatePartner(const QUuid& widget_id, const QUuid& partner_id);

public:
    explicit TableWidgetSettlement(TreeModel* tree_model_partner, TableModelSettlement* model, Settlement* settlement, bool is_persisted, Section section,
        CUuid& widget_id, CUuid& parent_widget_id, QWidget* parent = nullptr);
    ~TableWidgetSettlement();

    QTableView* View() const;
    TableModelSettlement* Model() const { return model_; }

    void InsertSucceeded(int version);
    void RecallSucceeded(int version);
    void UpdateSucceeded(int version);

public slots:
    void RSyncAmount(double amount);

private slots:
    void on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime);
    void on_lineDescription_textChanged(const QString& arg1);
    void on_comboPartner_currentIndexChanged(int index);

    void on_pBtnRelease_clicked();
    void on_pBtnRecall_clicked();

private:
    void InitWidget();
    void InitData();
    void FetchNode();
    void HideWidget(bool is_released);

private:
    Ui::TableWidgetSettlement* ui;

    Settlement* settlement_ {};
    Settlement tmp_settlement_ {};
    TableModelSettlement* model_ {};

    TreeModel* tree_model_partner_ {};
    QJsonObject pending_update_ {};

    const QUuid widget_id_ {};
    const QUuid parent_widget_id_ {};
    const Section section_ {};
    bool is_persisted_ {};
};

#endif // TABLEWIDGETSETTLEMENT_H
