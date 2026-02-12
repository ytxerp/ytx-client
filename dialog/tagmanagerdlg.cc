#include "tagmanagerdlg.h"

#include "dialog/deletenode/exactmatchconfirmdialog.h"
#include "enum/tagenum.h"
#include "ui_tagmanagerdlg.h"

TagManagerDlg::TagManagerDlg(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::TagManagerDlg)
{
    ui->setupUi(this);
    setMinimumSize(400, 300);

    ui->pBtnInsert->setShortcut(QKeySequence::New);
    ui->pBtnDelete->setShortcut(QKeySequence::Delete);
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

    const auto* tag { static_cast<Tag*>(idx.internalPointer()) };
    if (!tag)
        return;

    if (tag->name.isEmpty()) {
        model_->removeRows(idx.row(), 1);
        return;
    }

    const QString info { tr("Delete tag <b>%1</b>?<br>"
                            "<span style='color:#d32f2f; font-weight:bold;'><br>⚠️ Permanent deletion! Cannot be undone!</span>"
                            "<br><br><i>Note: Tag references in nodes and entries will be preserved but no longer displayed.</i>")
            .arg(tag->name) };

    auto* dlg { new ExactMatchConfirmDialog(info, tag->name, tr("Delete"), this) };
    dlg->setModal(true);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(dlg, &ExactMatchConfirmDialog::accepted, this, [=, this]() { model_->removeRows(idx.row(), 1); });
    dlg->show();
}
