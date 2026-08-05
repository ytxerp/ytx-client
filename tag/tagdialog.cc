#include "tagdialog.h"

#include "component/signalblocker.h"
#include "dialog/deletenode/exactmatchconfirmdialog.h"
#include "tagenum.h"
#include "ui_tagdialog.h"

TagDialog::TagDialog(tag::Model* model, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::TagDialog)
    , model_ { model }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model);
    model->setParent(ui->tableView);

    setMinimumSize(400, 300);
}

TagDialog::~TagDialog() { delete ui; }

QTableView* TagDialog::View() { return ui->tableView; }

void TagDialog::on_pBtnInsert_clicked()
{
    if (!model_)
        return;

    const auto idx { ui->tableView->currentIndex() };
    const int row { idx.isValid() ? idx.row() + 1 : model_->rowCount() };

    if (model_->insertRows(row, 1)) {
        const QModelIndex new_index { model_->index(row, std::to_underlying(tag::RowField::kName)) };
        ui->tableView->setCurrentIndex(new_index);
        ui->tableView->edit(new_index);
    }
}

void TagDialog::on_pBtnDelete_clicked()
{
    if (!model_)
        return;

    const auto idx { ui->tableView->currentIndex() };
    if (!idx.isValid())
        return;

    const auto* tag { static_cast<tag::Row*>(idx.internalPointer()) };
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

    auto* dlg { new ExactMatchConfirmDialog(info, tr("Delete"), this) };
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(dlg, &ExactMatchConfirmDialog::accepted, this, [this, idx]() { model_->removeRows(idx.row(), 1); });
    dlg->show();
}
