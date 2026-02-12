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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QComboBox>
#include <QDialog>

#include "component/config.h"
#include "component/info.h"
#include "component/using.h"
#include "tree/itemmodel.h"
#include "tree/model/treemodel.h"

namespace Ui {
class Preferences;
}

class Preferences final : public QDialog {
    Q_OBJECT

public:
    Preferences(CTreeModel* model, CSectionInfo& info, CAppConfig& app, CSharedConfig& shared, CSectionConfig& section, QWidget* parent = nullptr);
    ~Preferences() override;

signals:
    void SUpdateConfig(const AppConfig& app, const SharedConfig& shared, const SectionConfig& section);

private slots:
    void on_pBtnApply_clicked();
    void on_pBtnDocumentDir_clicked();
    void on_pBtnResetDocumentDir_clicked();

    void on_comboDefaultUnit_currentIndexChanged(int index);

    void on_comboOperation_currentIndexChanged(int index);
    void on_comboStatic_currentIndexChanged(int index);
    void on_comboDynamicLhs_currentIndexChanged(int index);
    void on_comboDynamicRhs_currentIndexChanged(int index);

    void on_spinAmountDecimal_editingFinished();
    void on_spinRateDecimal_editingFinished();
    void on_lineStatic_editingFinished();
    void on_lineDynamic_editingFinished();

    void on_comboTheme_currentIndexChanged(int index);
    void on_comboLanguage_currentIndexChanged(int index);
    void on_comboDateFormat_currentIndexChanged(int index);
    void on_comboSeparator_currentIndexChanged(int index);
    void on_lineCompanyName_editingFinished();

    void on_comboPrinter_currentIndexChanged(int index);

    void on_spinQuantityDecimal_editingFinished();

    void on_chkBoxDeleteConfirm_checkStateChanged(const Qt::CheckState& arg1);

private:
    void IniDialog(ItemModel* unit_model, Section section);
    void IniCombo(QComboBox* combo, CStringList& list);

    void IniConnect();
    void IniStringList();
    void ResizeLine(QLineEdit* line, CString& text);
    void IniText(Section section);

    void IniData(Section section);
    void IniDataCombo(QComboBox* combo, const QUuid& value);
    void IniDataCombo(QComboBox* combo, int value);
    void IniDataCombo(QComboBox* combo, CString& string);

private:
    Ui::Preferences* ui;

    QStringList theme_list_ {};
    QStringList language_list_ {};
    QStringList separator_list_ {};
    QStringList operation_list_ {};
    QStringList date_format_list_ {};

    ItemModel* leaf_path_branch_path_model_ {};

    AppConfig app_ {};
    SectionConfig section_ {};
    SharedConfig shared_ {};
    CTreeModel* model_ {};

    bool is_enable_status_ { false };
};

#endif // PREFERENCES_H
