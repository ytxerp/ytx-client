#include "settlementwidget.h"

#include "component/constant.h"
#include "component/enumclass.h"
#include "component/signalblocker.h"
#include "ui_settlementwidget.h"

SettlementWidget::SettlementWidget(
    SettlementModel* settlement_model, SettlementPrimaryModel* settlement_primary_model, CDateTime& start, CDateTime& end, QWidget* parent)
    : ReportWidget(parent)
    , ui(new Ui::SettlementWidget)
    , settlement_model_ { settlement_model }
    , settlement_primary_model_ { settlement_primary_model }
    , start_ { start }
    , end_ { end }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);
    IniWidget(settlement_model, settlement_primary_model);
}

SettlementWidget::~SettlementWidget() { delete ui; }

QTableView* SettlementWidget::View() const { return ui->settlementView; }

QTableView* SettlementWidget::PrimaryView() const { return ui->settlementViewPrimary; }

QAbstractItemModel* SettlementWidget::Model() const { return ui->settlementView->model(); }

void SettlementWidget::on_start_dateChanged(const QDate& date)
{
    ui->pBtnRefresh->setEnabled(date <= end_.date());
    start_.setDate(date);
}

void SettlementWidget::on_end_dateChanged(const QDate& date)
{
    ui->pBtnRefresh->setEnabled(date >= start_.date());
    end_.setDate(date);
}

void SettlementWidget::on_pBtnRefresh_clicked() { settlement_model_->ResetModel(start_, end_); }

void SettlementWidget::IniWidget(SettlementModel* settlement_model, SettlementPrimaryModel* settlement_primary_model)
{
    ui->start->setDisplayFormat(kDateFST);
    ui->end->setDisplayFormat(kDateFST);

    ui->settlementView->setModel(settlement_model);
    ui->settlementViewPrimary->setModel(settlement_primary_model);
    ui->pBtnRefresh->setFocus();

    ui->start->setDateTime(start_);
    ui->end->setDateTime(end_);
}

void SettlementWidget::on_pBtnAppend_clicked() { settlement_model_->insertRows(settlement_model_->rowCount(), 1); }

void SettlementWidget::on_pBtnRemoveSettlement_clicked()
{
    auto* view { ui->settlementView };

    const auto index { view->selectionModel()->selectedIndexes().first() };
    if (!index.isValid())
        return;

    settlement_model_->removeRows(index.row(), 1);
}

void SettlementWidget::on_settlementView_doubleClicked(const QModelIndex& index)
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

void SettlementWidget::on_settlementViewPrimary_doubleClicked(const QModelIndex& index)
{
    if (index.column() != std::to_underlying(SettlementEnum::kInitialTotal))
        return;

    const auto node_id { index.siblingAtColumn(std::to_underlying(SettlementEnum::kId)).data().toUuid() };
    emit SNodeLocation(node_id);
}
