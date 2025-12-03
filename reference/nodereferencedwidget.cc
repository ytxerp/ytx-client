#include "nodereferencedwidget.h"

#include "component/constant.h"
#include "component/signalblocker.h"
#include "ui_nodereferencedwidget.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

NodeReferencedWidget::NodeReferencedWidget(
    QAbstractItemModel* model, const Section section, const QUuid& node_id, int node_unit, CDateTime& start, CDateTime& end, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::NodeReferencedWidget)
    , start_ { start }
    , end_ { end }
    , node_id_ { node_id }
    , node_unit_ { node_unit }
    , section_ { section }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniWidget(model);
    InitTimer();

    QTimer::singleShot(0, this, &NodeReferencedWidget::on_pBtnFetch_clicked);
}

NodeReferencedWidget::~NodeReferencedWidget() { delete ui; }

QTableView* NodeReferencedWidget::View() const { return ui->tableView; }

QAbstractItemModel* NodeReferencedWidget::Model() const { return ui->tableView->model(); }

void NodeReferencedWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= end_.date() };
    start_.setDate(date);

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void NodeReferencedWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= start_.date() };

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
    end_ = QDateTime(date.addDays(1), kStartTime);
}

void NodeReferencedWidget::on_pBtnFetch_clicked()
{
    if (!ui->pBtnFetch->isEnabled()) {
        return;
    }

    ui->pBtnFetch->setEnabled(false);

    const auto message { JsonGen::NodeReferenced(section_, node_id_, node_unit_, start_.toUTC(), end_.toUTC()) };
    WebSocket::Instance()->SendMessage(kNodeReferenced, message);

    cooldown_timer_->start(kTwoThousand);
}

void NodeReferencedWidget::IniWidget(QAbstractItemModel* model)
{
    ui->start->setDisplayFormat(kDateFST);
    ui->end->setDisplayFormat(kDateFST);
    ui->tableView->setModel(model);
    ui->start->setDateTime(start_);
    ui->end->setDateTime(end_.addSecs(-1));

    ui->pBtnFetch->setFocus();
}

void NodeReferencedWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}
