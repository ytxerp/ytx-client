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

#include <QDateTime>
#include <QTableView>
#include <QWidget>

#include "finance/reimbursement/treemodelr.h"
#include "utils/daterange.h"

namespace Ui {
class TreeWidgetR;
}

class TreeWidgetR final : public QWidget {
    Q_OBJECT

public:
    explicit TreeWidgetR(QUuid widget_id, const QStringList& header, QWidget* parent = nullptr);
    ~TreeWidgetR() override;

    QTableView* View() const;
    reimbursement::TreeModelR* Model() { return model_; }

signals:
    void SCreateReimbursementSecondaryWidget(reimbursement::TreeModelR* model);

private slots:
    void on_pushButtonFetch_clicked();
    void on_start_dateChanged(const QDate& date);
    void on_end_dateChanged(const QDate& date);
    void on_pushButtonDelete_clicked();
    void on_pushButtonInsert_clicked();

private:
    void InitWidget();
    void InitTimer();
    void InitModel(const QStringList& header);

    static utils::DateRange DefaultRange()
    {
        const QDate today { QDate::currentDate() };
        const QDate start { today.addMonths(-3) };

        return { QDate(start.year(), start.month(), 1), QDate(today.year(), today.month(), today.daysInMonth()) };
    }

private:
    Ui::TreeWidgetR* ui;

    reimbursement::TreeModelR* model_ {};
    utils::DateRange range_ {};
    QTimer* cooldown_timer_ { nullptr };

    const QUuid widget_id_ {};
};
