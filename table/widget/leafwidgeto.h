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

#ifndef LEAFWIDGETO_H
#define LEAFWIDGETO_H

#include <QButtonGroup>

#include "component/arg/insertnodeargo.h"
#include "entryhub/entryhubo.h"
#include "leafwidget.h"
#include "print/printmanager.h"
#include "table/model/leafmodelo.h"

namespace Ui {
class LeafWidgetO;
}

class LeafWidgetO final : public LeafWidget {
    Q_OBJECT

public:
    LeafWidgetO(CInsertNodeArgO& arg, bool is_insert, const QMap<QString, QString>& print_template, QSharedPointer<PrintManager> print_manager,
        QWidget* parent = nullptr);
    ~LeafWidgetO();

signals:
    // send to TableModelOrder, MainWindow
    void SSyncParty(const QUuid& node_id, int column, const QUuid& value);

    // send to TreeModelOrder
    void SSyncFinished(const QUuid& node_id, bool value);

    // send to its widget
    void SSaveOrder();

    // send to MainWindow
    void SEnableAction(bool finished);

public slots:
    // receive from TableModelOrder
    void RSyncDelta(const QUuid& node_id, double initial_delta, double final_delta, double first_delta, double second_delta, double discount_delta);

public:
    LeafModel* Model() const override { return leaf_model_order_; }
    QTableView* View() const override;

private slots:

    void on_comboParty_currentIndexChanged(int index);
    void on_comboEmployee_currentIndexChanged(int index);

    void on_pBtnFinishOrder_toggled(bool checked);
    // void on_pBtnInsert_clicked();

    void on_dateTimeEdit_dateTimeChanged(const QDateTime& date_time);
    void on_lineDescription_editingFinished();

    void on_pBtnPreview_clicked();
    void on_pBtnPrint_clicked();

    void RRuleGroupClicked(int id);
    void RUnitGroupClicked(int id);

    void on_pBtnSaveOrder_clicked();

private:
    void IniWidget();
    void IniConnect();
    void IniData(const QUuid& party, const QUuid& employee);
    void LockWidgets(bool finished);
    void IniUnit(int unit);
    void IniLeafValue();
    void IniRule(bool rule);
    void IniFinished(bool finished);
    void IniRuleGroup();
    void IniUnitGroup();

    void PreparePrint();

private:
    Ui::LeafWidgetO* ui;
    NodeO* node_ {};
    EntryHubO* sql_ {};
    LeafModelO* leaf_model_order_ {};
    TreeModel* tree_model_partner_ {};
    CSectionConfig& config_ {};
    QButtonGroup* rule_group_ {};
    QButtonGroup* unit_group_ {};

    QSortFilterProxyModel* emodel_ {};
    QSortFilterProxyModel* pmodel_ {};

    bool is_insert_ {};

    const QUuid node_id_ {};
    const int party_unit_ {};

    const QMap<QString, QString>& print_template_ {};
    QSharedPointer<PrintManager> print_manager_ {};
};

inline const char* kLeafWidgetO = "LeafWidgetO";

#endif // LEAFWIDGETO_H
