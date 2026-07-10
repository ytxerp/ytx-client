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

#ifndef PERIODCLOSEDIALOG_H
#define PERIODCLOSEDIALOG_H

#include <QDialog>

#include "periodclosemodel.h"
#include "tree/model/treemodel.h"

namespace Ui {
class PeriodCloseDialog;
}

class PeriodCloseDialog : public QDialog {
    Q_OBJECT

public:
    explicit PeriodCloseDialog(Section section, CTreeModel* tree_model, period_close::Model* table_model, QWidget* parent = nullptr);
    ~PeriodCloseDialog() override;

    QTableView* View();

private slots:
    void on_pushButtonPreview_clicked();
    void on_pushButtonCommit_clicked();

private:
    void InitDialog();
    void ConstructEntry(const QSet<Node*>& closing_leaf_node, const Node* summary_node);
    void ResetState();

    QJsonArray BuildUuidArray(const QSet<Node*>& set);

private:
    Ui::PeriodCloseDialog* ui;
    const Section section_;

    CTreeModel* tree_model_ {};
    period_close::Model* table_model_ {};

    QUuid summary_node_id_ {};
    QSet<Node*> closing_leaf_node_ {};

    QList<Entry*> entry_list_ {};
};

#endif // PERIODCLOSEDIALOG_H
