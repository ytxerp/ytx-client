#include "settlementsecondarywidget.h"

#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "ui_settlementsecondarywidget.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

SettlementSecondaryWidget::SettlementSecondaryWidget(TreeModel* tree_model_p, SettlementSecondaryModel* model, CSectionConfig& config,
    const SettlementPrimary& settlement, CUuid& widget_id, CUuid& parent_widget_id, Section section, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettlementSecondaryWidget)
    , settlement_ { settlement }
    , model_ { model }
    , config_ { config }
    , tree_model_p_ { tree_model_p }
    , widget_id_ { widget_id }
    , parent_widget_id_ { parent_widget_id }
    , section_ { section }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model);
    model->setParent(ui->tableView);

    InitWidget();
    InitData();

    QTimer::singleShot(0, this, &SettlementSecondaryWidget::FetchNode);
    if (settlement.sync_state == SyncState::kCreating)
        QTimer::singleShot(0, this, [this]() { ui->comboPartner->setFocus(); });
}

SettlementSecondaryWidget::~SettlementSecondaryWidget() { delete ui; }

QTableView* SettlementSecondaryWidget::View() const { return ui->tableView; }

void SettlementSecondaryWidget::RSyncAmount(double amount)
{
    settlement_.amount += amount;
    ui->dSpinAmount->setValue(settlement_.amount);
}

void SettlementSecondaryWidget::InitWidget()
{
    auto* pmodel { tree_model_p_->IncludeUnit(section_ == Section::kSale ? NodeUnit::PCustomer : NodeUnit::PVendor, this) };
    ui->comboPartner->setModel(pmodel);
    ui->comboPartner->setCurrentIndex(-1);

    ui->dateTimeEdit->setDisplayFormat(datetime_format::kDateTime);
    ui->dSpinAmount->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
    ui->dSpinAmount->setDecimals(config_.amount_decimal);

    utils::SetPushButton(ui->pBtnRelease, QKeySequence(Qt::CTRL | Qt::Key_Return));
    utils::SetPushButton(ui->pBtnRecall, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
}

void SettlementSecondaryWidget::InitData()
{
    const int partner_index { ui->comboPartner->findData(settlement_.partner_id) };
    ui->comboPartner->setCurrentIndex(partner_index);

    ui->lineDescription->setText(settlement_.description);
    ui->dateTimeEdit->setDateTime(settlement_.issued_time.toLocalTime());
    ui->dSpinAmount->setValue(settlement_.amount);

    ui->comboPartner->setReadOnly(settlement_.sync_state != SyncState::kCreating);

    const bool is_settled { settlement_.status == SettlementStatus::kSettled };

    LockWidget(is_settled);
}

void SettlementSecondaryWidget::FetchNode()
{
    if (settlement_.partner_id.isNull())
        return;

    const auto message { JsonGen::SettlementItemAck(section_, widget_id_, settlement_.partner_id, settlement_.id) };
    WebSocket::Instance()->SendMessage(WsKey::kSettlementItemAck, message);
}

void SettlementSecondaryWidget::LockWidget(bool is_settled)
{
    ui->pBtnRelease->setEnabled(!is_settled);
    ui->pBtnRecall->setEnabled(is_settled);

    ui->lineDescription->setReadOnly(is_settled);
    ui->dateTimeEdit->setReadOnly(is_settled);
}

bool SettlementSecondaryWidget::ValidateSyncState()
{
    if (settlement_.sync_state == SyncState::kUpdating) {
        utils::ShowNotification(QMessageBox::Information, tr("Invalid Operation"),
            tr("The operation you attempted is invalid because your local data is outdated. Please refresh and try again."), time_const::kAutoCloseMs);
        return false;
    }

    qDebug() << "[ValidateSyncState] Passed: sync_state =" << std::to_underlying(settlement_.sync_state);

    return true;
}

void SettlementSecondaryWidget::on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime)
{
    const QDateTime utc_time { dateTime.toUTC() };
    if (settlement_.issued_time == utc_time)
        return;

    settlement_.issued_time = utc_time;

    if (settlement_.sync_state == SyncState::kSynced)
        pending_update_.insert(kIssuedTime, utc_time.toString(Qt::ISODate));
}

void SettlementSecondaryWidget::on_lineDescription_textChanged(const QString& arg1)
{
    settlement_.description = arg1;

    if (settlement_.sync_state == SyncState::kSynced)
        pending_update_.insert(kDescription, arg1);
}

