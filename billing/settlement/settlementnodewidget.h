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

#ifndef SETTLEMENTNODEWIDGET_H
#define SETTLEMENTNODEWIDGET_H

#include <QTableView>
#include <QWidget>

#include "component/using.h"
#include "enum/section.h"
#include "settlement.h"
#include "settlementnodemodel.h"

namespace Ui {
class SettlementNodeWidget;
}

class SettlementNodeWidget final : public QWidget {
    Q_OBJECT

public:
    explicit SettlementNodeWidget(SettlementNodeModel* model, const std::shared_ptr<Settlement>& settlement, bool is_persisted, Section section,
        CUuid& widget_id, QWidget* parent = nullptr);
    ~SettlementNodeWidget();

    QTableView* View() const;

private:
    void InitWidget();

private:
    Ui::SettlementNodeWidget* ui;

    const std::shared_ptr<Settlement> settlement_ {};
    SettlementNodeModel* model_ {};

    const QUuid widget_id_ {};
    const Section section_ {};
    bool is_persisted_ {};
};

#endif // SETTLEMENTNODEWIDGET_H
