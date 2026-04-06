#include "workspacememberdialog.h"

#include "ui_workspacememberdialog.h"

WorkspaceMemberDialog::WorkspaceMemberDialog(const QStringList& header, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::WorkspaceMemberDialog)
{
    ui->setupUi(this);
    InitDialog(header);
}

WorkspaceMemberDialog::~WorkspaceMemberDialog() { delete ui; }

QTableView* WorkspaceMemberDialog::View() { return ui->tableView; }

void WorkspaceMemberDialog::InitDialog(const QStringList& header)
{
    model_ = new WorkspaceMemberModel(header, ui->tableView);
    ui->tableView->setModel(model_);
}
