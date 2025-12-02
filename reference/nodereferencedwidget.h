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

#ifndef NODEREFERENCEDWIDGET_H
#define NODEREFERENCEDWIDGET_H

#include <QAbstractItemModel>
#include <QDateTime>
#include <QTableView>
#include <QUuid>

#include "component/using.h"

namespace Ui {
class NodeReferencedWidget;
}

class NodeReferencedWidget final : public QWidget {
    Q_OBJECT

signals:
    void SResetModel(const QUuid& node_id, const QDateTime& start, const QDateTime& end);

public:
    NodeReferencedWidget(QAbstractItemModel* model, const QUuid& node_id, CDateTime& start, CDateTime& end, QWidget* parent = nullptr);
    ~NodeReferencedWidget() override;

    QTableView* View() const;
    QAbstractItemModel* Model() const;

private slots:
    void on_pBtnRefresh_clicked();
    void on_start_dateChanged(const QDate& date);
    void on_end_dateChanged(const QDate& date);

private:
    void IniWidget(QAbstractItemModel* model);

private:
    Ui::NodeReferencedWidget* ui;
    QDateTime start_ {};
    QDateTime end_ {};

    const QUuid node_id_ {};
};

#endif // NODEREFERENCEDWIDGET_H
