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

#ifndef TABLEWIDGETFIPT_H
#define TABLEWIDGETFIPT_H

#include <QTableView>

#include "table/model/tablemodel.h"
#include "table/widget/tablewidget.h"

namespace Ui {
class TableWidgetFIPT;
}

class TableWidgetFIPT final : public TableWidget {
    Q_OBJECT

public:
    explicit TableWidgetFIPT(TableModel* model, QWidget* parent = nullptr);
    ~TableWidgetFIPT();

    TableModel* Model() const override { return model_; }
    QTableView* View() const override;

private:
    Ui::TableWidgetFIPT* ui;
    TableModel* model_ {};
};

inline const char* kTableWidgetFIPT = "TableWidgetFIPT";

#endif // TABLEWIDGETFIPT_H
