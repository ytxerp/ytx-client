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

#ifndef SETTLEMENTPRIMARYWIDGET_H
#define SETTLEMENTPRIMARYWIDGET_H

#include <QDateTime>
#include <QTableView>

#include "component/using.h"
#include "settlementprimarymodel.h"

namespace Ui {
class SettlementPrimaryWidget;
}

class SettlementPrimaryWidget final : public QWidget {
    Q_OBJECT

public:
    explicit SettlementPrimaryWidget(SettlementPrimaryModel* model, CUuid& widget_id, Section section, QWidget* parent = nullptr);
    ~SettlementPrimaryWidget() override;

    QTableView* View() const;
    SettlementPrimaryModel* Model() const { return model_; }
    QUuid WidgetId() const { return widget_id_; }

private slots:
    void on_pBtnFetch_clicked();
    void on_start_dateChanged(const QDate& date);
    void on_end_dateChanged(const QDate& date);

private:
    void IniWidget();
    void InitTimer();

private:
    Ui::SettlementPrimaryWidget* ui;
    SettlementPrimaryModel* model_ {};
    QDateTime start_ {};
    QDateTime end_ {};
    QTimer* cooldown_timer_ { nullptr };

    const Section section_ {};
    const QUuid widget_id_ {};
};

inline const char* kSettlementPrimaryWidget = "SettlementPrimaryWidget";

#endif // SETTLEMENTPRIMARYWIDGET_H
