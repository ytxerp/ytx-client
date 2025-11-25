#include "preferences.h"

#include <QCompleter>
#include <QDir>
#include <QFileDialog>
#include <QPrinterInfo>
#include <QStandardPaths>

#include "component/constant.h"
#include "component/signalblocker.h"
#include "ui_preferences.h"

Preferences::Preferences(CTreeModel* model, CSectionInfo& info, AppConfig app, SharedConfig shared, SectionConfig section, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Preferences)
    , app_ { app }
    , section_ { section }
    , shared_ { shared }
    , model_ { model }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniStringList();
    IniDialog(info.unit_model, info.section);
    IniConnect();

    IniData(info.section);
    IniText(info.section);
}

Preferences::~Preferences() { delete ui; }

void Preferences::IniDialog(ItemModel* unit_model, Section section)
{
    ui->listWidget->setCurrentRow(0);
    ui->stackedWidget->setCurrentIndex(0);
    ui->pBtnOk->setDefault(true);

    this->setWindowTitle(tr("Preferences"));
    this->setFixedSize(800, 600);

    IniCombo(ui->comboDateFormat, date_format_list_);
    IniCombo(ui->comboLanguage, language_list_);
    IniCombo(ui->comboSeparator, separator_list_);
    IniCombo(ui->comboTheme, theme_list_);

    ui->comboDefaultUnit->setModel(unit_model);

    is_enable_status_ = { section == Section::kFinance || section == Section::kInventory || section == Section::kTask };
    ui->lineStatic->setEnabled(is_enable_status_);
    ui->comboStatic->setEnabled(is_enable_status_);
    ui->lineDynamic->setEnabled(is_enable_status_);
    ui->comboDynamicLhs->setEnabled(is_enable_status_);
    ui->comboDynamicRhs->setEnabled(is_enable_status_);
    ui->comboOperation->setEnabled(is_enable_status_);

    if (section == Section::kPartner) {
        ui->spinQuantityDecimal->setHidden(true);
        ui->labelQuantityDecimal->setHidden(true);
    }

    ui->comboPrinter->addItem(QString());
    const auto& printers { QPrinterInfo::availablePrinters() };

    for (const QPrinterInfo& printer : printers) {
        ui->comboPrinter->addItem(printer.printerName());
    }
}

void Preferences::IniCombo(QComboBox* combo, CStringList& list) { combo->addItems(list); }

void Preferences::IniConnect() { connect(ui->pBtnOk, &QPushButton::clicked, this, &Preferences::on_pBtnApply_clicked); }

void Preferences::IniData(Section section)
{
    IniDataCombo(ui->comboTheme, app_.theme);
    IniDataCombo(ui->comboLanguage, app_.language);
    IniDataCombo(ui->comboSeparator, app_.separator);
    IniDataCombo(ui->comboPrinter, app_.printer);

    ui->lineCompanyName->setText(app_.company_name);

    IniDataCombo(ui->comboDateFormat, section_.date_format);
    ui->spinAmountDecimal->setValue(section_.amount_decimal);
    ui->spinRateDecimal->setValue(section_.rate_decimal);
    ui->spinQuantityDecimal->setValue(section_.quantity_decimal);

    IniDataCombo(ui->comboDefaultUnit, shared_.default_unit);
    ui->pBtnDocumentDir->setText(shared_.document_dir);

    if (is_enable_status_) {
        leaf_path_branch_path_model_ = new ItemModel(this);
        model_->LeafPathBranchPathModel(leaf_path_branch_path_model_);
        leaf_path_branch_path_model_->sort(0);

        ui->comboStatic->setModel(leaf_path_branch_path_model_);
        ui->comboDynamicLhs->setModel(leaf_path_branch_path_model_);
        ui->comboDynamicRhs->setModel(leaf_path_branch_path_model_);

        IniCombo(ui->comboOperation, operation_list_);

        ui->lineStatic->setText(section_.static_label);
        IniDataCombo(ui->comboStatic, section_.static_node);
        ui->lineDynamic->setText(section_.dynamic_label);
        IniDataCombo(ui->comboDynamicLhs, section_.dynamic_node_lhs);
        IniDataCombo(ui->comboOperation, section_.operation);
        IniDataCombo(ui->comboDynamicRhs, section_.dynamic_node_rhs);
    }

    ui->spinAmountDecimal->setMaximum(kMaxNumericScale_4);
    ui->spinRateDecimal->setMaximum(kMaxNumericScale_8);
    ui->spinQuantityDecimal->setMaximum(section == Section::kFinance ? kMaxNumericScale_4 : kMaxNumericScale_8);

    ResizeLine(ui->lineStatic, section_.static_label);
    ResizeLine(ui->lineDynamic, section_.dynamic_label);
}

