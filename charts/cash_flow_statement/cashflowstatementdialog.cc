#include "cashflowstatementdialog.h"

#include "component/constant.h"
#include "component/constantint.h"
#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "ui_cashflowstatementdialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

CashFlowStatementDialog::CashFlowStatementDialog(CashFlowStatementModel* model, const QUuid& widget_id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::CashFlowStatementDialog)
    , start_ { QDateTime(QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1), kStartTime) }
    , end_ { QDateTime(QDate::currentDate().addDays(1), kStartTime) }
    , widget_id_ { widget_id }
    , model_ { model }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    InitTimer();
    InitDialog();

    ui->treeView->setModel(model);
    model->setParent(ui->treeView);

    QTimer::singleShot(0, this, &::CashFlowStatementDialog::on_pushButtonFetch_clicked);
}

CashFlowStatementDialog::~CashFlowStatementDialog() { delete ui; }

QTreeView* CashFlowStatementDialog::View() { return ui->treeView; }

void CashFlowStatementDialog::on_dateTimeEditEnd_dateChanged(const QDate& date)
{
    const bool valid { date >= start_.date() };

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);

    end_ = QDateTime(date.addDays(1), kStartTime);
}

void CashFlowStatementDialog::on_dateTimeEditStart_dateChanged(const QDate& date)
{
    const bool valid { date < end_.date() };
    start_ = QDateTime(date, kStartTime);

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}

void CashFlowStatementDialog::on_pushButtonFetch_clicked()
{
    if (!ui->pushButtonFetch->isEnabled()) {
        return;
    }

    const auto message { JsonGen::CashFlowStatementAck(widget_id_, start_.toUTC(), end_.toUTC()) };

    WebSocket::Instance()->SendMessage(WsKey::kCashFlowStatementAck, message);

    ui->pushButtonFetch->setEnabled(false);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void CashFlowStatementDialog::InitDialog()
{
    ui->dateTimeEditStart->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateTimeEditEnd->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateTimeEditStart->setDateTime(start_);
    ui->dateTimeEditEnd->setDateTime(end_.addDays(-1));

    ui->pushButtonFetch->setFocus();
}

void CashFlowStatementDialog::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pushButtonFetch->setEnabled(true); });
}
