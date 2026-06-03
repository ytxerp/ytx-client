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

void WorkspaceMemberDialog::on_pushButtonDelete_clicked()
{
    auto* view { ui->tableView };
    const auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    const int row { index.row() };
    model_->removeRows(row, 1);
}
