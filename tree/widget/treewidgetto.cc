#include "treewidgetto.h"

#include "component/signalblocker.h"
#include "ui_treewidgetto.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TreeWidgetTO::TreeWidgetTO(Section section, TreeModel* model, const QDateTime& start, const QDateTime& end, QWidget* parent)
    : TreeWidget(parent)
    , ui(new Ui::TreeWidgetTO)
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

TreeWidgetTO::~TreeWidgetTO() { delete ui; }

QTreeView* TreeWidgetTO::View() const { return ui->treeViewTO; }

void TreeWidgetTO::on_start_dateChanged(const QDate& date)
{
    ui->pBtnFetch->setEnabled(date <= end_.date());
    start_.setDate(date);
}

void TreeWidgetTO::on_end_dateChanged(const QDate& date)
{
    end_ = QDateTime(date.addDays(1), kStartTime);
    ui->pBtnFetch->setEnabled(date >= start_.date());
}

void TreeWidgetTO::on_pBtnFetch_clicked()
{
    const auto message { JsonGen::TreeAcked(section_, start_.toUTC(), end_.toUTC()) };
    WebSocket::Instance()->SendMessage(kTreeAcked, message);
}
