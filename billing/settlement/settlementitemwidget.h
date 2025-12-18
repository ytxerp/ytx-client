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

#ifndef SETTLEMENTITEMWIDGET_H
#define SETTLEMENTITEMWIDGET_H

#include <QTableView>
#include <QWidget>

#include "component/using.h"
#include "enum/section.h"
#include "settlement.h"
#include "settlementitemmodel.h"
#include "tree/model/treemodel.h"

namespace Ui {
class SettlementItemWidget;
}

class SettlementItemWidget final : public QWidget {
    Q_OBJECT

public:
    explicit SettlementItemWidget(TreeModel* tree_model_partner, SettlementItemModel* model, Settlement* settlement, bool is_persisted, Section section,
        CUuid& widget_id, CUuid& parent_widget_id, QWidget* parent = nullptr);
    ~SettlementItemWidget();

    QTableView* View() const;
    SettlementItemModel* Model() const { return model_; }

    void InsertSucceeded();
    void RecallSucceeded();

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
    Ui::SettlementItemWidget* ui;

    Settlement* settlement_ {};
    Settlement tmp_settlement_ {};
    SettlementItemModel* model_ {};

    TreeModel* tree_model_partner_ {};
    QJsonObject pending_update_ {};

    const QUuid widget_id_ {};
    const QUuid parent_widget_id_ {};
    const Section section_ {};
    bool is_persisted_ {};
};

#endif // SETTLEMENTITEMWIDGET_H