void SettlementSecondaryWidget::on_comboPartner_currentIndexChanged(int /*index*/)
{
    if (settlement_.sync_state == SyncState::kSynced)
        return;

    const QUuid partner_id { ui->comboPartner->currentData().toUuid() };
    if (settlement_.partner_id == partner_id)
        return;

    settlement_.amount = 0.0;
    settlement_.status = SettlementStatus::kUnsettled;
    settlement_.description.clear();

    ui->dSpinAmount->setValue(0.0);
    ui->lineDescription->clear();

    settlement_.partner_id = partner_id;

    FetchNode();
    emit SUpdatePartner(widget_id_, partner_id);
}

void SettlementSecondaryWidget::on_pBtnRelease_clicked()
{
    Q_ASSERT(!settlement_.partner_id.isNull());
    Q_ASSERT(settlement_.status == SettlementStatus::kUnsettled);

    {
        if (!model_->HasPendingUpdate()) {
            return;
        }

        if (!ValidateSyncState())
            return;
    }

    {
        QJsonObject message {};
        message.insert(kSection, std::to_underlying(section_));
        message.insert(kSessionId, QString());

        model_->Finalize(message);

        message.insert(kWidgetId, widget_id_.toString(QUuid::WithoutBraces));
        message.insert(kParentWidgetId, parent_widget_id_.toString(QUuid::WithoutBraces));
        message.insert(kSettlementId, settlement_.id.toString(QUuid::WithoutBraces));

        if (settlement_.sync_state == SyncState::kSynced) {
            pending_update_.insert(kStatus, std::to_underlying(SettlementStatus::kSettled));
            pending_update_.insert(kAmount, QString::number(settlement_.amount, 'f', numeric_const::kDecimalPlaces4));
            pending_update_.insert(kVersion, settlement_.version);

            message.insert(kSettlement, pending_update_);

            WebSocket::Instance()->SendMessage(WsKey::kSettlementUpdate, message);
            pending_update_ = QJsonObject();
        }

        if (settlement_.sync_state == SyncState::kCreating) {
            settlement_.status = SettlementStatus::kSettled;
            message.insert(kSettlement, settlement_.WriteJson());
            WebSocket::Instance()->SendMessage(WsKey::kSettlementInsert, message);
        }

        settlement_.sync_state = SyncState::kUpdating;
    }
}

void SettlementSecondaryWidget::on_pBtnRecall_clicked()
{
    Q_ASSERT(settlement_.status == SettlementStatus::kSettled);

    if (!ValidateSyncState())
        return;

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section_));
    message.insert(kSessionId, QString());

    model_->Finalize(message);

    pending_update_.insert(kStatus, std::to_underlying(SettlementStatus::kUnsettled));
    pending_update_.insert(kVersion, settlement_.version);

    message.insert(kWidgetId, widget_id_.toString(QUuid::WithoutBraces));
    message.insert(kParentWidgetId, parent_widget_id_.toString(QUuid::WithoutBraces));
    message.insert(kSettlementId, settlement_.id.toString(QUuid::WithoutBraces));
    message.insert(kSettlement, pending_update_);

    WebSocket::Instance()->SendMessage(WsKey::kSettlementRecall, message);
    pending_update_ = QJsonObject();

    settlement_.sync_state = SyncState::kUpdating;
}

void SettlementSecondaryWidget::InsertSucceeded(int version)
{
    ui->comboPartner->setEnabled(false);
    UpdateSucceeded(version);
}

void SettlementSecondaryWidget::RecallSucceeded(int version)
{
    ui->dSpinAmount->setValue(0.0);
    settlement_.amount = 0.0;

    settlement_.version = version;
    settlement_.status = SettlementStatus::kUnsettled;

    settlement_.sync_state = SyncState::kSynced;
    model_->UpdateStatus(SettlementStatus::kUnsettled);

    LockWidget(false);
}

void SettlementSecondaryWidget::UpdateSucceeded(int version)
{
    settlement_.version = version;
    settlement_.status = SettlementStatus::kSettled;

    settlement_.sync_state = SyncState::kSynced;
    model_->UpdateStatus(SettlementStatus::kSettled);

    ui->tableView->clearSelection();
    LockWidget(true);
}
