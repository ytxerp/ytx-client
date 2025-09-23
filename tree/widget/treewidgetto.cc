#include "treewidgetto.h"

#include "component/signalblocker.h"
#include "global/websocket.h"
#include "ui_treewidgetto.h"
#include "utils/jsongen.h"

TreeWidgetTO::TreeWidgetTO(CString& section, TreeModel* model, const QDateTime& start, const QDateTime& end, QWidget* parent)
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
    ui->end->setDateTime(end_);

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
    ui->pBtnFetch->setEnabled(date >= start_.date());
    end_.setDate(date);
}

void TreeWidgetTO::on_pBtnFetch_clicked()
{
    const auto message { JsonGen::NodeDataAcked(section_, start_.toUTC(), end_.toUTC()) };
    WebSocket::Instance()->SendMessage(kTreeAcked, message);
}
