#include "periodclosedialog.h"

#include "component/signalblocker.h"
#include "ui_periodclosedialog.h"

PeriodCloseDialog::PeriodCloseDialog(CTreeModel* model, SearchEntryModel* table_model, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PeriodCloseDialog)
    , model_ { model }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(table_model);
    table_model->setParent(ui->tableView);

    InitDialog();
}

PeriodCloseDialog::~PeriodCloseDialog() { delete ui; }

QTableView* PeriodCloseDialog::View() { return ui->tableView; }

void PeriodCloseDialog::InitDialog()
{
    {
        auto* leaf_path_branch_path_model = new ItemModel(this);
        model_->LeafPathBranchPathModel(leaf_path_branch_path_model);
        leaf_path_branch_path_model->sort(0);

        ui->comboBoxFrom->setModel(leaf_path_branch_path_model);
        ui->comboBoxFrom->setCurrentIndex(-1);
    }

    {
        auto* leaf_model { model_->LeafModel() };
        leaf_model->sort(0);

        ui->comboBoxTo->setModel(leaf_model);
        ui->comboBoxTo->setCurrentIndex(-1);
    }
}
