#include "tablewidgetr.h"

#include <QTimer>

#include "component/signalblocker.h"
#include "ui_tablewidgetr.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableWidgetR::TableWidgetR(
    reimbursement::TableModelR* model, CSectionConfig& config, const reimbursement::PrimaryRow& row, QUuid widget_id, int finance_permission, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TableWidgetR)
    , model_ { model }
    , primary_row_ { row }
    , widget_id_ { widget_id }
    , finance_permission_ { finance_permission }
    , config_ { config }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model);
    model->setParent(ui->tableView);

    InitWidget();
    InitData();
    UpdateButtons();

    QTimer::singleShot(0, this, [this]() { FetchNode(); });
}

TableWidgetR::~TableWidgetR() { delete ui; }

QTableView* TableWidgetR::View() const { return ui->tableView; }

void TableWidgetR::ApproveSucceeded(int version) { }

void TableWidgetR::UnapproveSucceeded(int version) { }

void TableWidgetR::SaveSucceeded(int version) { }

void TableWidgetR::PostSucceeded(int version) { }

void TableWidgetR::UnpostSucceeded(int version) { }

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
    ui->lineEditDescription->setText(primary_row_.description);
    ui->dateTimeEdit->setDateTime(primary_row_.issued_time);
    ui->dSpinInitialTotal->setValue(primary_row_.initial_total);
}

void TableWidgetR::FetchNode()
{
    if (primary_row_.sync_state != SyncState::kSynced)
        return;

    const auto message { JsonGen::ReimbursementSecondary(widget_id_, primary_row_.id) };
    WebSocket::Instance()->SendMessage(WsKey::kReimbursementSecondary, message);
}

void TableWidgetR::UpdateButtons()
{
    using reimbursement::Status;

    const bool ready { primary_row_.sync_state == SyncState::kSynced && !model_->HasPendingUpdate() && pending_update_.isEmpty() };

    ui->pushButtonApprove->setEnabled(ready && primary_row_.status == Status::kDraft && (finance_permission_ & finance_permission::kApprove));

    ui->pushButtonUnapprove->setEnabled(ready && primary_row_.status == Status::kApproved && (finance_permission_ & finance_permission::kUnapprove));

    ui->pushButtonPost->setEnabled(ready && primary_row_.status == Status::kApproved && (finance_permission_ & finance_permission::kPost));

    ui->pushButtonUnpost->setEnabled(ready && primary_row_.status == Status::kPosted && (finance_permission_ & finance_permission::kUnpost));
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
    if (primary_row_.sync_state == SyncState::kUpdating) {
        utils::ShowMessage(
            QMessageBox::Information, tr("Operation Rejected"), tr("The data is being updated. Please refresh and try again."), time_const::kAutoCloseMs);
        return;
    }

    model_->Purify();

    if (!model_->HasPendingUpdate() && pending_update_.isEmpty())
        return;

    QJsonObject message {};

    model_->Finalize(message);

    if (primary_row_.sync_state == SyncState::kSynced) {
        message.insert(kNodeId, primary_row_.id.toString(QUuid::WithoutBraces));
        message.insert(kVersion, primary_row_.version);
        message.insert(kNodeUpdate, pending_update_);
        WebSocket::Instance()->SendMessage(WsKey::kReimbursementUpdate, message);
    }

    if (primary_row_.sync_state == SyncState::kCreating) {
        message.insert(kNode, primary_row_.WriteJson());
        WebSocket::Instance()->SendMessage(WsKey::kReimbursementInsert, message);
    }

    qInfo() << Q_FUNC_INFO << primary_row_.version;

    primary_row_.sync_state = SyncState::kUpdating;
}

void TableWidgetR::on_pushButtonApprove_clicked() { }

void TableWidgetR::on_pushButtonPost_clicked() { }

void TableWidgetR::on_pushButtonUnapprove_clicked() { }

void TableWidgetR::on_pushButtonUnpost_clicked() { }

void TableWidgetR::on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime)
{
    if (primary_row_.issued_time == dateTime)
        return;

    primary_row_.issued_time = dateTime;

    if (primary_row_.sync_state == SyncState::kSynced)
        pending_update_.insert(kIssuedTime, dateTime.toUTC().toString(Qt::ISODate));

    UpdateButtons();
}

void TableWidgetR::on_lineEditDescription_textEdited(const QString& arg1)
{
    if (primary_row_.description == arg1)
        return;

    primary_row_.description = arg1;

    if (primary_row_.sync_state == SyncState::kSynced)
        pending_update_.insert(kDescription, arg1);

    UpdateButtons();
}
