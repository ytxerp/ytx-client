#include "orderreferencewidget.h"

#include "component/constant.h"
#include "component/signalblocker.h"
#include "ui_orderreferencewidget.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

OrderReferenceWidget::OrderReferenceWidget(OrderReferenceModel* model, Section section, CUuid& widget_id, CUuid& node_id, int node_unit, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::OrderReferenceWidget)
    , start_ { QDateTime(QDate(QDate::currentDate().year() - 1, 1, 1), kStartTime) }
    , end_ { QDateTime(QDate(QDate::currentDate().year() + 1, 1, 1), kStartTime) }
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

    IniWidget();
    InitTimer();

    QTimer::singleShot(0, this, &OrderReferenceWidget::on_pBtnFetch_clicked);
}

OrderReferenceWidget::~OrderReferenceWidget() { delete ui; }

QTableView* OrderReferenceWidget::View() const { return ui->tableView; }

void OrderReferenceWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= end_.date() };
    start_.setDate(date);

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void OrderReferenceWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= start_.date() };

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
    end_ = QDateTime(date.addDays(1), kStartTime);
}

void OrderReferenceWidget::on_pBtnFetch_clicked()
{
    if (!ui->pBtnFetch->isEnabled()) {
        return;
    }

    ui->pBtnFetch->setEnabled(false);

    const auto message { JsonGen::OrderReferenceAck(section_, widget_id_, node_id_, node_unit_, start_.toUTC(), end_.toUTC()) };
    WebSocket::Instance()->SendMessage(kOrderReferenceAck, message);

    cooldown_timer_->start(kTwoThousand);
}

void OrderReferenceWidget::IniWidget()
{
    ui->start->setDisplayFormat(kDateFST);
    ui->end->setDisplayFormat(kDateFST);
    ui->start->setDateTime(start_);
    ui->end->setDateTime(end_.addSecs(-1));

    ui->pBtnFetch->setFocus();
}

void OrderReferenceWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}
