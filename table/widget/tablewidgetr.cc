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

void TableWidgetR::SaveSucceeded(int version) { }

void TableWidgetR::PostSucceeded(int version) { }

void TableWidgetR::UnpostSucceeded(int version) { }

QTableView* TableWidgetR::View() const { return ui->tableView; }

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

    model_->Finalize(message);

    if (node_->sync_state == SyncState::kSynced) {
        message.insert(kNodeId, node_->id.toString(QUuid::WithoutBraces));
        message.insert(kVersion, node_->version);
        message.insert(kNodeUpdate, pending_update_);
        WebSocket::Instance()->SendMessage(WsKey::kReimbursementUpdate, message);
    }

    if (node_->sync_state == SyncState::kCreating) {
        message.insert(kNode, node_->WriteJson());
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
    if (node_->issued_time == dateTime)
        return;

    node_->issued_time = dateTime;

    if (node_->sync_state == SyncState::kSynced)
        pending_update_.insert(kIssuedTime, dateTime.toUTC().toString(Qt::ISODate));

    UpdateButtons();
}

void TableWidgetR::on_lineEditDescription_textEdited(const QString& arg1)
{
    if (node_->description == arg1)
        return;

    node_->description = arg1;

    if (node_->sync_state == SyncState::kSynced)
        pending_update_.insert(kDescription, arg1);

    UpdateButtons();
}
