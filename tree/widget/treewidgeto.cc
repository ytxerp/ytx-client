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

    ui->start->setDisplayFormat(kDateFST);
    ui->end->setDisplayFormat(kDateFST);

    ui->start->setDateTime(start_);
    ui->end->setDateTime(end.addSecs(-1));

    ui->treeViewTO->setModel(model);
}

TreeWidgetO::~TreeWidgetO() { delete ui; }

QTreeView* TreeWidgetO::View() const { return ui->treeViewTO; }

void TreeWidgetO::on_start_dateChanged(const QDate& date)
{
    ui->pBtnFetch->setEnabled(date <= end_.date());
    start_.setDate(date);
}

void TreeWidgetO::on_end_dateChanged(const QDate& date)
{
    end_ = QDateTime(date.addDays(1), kStartTime);
    ui->pBtnFetch->setEnabled(date >= start_.date());
}

void TreeWidgetO::on_pBtnFetch_clicked()
{
    if (start_ == last_start_ && end_ == last_end_)
        return;

    last_start_ = start_;
    last_end_ = end_;

    const auto message { JsonGen::TreeAcked(section_, start_.toUTC(), end_.toUTC()) };
    WebSocket::Instance()->SendMessage(kTreeAcked, message);
}
