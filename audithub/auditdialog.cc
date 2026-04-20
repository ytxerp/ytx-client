#include "auditdialog.h"

#include "component/constant.h"
#include "component/constantint.h"
#include "component/signalblocker.h"
#include "global/logininfo.h"
#include "ui_auditdialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

AuditDialog::AuditDialog(const audit_hub::AuditInfo& info, CUuid& widget_id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AuditDialog)
    , info_ { info }
    , model_ { new audit_hub::AuditModel(info, this) }
    , start_ { QDateTime(QDate::currentDate().addDays(-7), kStartTime) }
    , end_ { QDateTime(QDate::currentDate().addDays(1), kStartTime) }
    , widget_id_ { widget_id }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model_);
    model_->setParent(ui->tableView);

    InitDialog();
    InitTimer();

    QTimer::singleShot(0, this, &AuditDialog::on_pBtnFetch_clicked);
}

AuditDialog::~AuditDialog() { delete ui; }

QTableView* AuditDialog::View() { return ui->tableView; }

void AuditDialog::on_pBtnFetch_clicked()
{
    if (!ui->pBtnFetch->isEnabled()) {
        return;
    }

    ui->pBtnFetch->setEnabled(false);

    const auto message { JsonGen::AuditLogAck(widget_id_, LoginInfo::Instance().Workspace(), start_.toUTC(), end_.toUTC()) };
    WebSocket::Instance()->SendMessage(WsKey::kAuditLogAck, message);

    cooldown_timer_->start(TimeConst::kCooldownMs);
}

void AuditDialog::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}

void AuditDialog::on_dateTimeEditStart_dateChanged(const QDate& date)
{
    const bool valid { date < end_.date() };
    start_ = QDateTime(date, kStartTime);

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void AuditDialog::on_dateTimeEditEnd_dateChanged(const QDate& date)
{
    const bool valid { date >= start_.date() };

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
    end_ = QDateTime(date.addDays(1), kStartTime);
}

void AuditDialog::InitDialog()
{
    ui->dateTimeEditStart->setDisplayFormat(kDateFST);
    ui->dateTimeEditEnd->setDisplayFormat(kDateFST);

    ui->pBtnFetch->setFocus();

    ui->dateTimeEditStart->setDateTime(start_);
    ui->dateTimeEditEnd->setDateTime(end_.addDays(-1));
}
