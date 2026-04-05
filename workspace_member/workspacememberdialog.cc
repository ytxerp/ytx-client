#include "workspacememberdialog.h"

#include "ui_workspacememberdialog.h"

WorkspaceMemberDialog::WorkspaceMemberDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::WorkspaceMemberDialog)
{
    ui->setupUi(this);
    InitDialog();
}

WorkspaceMemberDialog::~WorkspaceMemberDialog() { delete ui; }

QTableView* WorkspaceMemberDialog::View() { return ui->tableView; }

void WorkspaceMemberDialog::InitDialog()
{
    model_ = new WorkspaceMemberModel(ui->tableView);
    ui->tableView->setModel(model_);
}
