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

#include "component/arg/nodeopargo.h"
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
    LeafWidgetO(CNodeOpArgO& arg, QWidget* parent = nullptr);
    ~LeafWidgetO();

signals:
    // send to TableModelOrder, MainWindow
    void SSyncPartner(const QUuid& node_id, const QUuid& value);

    // send to TreeModelOrder
    void SSyncStatus(const QUuid& node_id, bool value);

    // send to its widget
    void SInsertOrder();

public slots:
    // receive from TableModelOrder
    void RSyncDelta(const QUuid& node_id, double initial_delta, double final_delta, double count_delta, double measure_delta, double discount_delta);

public:
    LeafModel* Model() const override { return leaf_model_order_; }
    QTableView* View() const override;

    void on_pBtnSave_clicked();
    bool HasUnsavedData() const;

private slots:

    void on_comboPartner_currentIndexChanged(int index);
    void on_comboEmployee_currentIndexChanged(int index);

    void on_dateTimeEdit_dateTimeChanged(const QDateTime& date_time);
    void on_lineDescription_textChanged(const QString& arg1);

    void on_pBtnPreview_clicked();
    void on_pBtnPrint_clicked();

    void RRuleGroupClicked(int id);
    void RUnitGroupClicked(int id);

    void on_pBtnStatus_toggled(bool checked);

private:
    void IniWidget();
    void IniConnect();
    void IniData(const QUuid& partner, const QUuid& employee);
    void LockWidgets(bool released);
    void IniUnit(int unit);
    void IniUiValue();
    void IniRule(bool rule);
    void IniStatus(bool released);
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

    bool is_new_ {};

    QJsonObject node_cache_ {};
    QJsonObject node_delta_ {};

    double initial_delta_ {};
    double final_delta_ {};
    double count_delta_ {};
    double measure_delta_ {};
    double discount_delta_ {};

    const QUuid node_id_ {};
    const int partner_unit_ {};
    const Section section_ {};

    const QMap<QString, QString>& print_template_ {};
    PrintManager print_manager_;
};

inline const char* kLeafWidgetO = "LeafWidgetO";

#endif // LEAFWIDGETO_H
