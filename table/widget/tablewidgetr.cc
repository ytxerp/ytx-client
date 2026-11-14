#include "tablewidgetr.h"

#include <QTimer>

#include "component/signalblocker.h"
#include "global/nodepool.h"
#include "ui_tablewidgetr.h"
#include "utils/mainwindowutils.h"
#include "websocket/websocket.h"
#include "workspace/sectionpermissions.h"

TableWidgetR::TableWidgetR(TableModel* model, NodeR* node, CSectionConfig& section_config, int permission, QWidget* parent)
    : TableWidget(parent)
    , ui(new Ui::TableWidgetR)
    , node_ { node }
    , original_node_ { *node }
    , model_ { qobject_cast<TableModelR*>(model) }
    , permission_ { permission }
    , node_id_ { node->id }
    , config_ { section_config }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model);
    model->setParent(ui->tableView);

    InitWidget();
    InitData();
    UpdateButtons();
}

TableWidgetR::~TableWidgetR()
{
    NodePool::Instance().Recycle(node_, Section::kReimbursement);
    delete ui;
}

void TableWidgetR::SyncSucceeded(int version)
{
    qDebug() << Q_FUNC_INFO;

    model_->SyncSucceeded();
    pending_update_ = QJsonObject();

    node_->version = version;
    node_->sync_state = SyncState::kSynced;

    original_node_ = *node_;

    ui->pushButtonSave->setEnabled(false);
    ui->pushButtonApprove->setEnabled(permission_ & reimbursement_permission::kApprove);
}

void TableWidgetR::StatusSucceeded(int status, int version)
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(status >= std::to_underlying(ReimbursementStatus::kDraft) && status <= std::to_underlying(ReimbursementStatus::kPosted));

    node_->reimbursement_status = static_cast<ReimbursementStatus>(status);
    node_->version = version;
    node_->sync_state = SyncState::kSynced;

    UpdateButtons();
}

void TableWidgetR::PermissionDenied()
{
    qDebug() << Q_FUNC_INFO;
    node_->sync_state = SyncState::kSynced;
}

QTableView* TableWidgetR::View() const { return ui->tableView; }

bool TableWidgetR::HasPendingUpdate() const
{
    const bool node_pending { original_node_ != *node_ };
    const bool entry_pending { model_->HasPendingUpdate() };

    qDebug() << "[HasPendingUpdate]"
             << "node_pending =" << node_pending << ", entry_pending =" << entry_pending << ", result =" << (node_pending || entry_pending);

    return node_pending || entry_pending;
}

void TableWidgetR::RSyncDelta(QUuid node_id, const entry::Delta& delta)
{
    Q_ASSERT(node_id_ == node_id && "RSyncDelta called with mismatched node_id");
    node_->initial_total += delta.initial;
    ui->dSpinInitialTotal->setValue(node_->initial_total);
}

void TableWidgetR::RDataChanged() { UpdateSaveApproveButtons(); }

