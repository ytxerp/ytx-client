#include "tablewidgetsettlement.h"

#include <QMessageBox>

#include "component/signalblocker.h"
#include "enum/settlementenum.h"
#include "ui_tablewidgetsettlement.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableWidgetSettlement::TableWidgetSettlement(CSectionConfig& config, TreeModel* tree_model_p, TableModelSettlement* model, const Settlement& settlement,
    Section section, CUuid& widget_id, CUuid& parent_widget_id, SyncState sync_state, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TableWidgetSettlement)
    , settlement_ { settlement }
    , model_ { model }
    , config_ { config }
    , tree_model_p_ { tree_model_p }
    , widget_id_ { widget_id }
    , parent_widget_id_ { parent_widget_id }
    , section_ { section }
    , sync_state_ { sync_state }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model);
    model->setParent(ui->tableView);

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
    auto* pmodel { tree_model_p_->IncludeUnitModel(section_ == Section::kSale ? NodeUnit::PCustomer : NodeUnit::PVendor, this) };
    ui->comboPartner->setModel(pmodel);
    ui->comboPartner->setCurrentIndex(-1);

    ui->dateTimeEdit->setDisplayFormat(kDateTimeFST);
    ui->dSpinAmount->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
    ui->dSpinAmount->setDecimals(config_.amount_decimal);

    Utils::SetButton(ui->pBtnRelease, tr("Release"), QKeySequence(Qt::CTRL | Qt::Key_Return));
    Utils::SetButton(ui->pBtnRecall, tr("Recall"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
}

void TableWidgetSettlement::InitData()
{
    const int partner_index { ui->comboPartner->findData(settlement_.partner_id) };
    ui->comboPartner->setCurrentIndex(partner_index);

    ui->lineDescription->setText(settlement_.description);
    ui->dateTimeEdit->setDateTime(settlement_.issued_time.toLocalTime());
    ui->dSpinAmount->setValue(settlement_.amount);

    ui->comboPartner->setEnabled(sync_state_ == SyncState::kLocalOnly);

    const bool is_settled { settlement_.status == SettlementStatus::kSettled };
    ui->lineDescription->setReadOnly(is_settled);
    ui->dateTimeEdit->setReadOnly(is_settled);

    HideWidget(is_settled);
}

void TableWidgetSettlement::FetchNode()
{
    if (settlement_.partner_id.isNull())
        return;

    const auto message { JsonGen::SettlementNodeAcked(section_, widget_id_, settlement_.partner_id, settlement_.id) };
    WebSocket::Instance()->SendMessage(kSettlementItemAcked, message);
}

void TableWidgetSettlement::HideWidget(bool is_settled)
{
    ui->pBtnRelease->setVisible(!is_settled);
    ui->pBtnRecall->setVisible(is_settled);
}

bool TableWidgetSettlement::ValidateSyncState()
{
    if (sync_state_ == SyncState::kOutOfSync) {
        QMessageBox::information(
            this, tr("Invalid Operation"), tr("The operation you attempted is invalid because your local data is outdated. Please refresh and try again."));
        return false;
    }

    qDebug() << "[ValidateSyncState] Passed: sync_state =" << static_cast<int>(sync_state_);

    return true;
}

void TableWidgetSettlement::on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime)
{
    const QDateTime utc_time { dateTime.toUTC() };
    if (settlement_.issued_time == utc_time)
        return;

    settlement_.issued_time = dateTime.toUTC();

    if (sync_state_ == SyncState::kSynced)
        pending_update_.insert(kIssuedTime, utc_time.toString(Qt::ISODate));
}

void TableWidgetSettlement::on_lineDescription_textChanged(const QString& arg1)
{
    settlement_.description = arg1;

    if (sync_state_ == SyncState::kSynced)
        pending_update_.insert(kDescription, arg1);
}

void TableWidgetSettlement::on_comboPartner_currentIndexChanged(int /*index*/)
{
    if (sync_state_ == SyncState::kSynced)
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

void TableWidgetSettlement::on_pBtnRelease_clicked()
{
    Q_ASSERT(!settlement_.partner_id.isNull());
    Q_ASSERT(settlement_.status != SettlementStatus::kSettled);

    model_->NormalizeBuffer();

    {
        if (!model_->HasSelected() && !model_->HasPendingUpdate()) {
            return;
        }

        if (!ValidateSyncState())
            return;
    }

    {
        QJsonObject message { JsonGen::MetaMessage(section_) };
        model_->Finalize(message);

        message.insert(kWidgetId, widget_id_.toString(QUuid::WithoutBraces));
        message.insert(kParentWidgetId, parent_widget_id_.toString(QUuid::WithoutBraces));
        message.insert(kSettlementId, settlement_.id.toString(QUuid::WithoutBraces));

        if (sync_state_ == SyncState::kSynced) {
            pending_update_.insert(kStatus, std::to_underlying(SettlementStatus::kSettled));
            pending_update_.insert(kAmount, QString::number(settlement_.amount, 'f', kMaxNumericScale_4));
            pending_update_.insert(kVersion, settlement_.version);

            message.insert(kSettlement, pending_update_);

            WebSocket::Instance()->SendMessage(kSettlementUpdated, message);
            pending_update_ = QJsonObject();
        }

        if (sync_state_ == SyncState::kLocalOnly) {
            message.insert(kSettlement, settlement_.WriteJson());
            WebSocket::Instance()->SendMessage(kSettlementInserted, message);
        }

        sync_state_ = SyncState::kOutOfSync;
    }
}

void TableWidgetSettlement::on_pBtnRecall_clicked()
{
    Q_ASSERT(settlement_.status != SettlementStatus::kUnsettled);

    if (!ValidateSyncState())
        return;

    QJsonObject message { JsonGen::MetaMessage(section_) };
    model_->Finalize(message);

    pending_update_.insert(kStatus, std::to_underlying(SettlementStatus::kUnsettled));
    pending_update_.insert(kVersion, settlement_.version);

    message.insert(kWidgetId, widget_id_.toString(QUuid::WithoutBraces));
    message.insert(kParentWidgetId, parent_widget_id_.toString(QUuid::WithoutBraces));
    message.insert(kSettlementId, settlement_.id.toString(QUuid::WithoutBraces));
    message.insert(kSettlement, pending_update_);

    WebSocket::Instance()->SendMessage(kSettlementRecalled, message);
    pending_update_ = QJsonObject();

    sync_state_ = SyncState::kOutOfSync;
}

void TableWidgetSettlement::InsertSucceeded(int version)
{
    ui->comboPartner->setEnabled(false);
    UpdateSucceeded(version);
}

void TableWidgetSettlement::RecallSucceeded(int version)
{
    ui->lineDescription->setReadOnly(false);
    ui->dateTimeEdit->setReadOnly(false);

    settlement_.version = version;
    sync_state_ = SyncState::kSynced;
    settlement_.status = SettlementStatus::kUnsettled;
    model_->UpdateStatus(SettlementStatus::kUnsettled);

    HideWidget(false);
}

void TableWidgetSettlement::UpdateSucceeded(int version)
{
    ui->lineDescription->setReadOnly(true);
    ui->dateTimeEdit->setReadOnly(true);

    settlement_.version = version;
    sync_state_ = SyncState::kSynced;
    settlement_.status = SettlementStatus::kSettled;
    model_->UpdateStatus(SettlementStatus::kSettled);

    ui->tableView->clearSelection();
    HideWidget(true);
}
