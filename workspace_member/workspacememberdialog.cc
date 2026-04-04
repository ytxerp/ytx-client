#include "workspacememberdialog.h"

#include "ui_workspacememberdialog.h"
#include "workspacemembermodel.h"

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
    auto* member_model { new WorkspaceMemberModel(ui->tableView) };
    ui->tableView->setModel(member_model);
}