void TableWidgetR::InitWidget()
{
    ui->dateTimeEdit->setDisplayFormat(datetime_format::kDateTime);
    ui->dSpinInitialTotal->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
    ui->dSpinInitialTotal->setDecimals(config_.amount_decimal);

    utils::SetPushButton(ui->pushButtonSave, QKeySequence::Save);
    utils::SetPushButton(ui->pushButtonApprove, QKeySequence(Qt::CTRL | Qt::Key_Return));
    utils::SetPushButton(ui->pushButtonUnapprove, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
}

void TableWidgetR::InitData()
{
    ui->lineEditDescription->setText(node_->description);
    ui->dateTimeEdit->setDateTime(node_->issued_time);
    ui->dSpinInitialTotal->setValue(node_->initial_total);
}

void TableWidgetR::UpdateButtons()
{
    const bool draft { node_->reimbursement_status == ReimbursementStatus::kDraft };
    const bool pending { model_->HasPendingUpdate() || original_node_ != *node_ };
    const bool no_pending { !pending };

    ui->pushButtonSave->setEnabled(draft && pending);
    ui->pushButtonApprove->setEnabled(no_pending && draft && node_->sync_state == SyncState::kSynced && (permission_ & reimbursement_permission::kApprove));
    ui->pushButtonUnapprove->setEnabled(node_->reimbursement_status == ReimbursementStatus::kApproved && (permission_ & reimbursement_permission::kUnapprove));
    ui->pushButtonPost->setEnabled(node_->reimbursement_status == ReimbursementStatus::kApproved && (permission_ & reimbursement_permission::kPost));
    ui->pushButtonUnpost->setEnabled(node_->reimbursement_status == ReimbursementStatus::kPosted && (permission_ & reimbursement_permission::kUnpost));

    ui->dateTimeEdit->setEnabled(draft);
    ui->lineEditDescription->setEnabled(draft);
}

void TableWidgetR::UpdateSaveApproveButtons()
{
    const bool pending { model_->HasPendingUpdate() || original_node_ != *node_ };
    const bool draft { node_->reimbursement_status == ReimbursementStatus::kDraft };

    ui->pushButtonSave->setEnabled(draft && pending);
    ui->pushButtonApprove->setEnabled(draft && !pending && node_->sync_state == SyncState::kSynced && (permission_ & reimbursement_permission::kApprove));
}

void TableWidgetR::BuildNodeInsert(QJsonObject& message)
{
    const QJsonObject node_json { node_->WriteJson() };

    QJsonObject path_json {};
    path_json.insert(kAncestor, node_->parent->id.toString(QUuid::WithoutBraces));
    path_json.insert(kDescendant, node_id_.toString(QUuid::WithoutBraces));

    message.insert(kNode, node_json);
    message.insert(kPath, path_json);
}

void TableWidgetR::BuildNodeUpdate(QJsonObject& message)
{
    if (node_->description != original_node_.description)
        message.insert(kDescription, node_->description);

    if (node_->issued_time != original_node_.issued_time)
        message.insert(kIssuedTime, node_->issued_time.toUTC().toString(Qt::ISODate));

    message.insert(kInitialTotal, QString::number(node_->initial_total, 'f', numeric_const::kDecimalPlaces4));
}

void TableWidgetR::on_pushButtonSave_clicked()
{
    if (node_->reimbursement_status != ReimbursementStatus::kDraft)
        return;

    if (!ValidateSyncState())
        return;

    model_->Purify();

    if (!model_->HasPendingUpdate() && original_node_ == *node_) {
        ui->pushButtonApprove->setEnabled(permission_ & reimbursement_permission::kApprove);
        return;
    }

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(Section::kReimbursement));

    model_->Finalize(message);

    if (node_->sync_state == SyncState::kSynced) {
        pending_update_ = QJsonObject();
        BuildNodeUpdate(pending_update_);

        message.insert(kNodeId, node_->id.toString(QUuid::WithoutBraces));
        message.insert(kVersion, node_->version);
        message.insert(kNodeUpdate, pending_update_);
        WebSocket::Instance()->SendMessage(WsKey::kReimbursementUpdate, message);
    }

    if (node_->sync_state == SyncState::kCreating) {
        BuildNodeInsert(message);
        WebSocket::Instance()->SendMessage(WsKey::kReimbursementInsert, message);
    }

    qInfo() << Q_FUNC_INFO << node_->version;

    node_->sync_state = SyncState::kUpdating;
}

void TableWidgetR::on_pushButtonApprove_clicked()
{
    Q_ASSERT(node_->reimbursement_status == ReimbursementStatus::kDraft && (permission_ & reimbursement_permission::kApprove));
    Q_ASSERT(!model_->HasPendingUpdate() && original_node_ == *node_);

    if (!ValidateSyncState())
        return;

    WebSocket::Instance()->SendMessage(WsKey::kReimbursementApprove, BuildStatusMessage());
    node_->sync_state = SyncState::kUpdating;
}

void TableWidgetR::on_pushButtonPost_clicked() { }

void TableWidgetR::on_pushButtonUnapprove_clicked()
{
    Q_ASSERT(node_->reimbursement_status == ReimbursementStatus::kApproved && (permission_ & reimbursement_permission::kUnapprove));
    Q_ASSERT(!model_->HasPendingUpdate() && original_node_ == *node_);

    if (!ValidateSyncState())
        return;

    WebSocket::Instance()->SendMessage(WsKey::kReimbursementUnapprove, BuildStatusMessage());
    node_->sync_state = SyncState::kUpdating;
}

void TableWidgetR::on_pushButtonUnpost_clicked() { }

void TableWidgetR::on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime)
{
    if (node_->reimbursement_status != ReimbursementStatus::kDraft || node_->sync_state == SyncState::kUpdating)
        return;

    if (node_->issued_time == dateTime)
        return;

    node_->issued_time = dateTime;

    UpdateSaveApproveButtons();
}

void TableWidgetR::on_lineEditDescription_textEdited(const QString& arg1)
{
    if (node_->reimbursement_status != ReimbursementStatus::kDraft || node_->sync_state == SyncState::kUpdating)
        return;

    if (node_->description == arg1)
        return;

    node_->description = arg1;

    UpdateSaveApproveButtons();
}

QJsonObject TableWidgetR::BuildStatusMessage() const
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(Section::kReimbursement));
    message.insert(kNodeId, node_->id.toString(QUuid::WithoutBraces));
    message.insert(kVersion, node_->version);
    message.insert(kStatus, std::to_underlying(node_->reimbursement_status));

    return message;
}

bool TableWidgetR::ValidateSyncState() const
{
    if (node_->sync_state == SyncState::kUpdating) {
        utils::ShowMessage(
            QMessageBox::Information, tr("Operation Rejected"), tr("The data is being updated. Please refresh and try again."), time_const::kAutoCloseMs);
        return false;
    }

    qDebug() << Q_FUNC_INFO << static_cast<int>(node_->sync_state);

    return true;
}
