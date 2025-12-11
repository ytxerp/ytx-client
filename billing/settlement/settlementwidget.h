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

#ifndef SETTLEMENTWIDGET_H
#define SETTLEMENTWIDGET_H

#include <QDateTime>
#include <QTableView>

#include "component/using.h"
#include "settlementmodel.h"

namespace Ui {
class SettlementWidget;
}

class SettlementWidget final : public QWidget {
    Q_OBJECT

signals:
    void SSettlementNode(const QUuid& parent_widget_id, Settlement* settlement, bool is_persisted);

public:
    explicit SettlementWidget(SettlementModel* model, Section section, CUuid& widget_id, QWidget* parent = nullptr);
    ~SettlementWidget() override;

    QTableView* View() const;
    SettlementModel* Model() const { return model_; }

private slots:
    void on_pBtnFetch_clicked();
    void on_start_dateChanged(const QDate& date);
    void on_end_dateChanged(const QDate& date);
    void on_pBtnAppend_clicked();
    void on_pBtnRemove_clicked();
    void on_tableView_doubleClicked(const QModelIndex& index);

private:
    void IniWidget();
    void InitTimer();

private:
    Ui::SettlementWidget* ui;
    SettlementModel* model_ {};
    QDateTime start_ {};
    QDateTime end_ {};
    QTimer* cooldown_timer_ { nullptr };

    const Section section_ {};
    const QUuid widget_id_ {};
};

#endif // SETTLEMENTWIDGET_H
