#include "settlementnodewidget.h"

#include "component/signalblocker.h"
#include "enum/settlementenum.h"
#include "ui_settlementnodewidget.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

SettlementNodeWidget::SettlementNodeWidget(TreeModel* tree_model_partner, SettlementNodeModel* model, const std::shared_ptr<Settlement>& settlement,
    bool is_persisted, Section section, CUuid& widget_id, CUuid& parent_widget_id, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettlementNodeWidget)
    , settlement_ { settlement }
    , model_ { model }
    , tree_model_partner_ { tree_model_partner }
    , widget_id_ { widget_id }
    , parent_widget_id_ { parent_widget_id }
    , section_ { section }
    , is_persisted_ { is_persisted }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model);
    model->setParent(this);

    InitWidget();
    InitData();

    QTimer::singleShot(0, this, &SettlementNodeWidget::FetchNode);
}

SettlementNodeWidget::~SettlementNodeWidget() { delete ui; }

QTableView* SettlementNodeWidget::View() const { return ui->tableView; }

void SettlementNodeWidget::InitWidget()
{
    auto* pmodel { tree_model_partner_->IncludeUnitModel(
        section_ == Section::kSale ? std::to_underlying(UnitP::kCustomer) : std::to_underlying(UnitP::kVendor), this) };
    ui->comboPartner->setModel(pmodel);
    ui->comboPartner->setCurrentIndex(-1);

    ui->dateTimeEdit->setDisplayFormat(kDateTimeFST);
}

void SettlementNodeWidget::InitData()
{
    int partner_index { ui->comboPartner->findData(settlement_->partner) };
    ui->comboPartner->setCurrentIndex(partner_index);

    ui->comboPartner->setEnabled(!is_persisted_);
    HideWidget();

    ui->lineDescription->setText(settlement_->description);
    ui->dateTimeEdit->setDateTime(settlement_->issued_time.toLocalTime());
    ui->dSpinAmount->setValue(settlement_->amount);
}

void SettlementNodeWidget::FetchNode()
{
    if (settlement_->partner.isNull())
        return;

    const auto message { JsonGen::SettlementNodeAcked(section_, widget_id_, settlement_->partner) };
    WebSocket::Instance()->SendMessage(kSettlementNodeAcked, message);
}

void SettlementNodeWidget::HideWidget()
{
    const bool recalled { settlement_->status == std::to_underlying(SettlementStatus::kRecalled) };

    ui->pBtnRelease->setVisible(recalled);
    ui->pBtnRecall->setVisible(!recalled);
}

void SettlementNodeWidget::on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime) { settlement_->issued_time = dateTime.toUTC(); }

void SettlementNodeWidget::on_lineDescription_textChanged(const QString& arg1) { settlement_->description = arg1; }

void SettlementNodeWidget::on_comboPartner_currentIndexChanged(int /*index*/)
{
    if (is_persisted_)
        return;

    const QUuid partner_id { ui->comboPartner->currentData().toUuid() };
    if (settlement_->partner == partner_id)
        return;

    ui->dSpinAmount->setValue(0.0);

    settlement_->partner = partner_id;

    FetchNode();
}

void SettlementNodeWidget::on_pBtnRelease_clicked()
{
    // is_persisted_ = true;
    // ui->comboPartner->setEnabled(false);
}

void SettlementNodeWidget::on_pBtnRecall_clicked() { }
