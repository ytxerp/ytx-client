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

#ifndef TREEWIDGETF_H
#define TREEWIDGETF_H

#include <QDoubleSpinBox>

#include "component/config.h"
#include "component/info.h"
#include "tree/model/treemodel.h"
#include "treewidget.h"

namespace Ui {
class TreeWidgetF;
}

class TreeWidgetF final : public TreeWidget {
    Q_OBJECT

public slots:
    void RTotalsUpdated() override;

public:
    TreeWidgetF(TreeModel* model, CSectionInfo& info, CGlobalConfig global, CSectionConfig& config, QWidget* parent = nullptr);
    ~TreeWidgetF() override;

    QTreeView* View() const override;
    void UpdateStatus() override;

private:
    void UpdateStaticStatus();
    void UpdateDynamicStatus();

    void UpdateDynamicValue(const QUuid& lhs_node_id, const QUuid& rhs_node_id);
    void UpdateStaticValue(const QUuid& node_id);
    double Operate(double lhs, double rhs, const QString& operation);
    void ResetStatus(QDoubleSpinBox* spin_box, bool& flags);

private:
    Ui::TreeWidgetF* ui;
    TreeModel* model_ {};

    CSectionInfo& info_ {};
    CSectionConfig& section_ {};
    CGlobalConfig global_ {};

    bool dynamic_unit_is_not_default_but_equal_ { false };
    bool static_unit_is_default_ { false };
};

#endif // TREEWIDGETF_H
