#include "treewidgetr.h"

#include <QTimer>

#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "ui_treewidgetr.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TreeWidgetR::TreeWidgetR(QUuid widget_id, const QStringList& header, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TreeWidgetR)
    , range_ { DefaultRange() }
    , widget_id_ { widget_id }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    InitWidget();
    InitTimer();
    InitModel(header);

    QTimer::singleShot(0, this, &TreeWidgetR::on_pushButtonFetch_clicked);
}

TreeWidgetR::~TreeWidgetR() { delete ui; }

QTableView* TreeWidgetR::View() const { return ui->tableView; }

void TreeWidgetR::InitWidget()
{
    ui->start->setDisplayFormat(datetime_format::kDashedDate);
    ui->end->setDisplayFormat(datetime_format::kDashedDate);

    ui->pushButtonFetch->setFocus();

    ui->start->setDate(range_.start);
    ui->end->setDate(range_.end);

    utils::SetPushButton(ui->pushButtonFetch, QKeySequence(Qt::CTRL | Qt::Key_R));
}

void TreeWidgetR::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pushButtonFetch->setEnabled(true); });
}

void TreeWidgetR::InitModel(const QStringList& header)
{
    model_ = new reimbursement::TreeModelR(header, this);
    ui->tableView->setModel(model_);
}

void TreeWidgetR::on_pushButtonFetch_clicked()
{
    if (!ui->pushButtonFetch->isEnabled()) {
        return;
    }

    if (!range_.IsValid()) {
        return;
    }

    ui->pushButtonFetch->setEnabled(false);

    qDebug() << Q_FUNC_INFO << "DateRange:" << range_.ToString();

    const auto query_range { range_.ToQueryRange() };

    qDebug() << Q_FUNC_INFO << "QueryRange:" << query_range.ToString();

    const auto message { JsonGen::ReimbursementPrimary(widget_id_, query_range) };
    WebSocket::Instance()->SendMessage(WsKey::kReimbursementPrimary, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void TreeWidgetR::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= range_.end };
    range_.start = date;

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}

void TreeWidgetR::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= range_.start };
    range_.end = date;

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}

void TreeWidgetR::on_pushButtonDelete_clicked()
{
    auto* view { ui->tableView };
    Q_ASSERT(view != nullptr);

    if (!utils::HasSelection(view))
        return;

    const QModelIndex index { view->currentIndex() };
    if (!index.isValid())
        return;

    auto* row { static_cast<reimbursement::PrimaryRow*>(index.internalPointer()) };

    if (row->status != reimbursement::Status::kDraft) {
        utils::ShowMessage(QMessageBox::Information, tr("Operation Rejected"), tr("Only draft reimbursements can be deleted."), time_const::kAutoCloseMs);
        return;
    }

    model_->removeRows(index.row(), 1);
}

void TreeWidgetR::on_pushButtonInsert_clicked() { emit SCreateReimbursementSecondaryWidget(model_); }
