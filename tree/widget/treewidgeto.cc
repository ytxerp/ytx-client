#include "treewidgeto.h"

#include "component/signalblocker.h"
#include "ui_treewidgeto.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TreeWidgetO::TreeWidgetO(Section section, TreeModel* model, const QDateTime& start, const QDateTime& end, QWidget* parent)
    : TreeWidget(parent)
    , ui(new Ui::TreeWidgetO)
    , section_ { section }
    , model_ { model }
    , start_ { start }
    , end_ { end }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);
    InitTimer();

    ui->start->setDisplayFormat(kDateFST);
    ui->end->setDisplayFormat(kDateFST);

    ui->start->setDateTime(start_);
    ui->end->setDateTime(end.addSecs(-1));

    ui->treeViewO->setModel(model);
}

TreeWidgetO::~TreeWidgetO() { delete ui; }

QTreeView* TreeWidgetO::View() const { return ui->treeViewO; }

void TreeWidgetO::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= end_.date() };
    start_.setDate(date);

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void TreeWidgetO::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= start_.date() };

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);

    end_ = QDateTime(date.addDays(1), kStartTime);
}

void TreeWidgetO::on_pBtnFetch_clicked()
{
    if (!ui->pBtnFetch->isEnabled()) {
        return;
    }

    ui->pBtnFetch->setEnabled(false);

    const auto message { JsonGen::TreeAcked(section_, start_.toUTC(), end_.toUTC()) };
    WebSocket::Instance()->SendMessage(kTreeAcked, message);

    cooldown_timer_->start(kTwoThousand);
}

void TreeWidgetO::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}
