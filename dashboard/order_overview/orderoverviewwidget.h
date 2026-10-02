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

#pragma once

#include <QTableView>
#include <QWidget>

#include "dashboard/order_overview/orderoverviewmodel.h"
#include "enum/section.h"
#include "table/tablefiltermodel.h"
#include "table/tablefilterproxymodel.h"
#include "utils/daterange.h"

namespace Ui {
class OrderOverviewWidget;
}

class OrderOverviewWidget : public QWidget {
    Q_OBJECT

public:
    explicit OrderOverviewWidget(const QStringList& header, const QUuid widget_id, const Section section, QWidget* parent = nullptr);
    ~OrderOverviewWidget() override;

    QTableView* OverviewView() const;
    QTableView* FilterView() const;
    order_overview::Model* OverviewModel() const { return overview_model_; }

private slots:
    void on_pBtnFetch_clicked();
    void on_end_dateChanged(const QDate& date);
    void on_start_dateChanged(const QDate& date);

private:
    void InitWidget();
    void InitModel(const QStringList& header);
    void InitTimer();
    static utils::DateRange DefaultRange()
    {
        const auto today { QDate::currentDate() };
        return { QDate(today.year(), 1, 1), today };
    }

private:
    Ui::OrderOverviewWidget* ui;
    utils::DateRange range_ {};

    order_overview::Model* overview_model_ {};
    TableFilterModel* filter_model_ {};
    TableFilterProxyModel* filter_proxy_ {};

    QTimer* cooldown_timer_ { nullptr };
    const QUuid widget_id_ {};
    const Section section_ {};
};
