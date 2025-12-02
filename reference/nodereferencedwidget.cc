#include "nodereferencedwidget.h"

#include <QTimer>

#include "component/constant.h"
#include "component/signalblocker.h"
#include "ui_nodereferencedwidget.h"

NodeReferencedWidget::NodeReferencedWidget(QAbstractItemModel* model, const QUuid& node_id, CDateTime& start, CDateTime& end, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::NodeReferencedWidget)
    , start_ { start }
    , end_ { end }
    , node_id_ { node_id }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);
    IniWidget(model);

    QTimer::singleShot(0, this, [this]() { emit SResetModel(node_id_, start_.toUTC(), end_.toUTC()); });
}

NodeReferencedWidget::~NodeReferencedWidget() { delete ui; }

QTableView* NodeReferencedWidget::View() const { return ui->tableView; }

QAbstractItemModel* NodeReferencedWidget::Model() const { return ui->tableView->model(); }

void NodeReferencedWidget::on_start_dateChanged(const QDate& date)
{
    ui->pBtnRefresh->setEnabled(date <= end_.date());
    start_.setDate(date);
}

void NodeReferencedWidget::on_end_dateChanged(const QDate& date)
{
    ui->pBtnRefresh->setEnabled(date >= start_.date());
    end_.setDate(date);
}

void NodeReferencedWidget::on_pBtnRefresh_clicked() { emit SResetModel(node_id_, start_.toUTC(), end_.toUTC()); }

void NodeReferencedWidget::IniWidget(QAbstractItemModel* model)
{
    ui->start->setDisplayFormat(kDateFST);
    ui->end->setDisplayFormat(kDateFST);
    ui->tableView->setModel(model);
    ui->start->setDateTime(start_);
    ui->end->setDateTime(end_);

    ui->pBtnRefresh->setFocus();
}
