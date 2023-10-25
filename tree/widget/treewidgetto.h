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

#ifndef TREEWIDGETTO_H
#define TREEWIDGETTO_H

#include "tree/model/treemodel.h"
#include "treewidget.h"

namespace Ui {
class TreeWidgetTO;
}

class TreeWidgetTO final : public TreeWidget {
    Q_OBJECT

public slots:
    void on_start_dateChanged(const QDate& date);
    void on_end_dateChanged(const QDate& date);

public:
    TreeWidgetTO(CString& section, TreeModel* model, const QDateTime& start, const QDateTime& end, QWidget* parent = nullptr);
    ~TreeWidgetTO() override;

    QTreeView* View() const override;

private slots:
    void on_pBtnFetch_clicked();

private:
    Ui::TreeWidgetTO* ui;
    CString section_ {};
    TreeModel* model_ {};

    QDateTime start_ {};
    QDateTime end_ {};
};

#endif // TREEWIDGETTO_H
