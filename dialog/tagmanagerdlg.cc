#include "tagmanagerdlg.h"

#include "enum/tagenum.h"
#include "ui_tagmanagerdlg.h"

TagManagerDlg::TagManagerDlg(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::TagManagerDlg)
{
    ui->setupUi(this);
    setMinimumSize(400, 320);
}

TagManagerDlg::~TagManagerDlg() { delete ui; }

void TagManagerDlg::SetModel(TagModel* model)
{
    model_ = model;

    ui->tableView->setModel(model);
    model->setParent(ui->tableView);
}

QTableView* TagManagerDlg::View() { return ui->tableView; }

void TagManagerDlg::on_pBtnInsert_clicked()
{
    if (!model_)
        return;

    const auto idx { ui->tableView->currentIndex() };
    const int row { idx.isValid() ? idx.row() + 1 : model_->rowCount() };

    if (model_->insertRows(row, 1)) {
        const QModelIndex new_index { model_->index(row, std::to_underlying(TagEnum::kName)) };
        ui->tableView->setCurrentIndex(new_index);
        ui->tableView->edit(new_index);
    }
}

void TagManagerDlg::on_pBtnDelete_clicked()
{
    if (!model_)
        return;

    const auto idx { ui->tableView->currentIndex() };
    if (!idx.isValid())
        return;

    model_->removeRows(idx.row(), 1);
}
