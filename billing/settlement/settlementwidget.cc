#include "settlementwidget.h"

#include "component/constant.h"
#include "component/signalblocker.h"
#include "enum/statementenum.h"
#include "ui_settlementwidget.h"

SettlementWidget::SettlementWidget(SettlementModel* settlement_model, CDateTime& start, CDateTime& end, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettlementWidget)
    , settlement_model_ { settlement_model }
    , start_ { start }
    , end_ { end }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);
    IniWidget();
}

SettlementWidget::~SettlementWidget() { delete ui; }

QTableView* SettlementWidget::View() const { return ui->tableView; }

QAbstractItemModel* SettlementWidget::Model() const { return ui->tableView->model(); }

void SettlementWidget::on_start_dateChanged(const QDate& date)
{
    ui->pBtnFetch->setEnabled(date <= end_.date());
    start_.setDate(date);
}

void SettlementWidget::on_end_dateChanged(const QDate& date)
{
    ui->pBtnFetch->setEnabled(date >= start_.date());
    end_.setDate(date);
}

void SettlementWidget::on_pBtnFetch_clicked() { settlement_model_->ResetModel(start_, end_); }

void SettlementWidget::IniWidget()
{
    ui->start->setDisplayFormat(kDateFST);
    ui->end->setDisplayFormat(kDateFST);

    ui->pBtnFetch->setFocus();

    ui->start->setDateTime(start_);
    ui->end->setDateTime(end_);
}

void SettlementWidget::on_pBtnAppend_clicked() { settlement_model_->insertRows(settlement_model_->rowCount(), 1); }

void SettlementWidget::on_pBtnRemove_clicked()
{
    auto* view { ui->tableView };

    const auto index { view->selectionModel()->selectedIndexes().first() };
    if (!index.isValid())
        return;

    settlement_model_->removeRows(index.row(), 1);
}

void SettlementWidget::on_tableView_doubleClicked(const QModelIndex& index)
{
    if (index.column() != std::to_underlying(SettlementEnum::kInitialTotal))
        return;

    const auto partner { index.siblingAtColumn(std::to_underlying(SettlementEnum::kPartner)).data().toUuid() };
    if (partner.isNull())
        return;

    const auto settlement_id { index.siblingAtColumn(std::to_underlying(SettlementEnum::kId)).data().toUuid() };
    const bool status { index.siblingAtColumn(std::to_underlying(SettlementEnum::kStatus)).data().toBool() };

    settlement_primary_model_->RResetModel(partner, settlement_id, status);
}
