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

#ifndef TREEWIDGETO_H
#define TREEWIDGETO_H

#include <QTimer>

#include "tree/model/treemodel.h"
#include "treewidget.h"

namespace Ui {
class TreeWidgetO;
}

class TreeWidgetO final : public TreeWidget {
    Q_OBJECT

public slots:
    void on_start_dateChanged(const QDate& date);
    void on_end_dateChanged(const QDate& date);

public:
    TreeWidgetO(Section section, TreeModel* model, const QDateTime& start, const QDateTime& end, QWidget* parent = nullptr);
    ~TreeWidgetO() override;

    QTreeView* View() const override;

private slots:
    void on_pBtnFetch_clicked();
    void InitTimer();

private:
    Ui::TreeWidgetO* ui;
    Section section_ {};
    TreeModel* model_ {};

    QDateTime start_ {};
    QDateTime end_ {};

    QTimer* cooldown_timer_ { nullptr };
};

#endif // TREEWIDGETO_H
