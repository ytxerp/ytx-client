#include "tablewidgetr.h"

#include <QTimer>

#include "component/signalblocker.h"
#include "ui_tablewidgetr.h"
#include "utils/mainwindowutils.h"
#include "websocket/websocket.h"

TableWidgetR::TableWidgetR(TableModel* model, NodeR* node, CSectionConfig& section_config, int finance_permission, QWidget* parent)
    : TableWidget(parent)
    , ui(new Ui::TableWidgetR)
    , node_ { node }
    , model_ { qobject_cast<TableModelR*>(model) }
    , finance_permission_ { finance_permission }
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

TableWidgetR::~TableWidgetR() { delete ui; }

void TableWidgetR::ApproveSucceeded(int version) { }

void TableWidgetR::UnapproveSucceeded(int version) { }

void TableWidgetR::SyncSucceeded(int version)
{
    qDebug() << Q_FUNC_INFO;
    model_->SyncSucceeded();

    pending_update_ = QJsonObject();
    node_->version = version;

    node_->sync_state = SyncState::kSynced;

    UpdateButtons();
}

void TableWidgetR::PostSucceeded(int version) { }

void TableWidgetR::UnpostSucceeded(int version) { }

void TableWidgetR::PermissionDenied()
{
    qDebug() << Q_FUNC_INFO;
    node_->sync_state = SyncState::kSynced;
}

QTableView* TableWidgetR::View() const { return ui->tableView; }

bool TableWidgetR::HasPendingUpdate() const
{
    const bool node_pending { !pending_update_.isEmpty() };
    const bool entry_pending { model_->HasPendingUpdate() };

    qDebug() << "[HasPendingUpdate]"
             << "node_pending =" << node_pending << ", entry_pending =" << entry_pending << ", result =" << (node_pending || entry_pending);

    return node_pending || entry_pending;
}

void TableWidgetR::RSyncInitial(QUuid node_id, double initial_delta)
{
    Q_ASSERT(node_id_ == node_id && "RSyncDelta called with mismatched node_id");
    node_->initial_total += initial_delta;
    ui->dSpinInitialTotal->setValue(node_->initial_total);
}

void TableWidgetR::RDataChanged() { ui->pushButtonApprove->setEnabled(false); }

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
    const bool ready { node_->sync_state == SyncState::kSynced && !model_->HasPendingUpdate() && pending_update_.isEmpty() };

    ui->pushButtonApprove->setEnabled(
        ready && node_->reimbursement_status == ReimbursementStatus::kDraft && (finance_permission_ & finance_permission::kApprove));

    ui->pushButtonUnapprove->setEnabled(
        ready && node_->reimbursement_status == ReimbursementStatus::kApproved && (finance_permission_ & finance_permission::kUnapprove));

    ui->pushButtonPost->setEnabled(ready && node_->reimbursement_status == ReimbursementStatus::kApproved && (finance_permission_ & finance_permission::kPost));

    ui->pushButtonUnpost->setEnabled(
        ready && node_->reimbursement_status == ReimbursementStatus::kPosted && (finance_permission_ & finance_permission::kUnpost));
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

void TableWidgetR::on_pushButtonDelete_clicked()
{
    auto* view { ui->tableView };

    if (!utils::HasSelection(view))
        return;

    const QModelIndex index { view->currentIndex() };
    if (!index.isValid())
        return;

    if (model_->removeRow(index.row()))
        UpdateButtons();
}

void TableWidgetR::on_pushButtonInsert_clicked()
{
    const int row { model_->rowCount() };

    if (model_->insertRow(row))
        UpdateButtons();
}

void TableWidgetR::on_pushButtonSave_clicked()
{
    if (node_->sync_state == SyncState::kUpdating) {
        utils::ShowMessage(
            QMessageBox::Information, tr("Operation Rejected"), tr("The data is being updated. Please refresh and try again."), time_const::kAutoCloseMs);
        return;
    }

    model_->Purify();

    if (!model_->HasPendingUpdate() && pending_update_.isEmpty())
        return;

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(Section::kReimbursement));

    model_->Finalize(message);

    if (node_->sync_state == SyncState::kSynced) {
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

void TableWidgetR::on_pushButtonApprove_clicked() { }

void TableWidgetR::on_pushButtonPost_clicked() { }

void TableWidgetR::on_pushButtonUnapprove_clicked() { }

void TableWidgetR::on_pushButtonUnpost_clicked() { }

void TableWidgetR::on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime)
{
    if (node_->reimbursement_status != ReimbursementStatus::kDraft || node_->sync_state == SyncState::kUpdating)
        return;

    if (node_->issued_time == dateTime)
        return;

    node_->issued_time = dateTime;

    if (node_->sync_state == SyncState::kSynced)
        pending_update_.insert(kIssuedTime, dateTime.toUTC().toString(Qt::ISODate));

    UpdateButtons();
}

void TableWidgetR::on_lineEditDescription_textEdited(const QString& arg1)
{
    if (node_->reimbursement_status != ReimbursementStatus::kDraft || node_->sync_state == SyncState::kUpdating)
        return;

    if (node_->description == arg1)
        return;

    node_->description = arg1;

    if (node_->sync_state == SyncState::kSynced)
        pending_update_.insert(kDescription, arg1);

    UpdateButtons();
}
