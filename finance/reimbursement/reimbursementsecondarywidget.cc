#include "reimbursementsecondarywidget.h"

#include "component/signalblocker.h"
#include "ui_reimbursementsecondarywidget.h"

ReimbursementSecondaryWidget::ReimbursementSecondaryWidget(
    reimbursement::SecondaryModel* model, CSectionConfig& config, const reimbursement::PrimaryRow& row, QUuid widget_id, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ReimbursementSecondaryWidget)
    , model_ { model }
    , row_ { row }
    , widget_id_ { widget_id }
    , config_ { config }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);
}

ReimbursementSecondaryWidget::~ReimbursementSecondaryWidget() { delete ui; }

QTableView* ReimbursementSecondaryWidget::View() const { return ui->tableView; }

void ReimbursementSecondaryWidget::InitWidget() { }

void ReimbursementSecondaryWidget::InitModel(const QStringList& header) { }

void ReimbursementSecondaryWidget::on_pushButtonDelete_clicked() { }

void ReimbursementSecondaryWidget::on_pushButtonInsert_clicked() { }

void ReimbursementSecondaryWidget::on_pushButtonSave_clicked() { }

void ReimbursementSecondaryWidget::on_pushButtonApprove_clicked() { }

void ReimbursementSecondaryWidget::on_pushButtonPost_clicked() { }
