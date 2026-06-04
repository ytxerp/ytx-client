#include "settlementprimarywidget.h"

#include <QJsonArray>
#include <QTimer>

#include "component/constant.h"
#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "ui_settlementprimarywidget.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

SettlementPrimaryWidget::SettlementPrimaryWidget(SettlementPrimaryModel* model, CUuid& widget_id, Section section, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettlementPrimaryWidget)
    , model_ { model }
    , start_ { QDateTime(QDate(QDate::currentDate().year() - 1, 1, 1), kStartTime) }
    , end_ { QDateTime(QDate(QDate::currentDate().year() + 1, 1, 1), kStartTime) }
    , section_ { section }
    , widget_id_ { widget_id }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model);
    model->setParent(ui->tableView);

    IniWidget();
    InitTimer();

    QTimer::singleShot(0, this, &SettlementPrimaryWidget::on_pBtnFetch_clicked);
}

SettlementPrimaryWidget::~SettlementPrimaryWidget() { delete ui; }

QTableView* SettlementPrimaryWidget::View() const { return ui->tableView; }

void SettlementPrimaryWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date < end_.date() };
    start_ = QDateTime(date, kStartTime);

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void SettlementPrimaryWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= start_.date() };

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
    end_ = QDateTime(date.addDays(1), kStartTime);
}

void SettlementPrimaryWidget::on_pBtnFetch_clicked()
{
    if (!ui->pBtnFetch->isEnabled()) {
        return;
    }

    ui->pBtnFetch->setEnabled(false);

    const auto message { JsonGen::SettlementAck(section_, widget_id_, start_.toUTC(), end_.toUTC()) };
    WebSocket::Instance()->SendMessage(WsKey::kSettlementAck, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void SettlementPrimaryWidget::IniWidget()
{
    ui->start->setDisplayFormat(datetime_format::kDashedDate);
    ui->end->setDisplayFormat(datetime_format::kDashedDate);

    ui->pBtnFetch->setFocus();

    ui->start->setDateTime(start_);
    ui->end->setDateTime(end_.addDays(-1));
}

void SettlementPrimaryWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}
