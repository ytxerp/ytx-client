#include "nodereferencedwidget.h"

#include "component/constant.h"
#include "component/signalblocker.h"
#include "ui_nodereferencedwidget.h"

NodeReferencedWidget::NodeReferencedWidget(QAbstractItemModel* model, const QUuid& node_id, int node_unit, CDateTime& start, CDateTime& end, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::NodeReferencedWidget)
    , start_ { start }
    , end_ { end }
    , node_id_ { node_id }
    , node_unit_ { node_unit }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);
    IniWidget(model);
}

NodeReferencedWidget::~NodeReferencedWidget() { delete ui; }

QTableView* NodeReferencedWidget::View() const { return ui->tableView; }

QAbstractItemModel* NodeReferencedWidget::Model() const { return ui->tableView->model(); }

void NodeReferencedWidget::on_start_dateChanged(const QDate& date)
{
    ui->pBtnFetch->setEnabled(date <= end_.date());
    start_.setDate(date);
}

void NodeReferencedWidget::on_end_dateChanged(const QDate& date)
{
    ui->pBtnFetch->setEnabled(date >= start_.date());
    end_.setDate(date);
}

void NodeReferencedWidget::on_pBtnFetch_clicked() { }

void NodeReferencedWidget::IniWidget(QAbstractItemModel* model)
{
    ui->start->setDisplayFormat(kDateFST);
    ui->end->setDisplayFormat(kDateFST);
    ui->tableView->setModel(model);
    ui->start->setDateTime(start_);
    ui->end->setDateTime(end_);

    ui->pBtnFetch->setFocus();
}
