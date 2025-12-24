#include "tablewidgetsettlement.h"

#include "component/signalblocker.h"
#include "enum/settlementenum.h"
#include "ui_tablewidgetsettlement.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableWidgetSettlement::TableWidgetSettlement(TreeModel* tree_model_partner, TableModelSettlement* model, const Settlement& settlement, bool is_persisted,
    Section section, CUuid& widget_id, CUuid& parent_widget_id, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TableWidgetSettlement)
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

    QTimer::singleShot(0, this, &TableWidgetSettlement::FetchNode);
}

TableWidgetSettlement::~TableWidgetSettlement() { delete ui; }

QTableView* TableWidgetSettlement::View() const { return ui->tableView; }

void TableWidgetSettlement::RSyncAmount(double amount)
{
    settlement_.amount += amount;
    ui->dSpinAmount->setValue(settlement_.amount);
}

void TableWidgetSettlement::InitWidget()
{
    auto* pmodel { tree_model_partner_->IncludeUnitModel(
        section_ == Section::kSale ? std::to_underlying(UnitP::kCustomer) : std::to_underlying(UnitP::kVendor), this) };
    ui->comboPartner->setModel(pmodel);
    ui->comboPartner->setCurrentIndex(-1);

    ui->dateTimeEdit->setDisplayFormat(kDateTimeFST);
    ui->dSpinAmount->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
}

void TableWidgetSettlement::InitData()
{
    const int partner_index { ui->comboPartner->findData(settlement_.partner_id) };
    ui->comboPartner->setCurrentIndex(partner_index);

    ui->lineDescription->setText(settlement_.description);
    ui->dateTimeEdit->setDateTime(settlement_.issued_time.toLocalTime());
    ui->dSpinAmount->setValue(settlement_.amount);

    ui->comboPartner->setEnabled(!is_persisted_);

    const bool is_released { settlement_.status == SettlementStatus::kReleased };
    ui->lineDescription->setReadOnly(is_released);
    ui->dateTimeEdit->setReadOnly(is_released);

    HideWidget(is_released);
}

void TableWidgetSettlement::FetchNode()
{
    if (settlement_.partner_id.isNull())
        return;

    const auto message { JsonGen::SettlementNodeAcked(section_, widget_id_, settlement_.partner_id, settlement_.id) };
    WebSocket::Instance()->SendMessage(kSettlementItemAcked, message);
}

void TableWidgetSettlement::HideWidget(bool is_released)
{
    ui->pBtnRelease->setVisible(!is_released);
    ui->pBtnRecall->setVisible(is_released);
}

void TableWidgetSettlement::on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime)
{
    const QDateTime utc_time { dateTime.toUTC() };
    if (settlement_.issued_time == utc_time)
        return;

    settlement_.issued_time = dateTime.toUTC();

    if (is_persisted_)
        pending_update_.insert(kIssuedTime, utc_time.toString(Qt::ISODate));
}

void TableWidgetSettlement::on_lineDescription_textChanged(const QString& arg1)
{
    settlement_.description = arg1;

    if (is_persisted_)
        pending_update_.insert(kDescription, arg1);
}

void TableWidgetSettlement::on_comboPartner_currentIndexChanged(int /*index*/)
{
    if (is_persisted_)
        return;

    const QUuid partner_id { ui->comboPartner->currentData().toUuid() };
    if (settlement_.partner_id == partner_id)
        return;

    settlement_.amount = 0.0;
    settlement_.status = SettlementStatus::kRecalled;
    settlement_.description.clear();

    ui->dSpinAmount->setValue(0.0);
    ui->lineDescription->clear();

    settlement_.partner_id = partner_id;

    FetchNode();
    emit SUpdatePartner(widget_id_, partner_id);
}

void TableWidgetSettlement::on_pBtnRelease_clicked()
{
    assert(!settlement_.partner_id.isNull() && "settlement_.partner should never be null here");

    model_->NormalizeBuffer();

    {
        if (!is_persisted_ && !model_->HasPendingChange()) {
            return;
        }

        if (settlement_.status == SettlementStatus::kReleased)
            return;
    }

    {
        settlement_.status = SettlementStatus::kReleased;

        if (is_persisted_) {
            pending_update_.insert(kStatus, std::to_underlying(SettlementStatus::kReleased));
            pending_update_.insert(kAmount, QString::number(settlement_.amount, 'f', kMaxNumericScale_4));
            pending_update_.insert(kVersion, settlement_.version);
        }
    }

    {
        QJsonObject message { JsonGen::MetaMessage(section_) };
        model_->Finalize(message);

        message.insert(kWidgetId, widget_id_.toString(QUuid::WithoutBraces));
        message.insert(kParentWidgetId, parent_widget_id_.toString(QUuid::WithoutBraces));
        message.insert(kSettlementId, settlement_.id.toString(QUuid::WithoutBraces));

        if (is_persisted_) {
            message.insert(kSettlement, pending_update_);

            WebSocket::Instance()->SendMessage(kSettlementUpdated, message);
            pending_update_ = QJsonObject();
        } else {
            message.insert(kSettlement, settlement_.WriteJson());
            WebSocket::Instance()->SendMessage(kSettlementInserted, message);
        }
    }
}

void TableWidgetSettlement::on_pBtnRecall_clicked()
{
    if (settlement_.status == SettlementStatus::kRecalled)
        return;

    settlement_.status = SettlementStatus::kRecalled;

    QJsonObject message { JsonGen::MetaMessage(section_) };
    model_->Finalize(message);

    pending_update_.insert(kStatus, std::to_underlying(SettlementStatus::kRecalled));
    pending_update_.insert(kVersion, settlement_.version);

    message.insert(kWidgetId, widget_id_.toString(QUuid::WithoutBraces));
    message.insert(kParentWidgetId, parent_widget_id_.toString(QUuid::WithoutBraces));
    message.insert(kSettlementId, settlement_.id.toString(QUuid::WithoutBraces));
    message.insert(kSettlement, pending_update_);

    WebSocket::Instance()->SendMessage(kSettlementRecalled, message);
    pending_update_ = QJsonObject();
}

void TableWidgetSettlement::InsertSucceeded(int version)
{
    is_persisted_ = true;
    settlement_.version = version;

    ui->comboPartner->setEnabled(false);

    ui->lineDescription->setReadOnly(true);
    ui->dateTimeEdit->setReadOnly(true);

    model_->UpdateStatus(SettlementStatus::kReleased);
    ui->tableView->clearSelection();

    HideWidget(true);
}

void TableWidgetSettlement::RecallSucceeded(int version)
{
    ui->lineDescription->setReadOnly(false);
    ui->dateTimeEdit->setReadOnly(false);

    settlement_.version = version;
    model_->UpdateStatus(SettlementStatus::kRecalled);

    HideWidget(false);
}

void TableWidgetSettlement::UpdateSucceeded(int version)
{
    ui->lineDescription->setReadOnly(true);
    ui->dateTimeEdit->setReadOnly(true);

    settlement_.version = version;
    model_->UpdateStatus(SettlementStatus::kReleased);

    ui->tableView->clearSelection();
    HideWidget(true);
}
