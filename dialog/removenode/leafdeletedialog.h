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

#ifndef LEAFDELETEDIALOG_H
#define LEAFDELETEDIALOG_H

#include <QButtonGroup>
#include <QDialog>

#include "tree/model/treemodel.h"

namespace Ui {
class LeafDeleteDialog;
}

class LeafDeleteDialog final : public QDialog {
    Q_OBJECT

signals:
    void SDeleteNode(const QUuid& node_id);

public:
    LeafDeleteDialog(TreeModel* model, CSectionInfo& info, CJsonObject& obj, const QUuid& node_id, NodeUnit unit, QWidget* parent = nullptr);
    ~LeafDeleteDialog() override;

private slots:
    void on_pBtnOk_clicked();
    void RcomboBoxCurrentIndexChanged(int index);
    void RButtonGroup(int id);
    void RReplaceResult(bool result);

private:
    void IniData(Section section);
    void IniConnect();
    void IniOptionGroup();
    void InitCheckBoxGroup();

private:
    Ui::LeafDeleteDialog* ui;
    QButtonGroup* option_group_ {};

    const QUuid node_id_ {};
    const NodeUnit node_unit_ {};

    const bool inside_ref_ {};
    const bool inventory_int_ref_ {};
    const bool inventory_ext_ref_ {};
    const bool partner_ref_ {};
    const bool employee_ref_ {};
    const bool settlement_ref_ {};

    TreeModel* model_ {};
    CSectionInfo& info_ {};
};

#endif // LEAFDELETEDIALOG_H
