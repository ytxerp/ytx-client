#include "settlementnodeappendwidget.h"

#include "component/signalblocker.h"
#include "ui_settlementnodeappendwidget.h"

SettlementNodeAppendWidget::SettlementNodeAppendWidget(TreeModel* tree_model_partner, SettlementNodeAppendModel* model,
    const std::shared_ptr<Settlement>& settlement, Section section, CUuid& widget_id, CUuid& parent_widget_id, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettlementNodeAppendWidget)
    , settlement_ { settlement }
    , model_ { model }
    , tree_model_partner_ { tree_model_partner }
    , widget_id_ { widget_id }
    , parent_widget_id_ { parent_widget_id }
    , section_ { section }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model);
    model->setParent(this);

    InitWidget();
    InitData();
}

SettlementNodeAppendWidget::~SettlementNodeAppendWidget() { delete ui; }

QTableView* SettlementNodeAppendWidget::View() const { return ui->tableView; }

void SettlementNodeAppendWidget::InitWidget()
{
    auto* pmodel { tree_model_partner_->IncludeUnitModel(
        section_ == Section::kSale ? std::to_underlying(UnitP::kCustomer) : std::to_underlying(UnitP::kVendor), this) };
    ui->comboPartner->setModel(pmodel);
    ui->comboPartner->setCurrentIndex(-1);

    ui->dateTimeEdit->setDisplayFormat(kDateTimeFST);
}

void SettlementNodeAppendWidget::InitData()
{
    int partner_index { ui->comboPartner->findData(settlement_->partner) };
    ui->comboPartner->setCurrentIndex(partner_index);

    ui->lineDescription->setText(settlement_->description);
    ui->dateTimeEdit->setDateTime(settlement_->issued_time.toLocalTime());
    ui->dSpinAmount->setValue(settlement_->amount);
}

void SettlementNodeAppendWidget::on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime) { settlement_->issued_time = dateTime.toUTC(); }

void SettlementNodeAppendWidget::on_lineDescription_textChanged(const QString& arg1) { settlement_->description = arg1; }

void SettlementNodeAppendWidget::on_comboPartner_currentIndexChanged(int /*index*/)
{
    if (is_persisted_)
        return;

    const QUuid partner_id { ui->comboPartner->currentData().toUuid() };
    if (settlement_->partner == partner_id)
        return;

    ui->dSpinAmount->setValue(0.0);

    settlement_->partner = partner_id;
    model_->UpdatePartner(partner_id);
}

void SettlementNodeAppendWidget::on_pBtnRelease_clicked()
{
    // is_persisted_ = true;
    // ui->comboPartner->setEnabled(false);
}

void SettlementNodeAppendWidget::on_pBtnRecall_clicked() { }
