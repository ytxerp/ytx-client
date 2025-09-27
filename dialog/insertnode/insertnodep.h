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

#ifndef INSERTNODEP_H
#define INSERTNODEP_H

#include <QButtonGroup>
#include <QDialog>

#include "component/arg/insertnodeargfipt.h"
#include "component/using.h"

namespace Ui {
class InsertNodeP;
}

class InsertNodeP final : public QDialog {
    Q_OBJECT

public:
    InsertNodeP(CInsertNodeArgFIPT& arg, QWidget* parent = nullptr);
    ~InsertNodeP();

private slots:
    void RNameEdited(const QString& arg1);
    void RKindGroupClicked(int id);

    void on_lineEditName_editingFinished();
    void on_lineEditCode_editingFinished();
    void on_lineEditDescription_editingFinished();
    void on_spinPaymentPeriod_editingFinished();

    void on_comboUnit_currentIndexChanged(int index);

    void on_plainTextEdit_textChanged();

private:
    void IniDialog(ItemModel* unit_model);
    void IniConnect();
    void IniData();
    void IniKindGroup();

private:
    Ui::InsertNodeP* ui;
    NodeP* node_ {};
    QButtonGroup* kind_group_ {};

    CString& parent_path_ {};
    CStringList& name_list_ {};
};

#endif // INSERTNODEP_H
