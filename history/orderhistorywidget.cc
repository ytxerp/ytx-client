#include "orderhistorywidget.h"

#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "ui_orderhistorywidget.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

OrderHistoryWidget::OrderHistoryWidget(history::OrderModel* model, Section section, CUuid& widget_id, CUuid& node_id, NodeUnit node_unit, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::OrderHistoryWidget)
    , range_ { DefaultRange() }
    , model_ { model }
    , node_id_ { node_id }
    , widget_id_ { widget_id }
    , node_unit_ { node_unit }
    , section_ { section }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model);
    model->setParent(ui->tableView);

    InitWidget();
    InitTimer();

    QTimer::singleShot(0, this, &OrderHistoryWidget::on_pBtnFetch_clicked);
}

OrderHistoryWidget::~OrderHistoryWidget() { delete ui; }

QTableView* OrderHistoryWidget::View() const { return ui->tableView; }

void OrderHistoryWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= range_.end };
    range_.start = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void OrderHistoryWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= range_.start };
    range_.end = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void OrderHistoryWidget::on_pBtnFetch_clicked()
{
    if (!ui->pBtnFetch->isEnabled()) {
        return;
    }

    if (!range_.IsValid()) {
        return;
    }

    ui->pBtnFetch->setEnabled(false);

    qDebug() << Q_FUNC_INFO << "DateRange:" << range_.ToString();

    const auto query_range { range_.ToQueryRange() };

    qDebug() << Q_FUNC_INFO << "QueryRange:" << query_range.ToString();

    const auto message { JsonGen::OrderHistory(section_, widget_id_, node_id_, node_unit_, query_range) };
    WebSocket::Instance()->SendMessage(WsKey::kOrderHistory, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void OrderHistoryWidget::InitWidget()
{
    ui->start->setDisplayFormat(datetime_format::kDashedDate);
    ui->end->setDisplayFormat(datetime_format::kDashedDate);
    ui->start->setDate(range_.start);
    ui->end->setDate(range_.end);

    ui->pBtnFetch->setFocus();
}

void OrderHistoryWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}
