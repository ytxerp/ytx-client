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

#ifndef STATEMENTENTRYWIDGET_H
#define STATEMENTENTRYWIDGET_H

#include <QButtonGroup>
#include <QDateTime>
#include <QTableView>

#include "component/using.h"
#include "statemententrymodel.h"
#include "tree/model/treemodeli.h"

namespace Ui {
class StatementEntryWidget;
}

class StatementEntryWidget final : public QWidget {
    Q_OBJECT

public:
    StatementEntryWidget(StatementEntryModel* model, EntryHubP* entry_hub_p, TreeModelI* tree_model_i, CUuid& widget_id, CUuid& partner_id, CDateTime& start,
        CDateTime& end, CString& partner_name, CString& company_name, Section section, int unit, QWidget* parent = nullptr);
    ~StatementEntryWidget() override;

    QTableView* View() const;
    StatementEntryModel* Model() const { return model_; }

    void ResetTotal(const QJsonObject& total) { total_ = total; }

private slots:
    void on_pBtnFetch_clicked();
    void on_start_dateChanged(const QDate& date);
    void on_end_dateChanged(const QDate& date);
    void on_pBtnExport_clicked();

    void RUnitGroupClicked(int id);

private:
    void IniUnitGroup();
    void IniConnect();
    void IniUnit(int unit);
    void IniWidget();
    void InitTimer();

private:
    Ui::StatementEntryWidget* ui;
    int unit_ {};
    QDateTime start_ {};
    QDateTime end_ {};
    StatementEntryModel* model_ {};

    QJsonObject total_ {};
    const QString partner_name_ {};
    const QString company_name_ {};

    QButtonGroup* unit_group_ {};
    QTimer* cooldown_timer_ { nullptr };
    const Section section_ {};
    const QUuid widget_id_ {};
    CUuid partner_id_ {};
    EntryHubP* entry_hub_p_ {};
    TreeModelI* tree_model_i_ {};
};

inline const char* kStatementEntryWidget = "StatementEntryWidget";

#endif // STATEMENTENTRYWIDGET_H
