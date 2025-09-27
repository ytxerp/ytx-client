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

#ifndef LEAFWIDGETFIPT_H
#define LEAFWIDGETFIPT_H

#include <QTableView>

#include "table/model/leafmodel.h"
#include "table/widget/leafwidget.h"

namespace Ui {
class LeafWidgetFIPT;
}

class LeafWidgetFIPT final : public LeafWidget {
    Q_OBJECT

public:
    explicit LeafWidgetFIPT(LeafModel* model, QWidget* parent = nullptr);
    ~LeafWidgetFIPT();

    LeafModel* Model() const override { return model_; }
    QTableView* View() const override;

private:
    Ui::LeafWidgetFIPT* ui;
    LeafModel* model_ {};
};

inline const char* kLeafWidgetFIPT = "LeafWidgetFIPT";

#endif // LEAFWIDGETFIPT_H
