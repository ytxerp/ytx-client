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

#include "component/using.h"
#include "settlementprimarymodel.h"
#include "utils/daterange.h"

namespace Ui {
class SettlementPrimaryWidget;
}

class SettlementPrimaryWidget final : public QWidget {
    Q_OBJECT

public:
    explicit SettlementPrimaryWidget(settlement::PrimaryModel* model, CUuid& widget_id, Section section, QWidget* parent = nullptr);
    ~SettlementPrimaryWidget() override;

    QTableView* View() const;
    settlement::PrimaryModel* Model() const { return model_; }

signals:
    void SCreateSettlementSecondaryWidget(settlement::PrimaryModel* model);

private slots:
    void on_pBtnFetch_clicked();
    void on_pushButtonDelete_clicked();
    void on_pushButtonInsert_clicked();
    void on_start_dateChanged(const QDate& date);
    void on_end_dateChanged(const QDate& date);

private:
    void InitWidget();
    void InitTimer();
    static utils::DateRange DefaultRange()
    {
        const int year { QDate::currentDate().year() };
        return { QDate(year - 1, 1, 1), QDate(year, 12, 31) };
    }

private:
    Ui::SettlementPrimaryWidget* ui;
    settlement::PrimaryModel* model_ {};
    utils::DateRange range_ {};
    QTimer* cooldown_timer_ { nullptr };

    const Section section_ {};
    const QUuid widget_id_ {};
};
