#include "statementnodewidget.h"

#include <QTimer>

#include "component/constant.h"
#include "component/signalblocker.h"
#include "enum/nodeenum.h"
#include "enum/statementenum.h"
#include "ui_statementnodewidget.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

StatementNodeWidget::StatementNodeWidget(
    StatementNodeModel* model, CUuid& widget_id, CUuid& partner_id, CDateTime& start, CDateTime& end, Section section, int unit, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StatementNodeWidget)
    , unit_ { unit }
    , start_ { start }
    , end_ { end }
    , model_ { model }
    , section_ { section }
    , widget_id_ { widget_id }
    , partner_id_ { partner_id }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model);
    model->setParent(ui->tableView);

    IniUnitGroup();
    IniWidget();
    InitTimer();
    IniUnit(unit);
    IniConnect();

    QTimer::singleShot(0, this, &StatementNodeWidget::on_pBtnFetch_clicked);
}

StatementNodeWidget::~StatementNodeWidget() { delete ui; }

QTableView* StatementNodeWidget::View() const { return ui->tableView; }

void StatementNodeWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= end_.date() };
    start_.setDate(date);

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void StatementNodeWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= start_.date() };

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
    end_ = QDateTime(date.addDays(1), kStartTime);
}

void StatementNodeWidget::on_pBtnFetch_clicked()
{
    if (!ui->pBtnFetch->isEnabled()) {
        return;
    }

    ui->pBtnFetch->setEnabled(false);

    const auto message { JsonGen::StatementNodeAcked(section_, widget_id_, partner_id_, unit_, start_.toUTC(), end_.toUTC()) };
    WebSocket::Instance()->SendMessage(kStatementNodeAcked, message);

    cooldown_timer_->start(kTwoThousand);
}

void StatementNodeWidget::RUnitGroupClicked(int id)
{
    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(start_ <= end_);

    unit_ = id;
}

void StatementNodeWidget::IniUnitGroup()
{
    unit_group_ = new QButtonGroup(this);
    unit_group_->addButton(ui->rBtnIMM, std::to_underlying(NodeUnit::OImmediate));
    unit_group_->addButton(ui->rBtnMON, std::to_underlying(NodeUnit::OMonthly));
    unit_group_->addButton(ui->rBtnPEN, std::to_underlying(NodeUnit::OPending));
}

void StatementNodeWidget::IniConnect() { connect(unit_group_, &QButtonGroup::idClicked, this, &StatementNodeWidget::RUnitGroupClicked); }

void StatementNodeWidget::IniUnit(int unit)
{
    const NodeUnit kUnit { unit };

    switch (kUnit) {
    case NodeUnit::OImmediate:
        ui->rBtnIMM->setChecked(true);
        break;
    case NodeUnit::OMonthly:
        ui->rBtnMON->setChecked(true);
        break;
    case NodeUnit::OPending:
        ui->rBtnPEN->setChecked(true);
        break;
    default:
        break;
    }
}

void StatementNodeWidget::IniWidget()
{
    ui->start->setDisplayFormat(kDateFST);
    ui->end->setDisplayFormat(kDateFST);

    ui->pBtnFetch->setFocus();

    ui->start->setDateTime(start_);
    ui->end->setDateTime(end_.addSecs(-1));
}

void StatementNodeWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}

void StatementNodeWidget::on_tableView_doubleClicked(const QModelIndex& index)
{
    if (index.column() == std::to_underlying(StatementNodeEnum::kSettlement)) {
        emit SStatementEntry(partner_id_, start_, end_, unit_);
    }
}
