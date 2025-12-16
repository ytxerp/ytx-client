#include "settlementnodewidget.h"

#include "component/signalblocker.h"
#include "enum/settlementenum.h"
#include "global/resourcepool.h"
#include "ui_settlementnodewidget.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

SettlementNodeWidget::SettlementNodeWidget(TreeModel* tree_model_partner, SettlementNodeModel* model, Settlement* settlement, bool is_persisted,
    Section section, CUuid& widget_id, CUuid& parent_widget_id, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettlementNodeWidget)
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

    QTimer::singleShot(0, this, &SettlementNodeWidget::FetchNode);
}

SettlementNodeWidget::~SettlementNodeWidget()
{
    if (!is_persisted_)
        ResourcePool<Settlement>::Instance().Recycle(settlement_);

    delete ui;
}

QTableView* SettlementNodeWidget::View() const { return ui->tableView; }

void SettlementNodeWidget::RSyncAmount(double amount)
{
    tmp_settlement_.amount += amount;
    ui->dSpinAmount->setValue(tmp_settlement_.amount);
}

void SettlementNodeWidget::InitWidget()
{
    auto* pmodel { tree_model_partner_->IncludeUnitModel(
        section_ == Section::kSale ? std::to_underlying(UnitP::kCustomer) : std::to_underlying(UnitP::kVendor), this) };
    ui->comboPartner->setModel(pmodel);
    ui->comboPartner->setCurrentIndex(-1);

    ui->dateTimeEdit->setDisplayFormat(kDateTimeFST);
    ui->dSpinAmount->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
}

void SettlementNodeWidget::InitData()
{
    int partner_index { ui->comboPartner->findData(tmp_settlement_.partner) };
    ui->comboPartner->setCurrentIndex(partner_index);

    ui->comboPartner->setEnabled(!is_persisted_);
    HideWidget();

    ui->lineDescription->setText(tmp_settlement_.description);
    ui->dateTimeEdit->setDateTime(tmp_settlement_.issued_time.toLocalTime());
    ui->dSpinAmount->setValue(tmp_settlement_.amount);
}

void SettlementNodeWidget::FetchNode()
{
    if (tmp_settlement_.partner.isNull())
        return;

    const auto message { JsonGen::SettlementNodeAcked(section_, widget_id_, tmp_settlement_.partner, tmp_settlement_.id) };
    WebSocket::Instance()->SendMessage(kSettlementNodeAcked, message);
}

void SettlementNodeWidget::HideWidget()
{
    const bool recalled { tmp_settlement_.status == std::to_underlying(SettlementStatus::kRecalled) };

    ui->pBtnRelease->setVisible(recalled);
    ui->pBtnRecall->setVisible(!recalled);
}

void SettlementNodeWidget::on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime)
{
    const QDateTime utc_time { dateTime.toUTC() };
    if (tmp_settlement_.issued_time == utc_time)
        return;

    tmp_settlement_.issued_time = dateTime.toUTC();

    if (is_persisted_)
        pending_update_.insert(kIssuedTime, utc_time.toString(Qt::ISODate));
}

void SettlementNodeWidget::on_lineDescription_textChanged(const QString& arg1)
{
    tmp_settlement_.description = arg1;

    if (is_persisted_)
        pending_update_.insert(kDescription, arg1);
}

void SettlementNodeWidget::on_comboPartner_currentIndexChanged(int /*index*/)
{
    if (is_persisted_)
        return;

    const QUuid partner_id { ui->comboPartner->currentData().toUuid() };
    if (tmp_settlement_.partner == partner_id)
        return;

    tmp_settlement_.amount = 0.0;
    ui->dSpinAmount->setValue(0.0);

    tmp_settlement_.partner = partner_id;

    FetchNode();
}

void SettlementNodeWidget::on_pBtnRelease_clicked()
{
    assert(!tmp_settlement_.partner.isNull() && "tmp_settlement_.partner should never be null here");

    if (!is_persisted_ && !model_->HasPendingChange()) {
        return;
    }

    if (tmp_settlement_.status == std::to_underlying(NodeStatus::kReleased))
        return;

    if (is_persisted_)
        pending_update_.insert(kStatus, std::to_underlying(NodeStatus::kReleased));

    QJsonObject message { JsonGen::MetaMessage(section_) };
    model_->Finalize(message);

    message.insert(kWidgetId, widget_id_.toString(QUuid::WithoutBraces));
    message.insert(kParentWidgetId, parent_widget_id_.toString(QUuid::WithoutBraces));
    message.insert(kAmount, QString::number(tmp_settlement_.amount, 'f', kMaxNumericScale_4));

    if (is_persisted_) {
        message.insert(kSettlementUpdate, pending_update_);
        message.insert(kSettlementId, tmp_settlement_.id.toString(QUuid::WithoutBraces));

        WebSocket::Instance()->SendMessage(kSettlementUpdateReleased, message);

        pending_update_ = QJsonObject();
    } else {
        // is_persisted_ = true;
        // ui->comboPartner->setEnabled(false);

        message.insert(kSettlementInsert, tmp_settlement_.WriteJson());
        WebSocket::Instance()->SendMessage(kSettlementInsertReleased, message);
    }
}

void SettlementNodeWidget::on_pBtnRecall_clicked() { }
