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

#ifndef STATEMENTWIDGET_H
#define STATEMENTWIDGET_H

#include <QButtonGroup>
#include <QDateTime>
#include <QTableView>

#include "component/using.h"
#include "statementmodel.h"

namespace Ui {
class StatementWidget;
}

class StatementWidget final : public QWidget {
    Q_OBJECT

signals:
    void SStatementNode(const QUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end);

public:
    StatementWidget(StatementModel* model, Section section, CUuid& widget_id, QWidget* parent = nullptr);
    ~StatementWidget() override;

    QTableView* View() const;
    StatementModel* Model() const { return model_; }

private slots:
    void on_pBtnFetch_clicked();
    void on_tableView_doubleClicked(const QModelIndex& index);
    void on_start_dateChanged(const QDate& date);
    void on_end_dateChanged(const QDate& date);

    void RUnitGroupClicked(int id);

private:
    void IniUnitGroup();
    void IniConnect();
    void IniUnit(int unit);
    void IniWidget();
    void InitTimer();

private:
    Ui::StatementWidget* ui;
    int unit_ {};
    QDateTime start_ {};
    QDateTime end_ {};
    StatementModel* model_ {};
    QTimer* cooldown_timer_ { nullptr };

    const Section section_ {};
    const QUuid widget_id_ {};

    QButtonGroup* unit_group_ {};
};

#endif // STATEMENTWIDGET_H
