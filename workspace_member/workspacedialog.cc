#include "workspacedialog.h"

#include "ui_workspacedialog.h"

WorkspaceDialog::WorkspaceDialog(const QStringList& header, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::WorkspaceDialog)
{
    ui->setupUi(this);
    InitDialog(header);
}

WorkspaceDialog::~WorkspaceDialog() { delete ui; }

QTableView* WorkspaceDialog::View() { return ui->tableView; }

void WorkspaceDialog::InitDialog(const QStringList& header)
{
    model_ = new workspace::Model(header, ui->tableView);
    ui->tableView->setModel(model_);
}

void WorkspaceDialog::on_pushButtonDelete_clicked()
{
    auto* view { ui->tableView };
    const auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    const int row { index.row() };
    model_->removeRows(row, 1);
}
