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

#ifndef TREEWIDGETI_H
#define TREEWIDGETI_H

#include <QDoubleSpinBox>

#include "component/config.h"
#include "treewidget.h"
#include "tree/model/treemodel.h"

namespace Ui {
class TreeWidgetI;
}

class TreeWidgetI final : public TreeWidget {
    Q_OBJECT

public slots:
    void RSyncStatusValue() override;

public:
    TreeWidgetI(TreeModel* model, CSectionConfig& config, QWidget* parent = nullptr);
    ~TreeWidgetI() override;

    QTreeView* View() const override;
    void UpdateStatus() override;

private:
    void UpdateStaticStatus();
    void UpdateDynamicStatus();

    void UpdateDynamicValue(const QUuid& lhs_node_id, const QUuid& rhs_node_id);
    void UpdateStaticValue(const QUuid& node_id);
    double Operate(double lhs, double rhs, const QString& operation);

private:
    Ui::TreeWidgetI* ui;
    TreeModel* model_ {};

    CSectionConfig& config_ {};
};

#endif // TREEWIDGETI_H
