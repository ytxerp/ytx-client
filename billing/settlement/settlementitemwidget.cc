#include "settlementitemwidget.h"

#include "component/signalblocker.h"
#include "enum/settlementenum.h"
#include "global/resourcepool.h"
#include "ui_settlementitemwidget.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

SettlementItemWidget::SettlementItemWidget(TreeModel* tree_model_partner, SettlementItemModel* model, Settlement* settlement, bool is_persisted,
    Section section, CUuid& widget_id, CUuid& parent_widget_id, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettlementItemWidget)
    , settlement_ { settlement }
    , tmp_settlement_ { *settlement }
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

    QTimer::singleShot(0, this, &SettlementItemWidget::FetchNode);
}

SettlementItemWidget::~SettlementItemWidget()
{
    if (!is_persisted_)
        ResourcePool<Settlement>::Instance().Recycle(settlement_);

    delete ui;
}

QTableView* SettlementItemWidget::View() const { return ui->tableView; }

void SettlementItemWidget::RSyncAmount(double amount)
{
    tmp_settlement_.amount += amount;
    ui->dSpinAmount->setValue(tmp_settlement_.amount);
}

void SettlementItemWidget::InitWidget()
{
    auto* pmodel { tree_model_partner_->IncludeUnitModel(
        section_ == Section::kSale ? std::to_underlying(UnitP::kCustomer) : std::to_underlying(UnitP::kVendor), this) };
    ui->comboPartner->setModel(pmodel);
    ui->comboPartner->setCurrentIndex(-1);

    ui->dateTimeEdit->setDisplayFormat(kDateTimeFST);
    ui->dSpinAmount->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
}

void SettlementItemWidget::InitData()
{
    int partner_index { ui->comboPartner->findData(tmp_settlement_.partner_id) };
    ui->comboPartner->setCurrentIndex(partner_index);

    ui->lineDescription->setText(tmp_settlement_.description);
    ui->dateTimeEdit->setDateTime(tmp_settlement_.issued_time.toLocalTime());
    ui->dSpinAmount->setValue(tmp_settlement_.amount);

    ui->comboPartner->setEnabled(!is_persisted_);

    const bool is_released { tmp_settlement_.status == std::to_underlying(SettlementStatus::kReleased) };
    ui->lineDescription->setReadOnly(is_released);
    ui->dateTimeEdit->setReadOnly(is_released);

    HideWidget(is_released);
}

void SettlementItemWidget::FetchNode()
{
    if (tmp_settlement_.partner_id.isNull())
        return;

    const auto message { JsonGen::SettlementNodeAcked(section_, widget_id_, tmp_settlement_.partner_id, tmp_settlement_.id) };
    WebSocket::Instance()->SendMessage(kSettlementItemAcked, message);
}

void SettlementItemWidget::HideWidget(bool is_released)
{
    ui->pBtnRelease->setVisible(!is_released);
    ui->pBtnRecall->setVisible(is_released);
}

void SettlementItemWidget::on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime)
{
    const QDateTime utc_time { dateTime.toUTC() };
    if (tmp_settlement_.issued_time == utc_time)
        return;

    tmp_settlement_.issued_time = dateTime.toUTC();

    if (is_persisted_)
        pending_update_.insert(kIssuedTime, utc_time.toString(Qt::ISODate));
}

void SettlementItemWidget::on_lineDescription_textChanged(const QString& arg1)
{
    tmp_settlement_.description = arg1;

    if (is_persisted_)
        pending_update_.insert(kDescription, arg1);
}

void SettlementItemWidget::on_comboPartner_currentIndexChanged(int /*index*/)
{
    if (is_persisted_)
        return;

    const QUuid partner_id { ui->comboPartner->currentData().toUuid() };
    if (tmp_settlement_.partner_id == partner_id)
        return;

    tmp_settlement_.amount = 0.0;
    tmp_settlement_.status = 0;
    tmp_settlement_.description.clear();

    ui->dSpinAmount->setValue(0.0);
    ui->lineDescription->clear();

    tmp_settlement_.partner_id = partner_id;

    FetchNode();
}

void SettlementItemWidget::on_pBtnRelease_clicked()
{
    assert(!tmp_settlement_.partner_id.isNull() && "tmp_settlement_.partner should never be null here");

    {
        if (!is_persisted_ && !model_->HasPendingChange()) {
            return;
        }

        if (tmp_settlement_.status == std::to_underlying(SettlementStatus::kReleased))
            return;
    }

    {
        tmp_settlement_.status = std::to_underlying(SettlementStatus::kReleased);

        if (is_persisted_)
            pending_update_.insert(kStatus, std::to_underlying(SettlementStatus::kReleased));
    }

    {
        QJsonObject message { JsonGen::MetaMessage(section_) };
        model_->Finalize(message);

        message.insert(kWidgetId, widget_id_.toString(QUuid::WithoutBraces));
        message.insert(kParentWidgetId, parent_widget_id_.toString(QUuid::WithoutBraces));

        if (is_persisted_) {
            message.insert(kSettlement, pending_update_);
            message.insert(kSettlementId, tmp_settlement_.id.toString(QUuid::WithoutBraces));

            WebSocket::Instance()->SendMessage(kSettlementUpdated, message);

            pending_update_ = QJsonObject();
        } else {
            message.insert(kSettlement, tmp_settlement_.WriteJson());
            WebSocket::Instance()->SendMessage(kSettlementInserted, message);
        }
    }
}

void SettlementItemWidget::on_pBtnRecall_clicked() { tmp_settlement_.status = std::to_underlying(SettlementStatus::kRecalled); }

void SettlementItemWidget::ReleaseSucceeded()
{
    is_persisted_ = true;
    ui->comboPartner->setEnabled(false);

    ui->lineDescription->setReadOnly(true);
    ui->dateTimeEdit->setReadOnly(true);

    model_->UpdateStatus(SettlementStatus::kReleased);
    ui->tableView->clearSelection();

    HideWidget(true);
}

void SettlementItemWidget::RecallSucceeded()
{
    ui->lineDescription->setReadOnly(false);
    ui->dateTimeEdit->setReadOnly(false);

    model_->UpdateStatus(SettlementStatus::kRecalled);

    HideWidget(false);
}
