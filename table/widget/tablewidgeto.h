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

#ifndef TABLEWIDGETO_H
#define TABLEWIDGETO_H

#include <QButtonGroup>

#include "component/arg/orderwidgetarg.h"
#include "print/printmanager.h"
#include "table/model/tablemodelo.h"
#include "tablewidget.h"

namespace Ui {
class TableWidgetO;
}

class TableWidgetO final : public TableWidget {
    Q_OBJECT

public:
    TableWidgetO(COrderWidgetArg& arg, QWidget* parent = nullptr);
    ~TableWidgetO();

signals:
    // send to MainWindow
    void SSyncPartner(const QUuid& node_id, const QUuid& value);

    // send to TreeModelOrder
    void SNodeStatus(const QUuid& node_id, NodeStatus value);

    // send to its lambda
    void SInsertOrder();

public slots:
    // receive from TableModelOrder
    void RSyncDeltaOrder(const QUuid& node_id, double initial_delta, double final_delta, double count_delta, double measure_delta, double discount_delta);

public:
    TableModel* Model() const override { return table_model_order_; }
    QTableView* View() const override;

    bool HasUnsavedData() const;
    void SaveOrder();

private slots:

    void on_comboPartner_currentIndexChanged(int index);
    void on_comboEmployee_currentIndexChanged(int index);

    void on_dateTimeEdit_dateTimeChanged(const QDateTime& date_time);
    void on_lineDescription_textChanged(const QString& arg1);

    void on_pBtnPreview_clicked();
    void on_pBtnPrint_clicked();

    void RRuleGroupClicked(int id);
    void RUnitGroupClicked(int id);

    void on_pBtnSave_clicked();
    void on_pBtnRecall_clicked();
    void on_pBtnRelease_clicked();

private:
    void IniWidget();
    void IniConnect();
    void IniData(const QUuid& partner, const QUuid& employee);
    void LockWidgets(NodeStatus value);
    void IniUnit(int unit);
    void IniUiValue();
    void IniRule(bool rule);
    void IniRuleGroup();
    void IniUnitGroup();

    void PreparePrint();

    QJsonObject BuildOrderCache();
    QJsonObject BuildPartnerDelta();

    void BuildNodeInsert(QJsonObject& order_cache);
    void BuildNodeUpdate(QJsonObject& order_cache);
    void ResetCache();

    bool HasNodeDelta() const;

private:
    Ui::TableWidgetO* ui;
    NodeO* node_ {};
    NodeO tmp_node_ {};

    TableModelO* table_model_order_ {};
    TreeModel* tree_model_partner_ {};
    CSectionConfig& config_ {};
    QButtonGroup* rule_group_ {};
    QButtonGroup* unit_group_ {};

    bool is_new_ {};

    QJsonObject node_cache_ {};

    const QUuid node_id_ {};
    const Section section_ {};

    const QMap<QString, QString>& print_template_ {};
    PrintManager print_manager_;
};

inline const char* kTableWidgetO = "TableWidgetO";

#endif // TABLEWIDGETO_H
