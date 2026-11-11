#include "treewidgetr.h"

#include <QTimer>

#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "ui_treewidgetr.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TreeWidgetR::TreeWidgetR(Section section, TreeModel* model, QWidget* parent)
    : TreeWidget(parent)
    , ui(new Ui::TreeWidgetR)
    , section_ { section }
    , model_ { model }
    , range_ { DefaultRange() }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    InitWidget();
    InitTimer();

    ui->treeViewR->setModel(model);
    model->setParent(ui->treeViewR);
}

TreeWidgetR::~TreeWidgetR() { delete ui; }

QTreeView* TreeWidgetR::View() const { return ui->treeViewR; }

void TreeWidgetR::Reset()
{
    range_ = DefaultRange();

    ui->start->setDate(range_.start);
    ui->end->setDate(range_.end);
}

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

    const auto message { JsonGen::Tree(section_, query_range) };
    WebSocket::Instance()->SendMessage(WsKey::kOrderTreeAck, message);

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
