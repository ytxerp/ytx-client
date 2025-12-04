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

#ifndef SALEREFERENCEWIDGET_H
#define SALEREFERENCEWIDGET_H

#include <QDateTime>
#include <QTableView>
#include <QUuid>

#include "component/using.h"
#include "enum/section.h"
#include "reference/salereferencemodel.h"

namespace Ui {
class SaleReferenceWidget;
}

class SaleReferenceWidget final : public QWidget {
    Q_OBJECT

public:
    SaleReferenceWidget(SaleReferenceModel* model, Section section, CUuid& widget_id, CUuid& node_id, int node_unit, QWidget* parent = nullptr);
    ~SaleReferenceWidget() override;

    QTableView* View() const;
    SaleReferenceModel* Model() const { return model_; }

private slots:
    void on_pBtnFetch_clicked();
    void on_start_dateChanged(const QDate& date);
    void on_end_dateChanged(const QDate& date);

private:
    void IniWidget();
    void InitTimer();

private:
    Ui::SaleReferenceWidget* ui;
    QDateTime start_ {};
    QDateTime end_ {};
    SaleReferenceModel* model_ {};

    QTimer* cooldown_timer_ { nullptr };

    const QUuid node_id_ {};
    const QUuid widget_id_ {};
    const int node_unit_ {};
    const Section section_ {};
};

#endif // SALEREFERENCEWIDGET_H
