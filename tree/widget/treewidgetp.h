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

#ifndef TREEWIDGETP_H
#define TREEWIDGETP_H

#include "tree/model/treemodel.h"
#include "treewidget.h"

namespace Ui {
class TreeWidgetP;
}

class TreeWidgetP final : public TreeWidget {
    Q_OBJECT

public:
    explicit TreeWidgetP(TreeModel* model, QWidget* parent = nullptr);
    ~TreeWidgetP() override;

    QTreeView* View() const override;

private:
    Ui::TreeWidgetP* ui;
    TreeModel* model_ {};
};

#endif // TREEWIDGETP_H
