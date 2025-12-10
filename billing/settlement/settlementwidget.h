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
    void SSettlementNodeAppend(const QUuid& parent_widget_id, const std::shared_ptr<Settlement>& settlement, std::shared_ptr<SettlementNodeList>& list_cache);
    void SSettlementNodeEdit(const QUuid& parent_widget_id, const std::shared_ptr<Settlement>& settlement);

public:
    explicit SettlementWidget(SettlementModel* model, Section section, CUuid& widget_id, QWidget* parent = nullptr);
    ~SettlementWidget() override;

    QTableView* View() const;
    SettlementModel* Model() const { return model_; }
    void ResetUnsettledOrder(const QJsonArray& array);

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
    void InitSharedPtr();

private:
    Ui::SettlementWidget* ui;
    SettlementModel* model_ {};
    QDateTime start_ {};
    QDateTime end_ {};
    QTimer* cooldown_timer_ { nullptr };

    std::shared_ptr<SettlementNodeList> unsettled_order_ {};

    const Section section_ {};
    const QUuid widget_id_ {};
};

#endif // SETTLEMENTWIDGET_H
