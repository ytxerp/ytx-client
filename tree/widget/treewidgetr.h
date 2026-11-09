/*
 * Copyright (C) 2026 YTX
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

#include "tree/model/treemodel.h"
#include "treewidget.h"
#include "utils/daterange.h"

namespace Ui {
class TreeWidgetR;
}

class TreeWidgetR final : public TreeWidget {
    Q_OBJECT

public:
    explicit TreeWidgetR(Section section, TreeModel* model, QWidget* parent = nullptr);
    ~TreeWidgetR() override;

    QTreeView* View() const override;
    void Reset() override;

private slots:
    void on_start_dateChanged(const QDate& date);
    void on_end_dateChanged(const QDate& date);
    void on_pushButtonFetch_clicked();

private:
    void InitWidget();
    void InitTimer();

    static utils::DateRange DefaultRange()
    {
        const QDate today { QDate::currentDate() };
        const QDate start { today.addMonths(-3) };

        return { QDate(start.year(), start.month(), 1), QDate(today.year(), today.month(), today.daysInMonth()) };
    }

private:
    Ui::TreeWidgetR* ui;
    Section section_ {};
    TreeModel* model_ {};

    utils::DateRange range_ {};
    QTimer* cooldown_timer_ { nullptr };
};

inline const char* kTreeWidgetR = "TreeWidgetR";
