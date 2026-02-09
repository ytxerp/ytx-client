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

#ifndef INSERTNODEBRANCH_H
#define INSERTNODEBRANCH_H

#include <QButtonGroup>
#include <QDialog>

#include "component/using.h"
#include "tree/itemmodel.h"
#include "tree/node.h"

namespace Ui {
class InsertNodeBranch;
}

class InsertNodeBranch final : public QDialog {
    Q_OBJECT

public:
    InsertNodeBranch(Node* node, ItemModel* unit_model, CString& parent_path, const QSet<QString>& name_set, QWidget* parent = nullptr);
    ~InsertNodeBranch() override;

private slots:
    void RNameEdited(const QString& arg1);

    void on_lineName_editingFinished();
    void on_lineDescription_editingFinished();

    void on_comboUnit_currentIndexChanged(int index);

private:
    void IniDialog(ItemModel* unit_model);
    void IniData(Node* node);
    void IniConnect();

private:
    Ui::InsertNodeBranch* ui;
    Node* node_ {};

    CString parent_path_ {};
    const QSet<QString> name_set_ {};
};

#endif // INSERTNODEBRANCH_H
