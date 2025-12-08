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

#include <QWidget>

#include "billing/settlement/settlement.h"
#include "component/using.h"

namespace Ui {
class SettlementNodeWidget;
}

class SettlementNodeWidget final : public QWidget {
    Q_OBJECT

public:
    explicit SettlementNodeWidget(CUuid& partner_id, CUuid& settlement_id, std::shared_ptr<SettlementNodeList>& list, int status, QWidget* parent = nullptr);
    ~SettlementNodeWidget();

private:
    void IniWidget();

private:
    Ui::SettlementNodeWidget* ui;

    QUuid partner_id_ {};
    const QUuid settlement_id_ {};

    std::shared_ptr<SettlementNodeList> list_ {};
};

#endif // SETTLEMENTNODEWIDGET_H
