#include "orderoverviewwidget.h"

#include <QTimer>

#include "component/constantint.h"
#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "ui_orderoverviewwidget.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

OrderOverviewWidget::OrderOverviewWidget(const QStringList& header, const QUuid widget_id, const Section section, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::OrderOverviewWidget)
    , range_ { DefaultRange() }
    , widget_id_ { widget_id }
    , section_ { section }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    InitTimer();
    InitWidget();
    InitModel(header);

    QTimer::singleShot(0, this, &OrderOverviewWidget::on_pBtnFetch_clicked);
}

OrderOverviewWidget::~OrderOverviewWidget() { delete ui; }

QTableView* OrderOverviewWidget::OverviewView() const { return ui->tableViewOverview; }

QTableView* OrderOverviewWidget::FilterView() const { return ui->tableViewFilter; }

void OrderOverviewWidget::InitWidget()
{
    ui->start->setDisplayFormat(datetime_format::kDashedDate);
    ui->end->setDisplayFormat(datetime_format::kDashedDate);
    ui->start->setDate(range_.start);
    ui->end->setDate(range_.end);
}

void OrderOverviewWidget::InitModel(const QStringList& header)
{
    overview_model_ = new order_overview::Model(header, this);
    filter_model_ = new TableFilterModel(header, std::to_underlying(order_overview::RowField::kPlaceholder), this);
    filter_proxy_ = new TableFilterProxyModel(this);

    filter_proxy_->setSourceModel(overview_model_);

    ui->tableViewOverview->setModel(filter_proxy_);
    ui->tableViewFilter->setModel(filter_model_);

    connect(filter_model_, &TableFilterModel::SFilterChanged, filter_proxy_, &TableFilterProxyModel::RFilterChanged);
    connect(filter_model_, &TableFilterModel::SFiltersCleared, filter_proxy_, &TableFilterProxyModel::RFiltersCleared);
    connect(filter_model_, &TableFilterModel::SSortRequested, filter_proxy_, &TableFilterProxyModel::RSortRequested);

    ui->tableViewFilter->setSortingEnabled(true);
}

void OrderOverviewWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}

void OrderOverviewWidget::on_pBtnFetch_clicked()
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

    const auto message { JsonGen::OrderOverviewAck(section_, widget_id_, query_range) };
    WebSocket::Instance()->SendMessage(WsKey::kOrderOverview, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void OrderOverviewWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= range_.start };
    range_.end = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void OrderOverviewWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= range_.end };
    range_.start = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}
