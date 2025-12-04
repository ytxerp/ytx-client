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

#ifndef STATEMENTSECONDARYWIDGET_H
#define STATEMENTSECONDARYWIDGET_H

#include <QButtonGroup>
#include <QDateTime>
#include <QTableView>

#include "component/using.h"
#include "statementsecondarymodel.h"

namespace Ui {
class StatementSecondaryWidget;
}

class StatementSecondaryWidget final : public QWidget {
    Q_OBJECT

public:
    StatementSecondaryWidget(StatementSecondaryModel* model, Section section, CUuid& widget_id, CUuid& partner_id, int unit, CDateTime& start, CDateTime& end,
        QWidget* parent = nullptr);
    ~StatementSecondaryWidget() override;

    QTableView* View() const;
    StatementSecondaryModel* Model() const { return model_; }

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
    Ui::StatementSecondaryWidget* ui;
    int unit_ {};
    QDateTime start_ {};
    QDateTime end_ {};
    StatementSecondaryModel* model_ {};

    QButtonGroup* unit_group_ {};
    QTimer* cooldown_timer_ { nullptr };
    const Section section_ {};
    const QUuid widget_id_ {};
    CUuid partner_id_ {};
};

#endif // STATEMENTSECONDARYWIDGET_H
