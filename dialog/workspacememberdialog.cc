#include "workspacememberdialog.h"

#include "ui_workspacememberdialog.h"

WorkspaceMemberDialog::WorkspaceMemberDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::WorkspaceMemberDialog)
{
    ui->setupUi(this);
}

WorkspaceMemberDialog::~WorkspaceMemberDialog() { delete ui; }
