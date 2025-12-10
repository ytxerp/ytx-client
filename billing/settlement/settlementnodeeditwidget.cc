#include "settlementnodeeditwidget.h"

#include "component/signalblocker.h"
#include "ui_settlementnodeeditwidget.h"

SettlementNodeEditWidget::SettlementNodeEditWidget(TreeModel* tree_model_partner, SettlementNodeEditModel* model, const std::shared_ptr<Settlement>& settlement,
    Section section, CUuid& widget_id, CUuid& parent_widget_id, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettlementNodeEditWidget)
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

SettlementNodeEditWidget::~SettlementNodeEditWidget() { delete ui; }

QTableView* SettlementNodeEditWidget::View() const { return ui->tableView; }

void SettlementNodeEditWidget::InitWidget()
{
    auto* pmodel { tree_model_partner_->IncludeUnitModel(
        section_ == Section::kSale ? std::to_underlying(UnitP::kCustomer) : std::to_underlying(UnitP::kVendor), this) };
    ui->comboPartner->setModel(pmodel);
    ui->comboPartner->setCurrentIndex(-1);

    ui->dateTimeEdit->setDisplayFormat(kDateTimeFST);
}

void SettlementNodeEditWidget::InitData()
{
    int partner_index { ui->comboPartner->findData(settlement_->partner) };
    ui->comboPartner->setCurrentIndex(partner_index);

    ui->comboPartner->setEnabled(false);

    ui->lineDescription->setText(settlement_->description);
    ui->dateTimeEdit->setDateTime(settlement_->issued_time.toLocalTime());
    ui->dSpinAmount->setValue(settlement_->amount);
}

void SettlementNodeEditWidget::on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime) { settlement_->issued_time = dateTime.toUTC(); }

void SettlementNodeEditWidget::on_lineDescription_textChanged(const QString& arg1) { settlement_->description = arg1; }

void SettlementNodeEditWidget::on_pBtnRelease_clicked() { }

void SettlementNodeEditWidget::on_pBtnRecall_clicked() { }
