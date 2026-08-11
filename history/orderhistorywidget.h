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

#include <QDateTime>
#include <QTableView>
#include <QUuid>

#include "component/using.h"
#include "enum/nodeenum.h"
#include "enum/section.h"
#include "ordermodel.h"
#include "utils/daterange.h"

namespace Ui {
class OrderHistoryWidget;
}

class OrderHistoryWidget final : public QWidget {
    Q_OBJECT

public:
    OrderHistoryWidget(history::OrderModel* model, Section section, CUuid& widget_id, CUuid& node_id, NodeUnit node_unit, QWidget* parent = nullptr);
    ~OrderHistoryWidget() override;

    QTableView* View() const;
    history::OrderModel* Model() const { return model_; }

private slots:
    void on_pBtnFetch_clicked();
    void on_start_dateChanged(const QDate& date);
    void on_end_dateChanged(const QDate& date);

private:
    void InitWidget();
    void InitTimer();
    static utils::DateRange DefaultRange()
    {
        const auto today { QDate::currentDate() };
        const int year { today.year() };

        return { QDate(year - 1, 1, 1), today };
    }

private:
    Ui::OrderHistoryWidget* ui;
    utils::DateRange range_ {};
    history::OrderModel* model_ {};

    QTimer* cooldown_timer_ { nullptr };

    const QUuid node_id_ {};
    const QUuid widget_id_ {};
    const NodeUnit node_unit_ {};
    const Section section_ {};
};
