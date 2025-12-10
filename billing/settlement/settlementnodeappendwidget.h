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

#ifndef SETTLEMENTNODEAPPENDWIDGET_H
#define SETTLEMENTNODEAPPENDWIDGET_H

#include <QTableView>
#include <QWidget>

#include "component/using.h"
#include "enum/section.h"
#include "settlement.h"
#include "settlementnodeappendmodel.h"
#include "tree/model/treemodel.h"

namespace Ui {
class SettlementNodeAppendWidget;
}

class SettlementNodeAppendWidget final : public QWidget {
    Q_OBJECT

public:
    explicit SettlementNodeAppendWidget(TreeModel* tree_model_partner, SettlementNodeAppendModel* model, const std::shared_ptr<Settlement>& settlement,
        Section section, CUuid& widget_id, CUuid& parent_widget_id, QWidget* parent = nullptr);
    ~SettlementNodeAppendWidget();

    QTableView* View() const;

private slots:
    void on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime);
    void on_lineDescription_textChanged(const QString& arg1);
    void on_comboPartner_currentIndexChanged(int index);

    void on_pBtnRelease_clicked();
    void on_pBtnRecall_clicked();

private:
    void InitWidget();
    void InitData();

private:
    Ui::SettlementNodeAppendWidget* ui;

    const std::shared_ptr<Settlement> settlement_ {};
    SettlementNodeAppendModel* model_ {};

    TreeModel* tree_model_partner_ {};

    const QUuid widget_id_ {};
    const QUuid parent_widget_id_ {};
    const Section section_ {};
    bool is_persisted_ {};
};

#endif // SETTLEMENTNODEAPPENDWIDGET_H
