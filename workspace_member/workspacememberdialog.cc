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

void WorkspaceMemberDialog::InitDialog()
{
    auto* member_model { new WorkspaceMemberModel(ui->tableView) };
    ui->tableView->setModel(member_model);
}