void Preferences::IniDataCombo(QComboBox* combo, const QUuid& value)
{
    int item_index { combo->findData(value) };
    combo->setCurrentIndex(item_index);
}

void Preferences::IniDataCombo(QComboBox* combo, int value)
{
    int item_index { combo->findData(value) };
    combo->setCurrentIndex(item_index);
}

void Preferences::IniDataCombo(QComboBox* combo, CString& string)
{
    int item_index { combo->findText(string) };
    combo->setCurrentIndex(item_index);
}

void Preferences::IniStringList()
{
    language_list_.emplaceBack(kEnUS);
    language_list_.emplaceBack(kZhCN);

    separator_list_.emplaceBack(kDash);
    separator_list_.emplaceBack(kColon);
    separator_list_.emplaceBack(kSlash);

    theme_list_.emplaceBack(kSolarizedDark);

    operation_list_.emplaceBack(kPlus);
    operation_list_.emplaceBack(kMinux);

    date_format_list_.emplaceBack(kDateTimeFST);
    date_format_list_.emplaceBack(kDateFST);
}

void Preferences::on_pBtnApply_clicked() { emit SUpdateConfig(app_, shared_, section_); }

void Preferences::on_pBtnDocumentDir_clicked()
{
    const QString base_path { QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) };
    const QString current_relative { ui->pBtnDocumentDir->text() };
    const QString current_absolute { QDir(base_path).filePath(current_relative) };

    QDir().mkpath(current_absolute);

    const QString selected_path { QFileDialog::getExistingDirectory(this, tr("Select Directory"), current_absolute) };
    if (!selected_path.isEmpty()) {
        const QString relative_path { (selected_path == base_path) ? QString() : QDir(base_path).relativeFilePath(selected_path) };
        shared_.document_dir = relative_path;
        ui->pBtnDocumentDir->setText(relative_path);
    }
}

void Preferences::on_pBtnResetDocumentDir_clicked()
{
    shared_.document_dir = QString();
    ui->pBtnDocumentDir->setText(QString());
}

void Preferences::ResizeLine(QLineEdit* line, CString& text) { line->setMinimumWidth(QFontMetrics(line->font()).horizontalAdvance(text) + 8); }

void Preferences::IniText(Section section)
{
    switch (section) {
    case Section::kFinance:
        ui->labelRateDecimal->setText(tr("FXRate Decimal"));
        ui->labelDefaultUnit->setText(tr("Base Currency"));
        break;
    default:
        break;
    }
}

void Preferences::on_comboDefaultUnit_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    shared_.default_unit = ui->comboDefaultUnit->currentData().toInt();
}

void Preferences::on_comboStatic_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    section_.static_node = ui->comboStatic->currentData().toUuid();
}

void Preferences::on_comboDynamicLhs_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    section_.dynamic_node_lhs = ui->comboDynamicLhs->currentData().toUuid();
}

void Preferences::on_comboDynamicRhs_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    section_.dynamic_node_rhs = ui->comboDynamicRhs->currentData().toUuid();
}

void Preferences::on_spinAmountDecimal_editingFinished() { section_.amount_decimal = ui->spinAmountDecimal->value(); }

void Preferences::on_lineStatic_editingFinished()
{
    section_.static_label = ui->lineStatic->text();
    ResizeLine(ui->lineStatic, section_.static_label);
}

void Preferences::on_lineDynamic_editingFinished()
{
    section_.dynamic_label = ui->lineDynamic->text();
    ResizeLine(ui->lineDynamic, section_.dynamic_label);
}

void Preferences::on_spinRateDecimal_editingFinished() { section_.rate_decimal = ui->spinRateDecimal->value(); }

void Preferences::on_comboTheme_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    app_.theme = ui->comboTheme->currentText();
}

void Preferences::on_comboLanguage_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    app_.language = ui->comboLanguage->currentText();
}

void Preferences::on_comboDateFormat_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    section_.date_format = ui->comboDateFormat->currentText();
}

void Preferences::on_comboSeparator_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    app_.separator = ui->comboSeparator->currentText();
}

void Preferences::on_comboOperation_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    section_.operation = ui->comboOperation->currentText();
}

void Preferences::on_lineCompanyName_editingFinished()
{
    app_.company_name = ui->lineCompanyName->text();
    ResizeLine(ui->lineCompanyName, app_.company_name);
}

void Preferences::on_comboPrinter_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    app_.printer = ui->comboPrinter->currentText();
}

void Preferences::on_spinQuantityDecimal_editingFinished() { section_.quantity_decimal = ui->spinQuantityDecimal->value(); }
