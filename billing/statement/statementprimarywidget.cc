#include "statementprimarywidget.h"

#include <QTimer>

#include "component/constant.h"
#include "component/signalblocker.h"
#include "enum/nodeenum.h"
#include "enum/statementenum.h"
#include "ui_statementprimarywidget.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

StatementPrimaryWidget::StatementPrimaryWidget(
    StatementPrimaryModel* model, Section section, CUuid& widget_id, CUuid& partner_id, int unit, CDateTime& start, CDateTime& end, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StatementPrimaryWidget)
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
    model->setParent(this);

    IniUnitGroup();
    IniWidget();
    InitTimer();
    IniUnit(unit);
    IniConnect();

    QTimer::singleShot(0, this, &StatementPrimaryWidget::on_pBtnFetch_clicked);
}

StatementPrimaryWidget::~StatementPrimaryWidget() { delete ui; }

QTableView* StatementPrimaryWidget::View() const { return ui->tableView; }

void StatementPrimaryWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= end_.date() };
    start_.setDate(date);

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void StatementPrimaryWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= start_.date() };

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
    end_ = QDateTime(date.addDays(1), kStartTime);
}

void StatementPrimaryWidget::on_pBtnFetch_clicked()
{
    if (!ui->pBtnFetch->isEnabled()) {
        return;
    }

    ui->pBtnFetch->setEnabled(false);

    const auto message { JsonGen::StatementPrimaryAcked(section_, widget_id_, partner_id_, unit_, start_.toUTC(), end_.toUTC()) };
    WebSocket::Instance()->SendMessage(kStatementPrimaryAcked, message);

    cooldown_timer_->start(kTwoThousand);
}

void StatementPrimaryWidget::RUnitGroupClicked(int id)
{
    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(start_ <= end_);

    unit_ = id;
}

void StatementPrimaryWidget::IniUnitGroup()
{
    unit_group_ = new QButtonGroup(this);
    unit_group_->addButton(ui->rBtnIMM, 0);
    unit_group_->addButton(ui->rBtnMON, 1);
    unit_group_->addButton(ui->rBtnPEN, 2);
}

void StatementPrimaryWidget::IniConnect() { connect(unit_group_, &QButtonGroup::idClicked, this, &StatementPrimaryWidget::RUnitGroupClicked); }

void StatementPrimaryWidget::IniUnit(int unit)
{
    const UnitO kUnit { unit };

    switch (kUnit) {
    case UnitO::kImmediate:
        ui->rBtnIMM->setChecked(true);
        break;
    case UnitO::kMonthly:
        ui->rBtnMON->setChecked(true);
        break;
    case UnitO::kPending:
        ui->rBtnPEN->setChecked(true);
        break;
    default:
        break;
    }
}

void StatementPrimaryWidget::IniWidget()
{
    ui->start->setDisplayFormat(kDateFST);
    ui->end->setDisplayFormat(kDateFST);

    ui->pBtnFetch->setFocus();

    ui->start->setDateTime(start_);
    ui->end->setDateTime(end_.addSecs(-1));
}

void StatementPrimaryWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}

void StatementPrimaryWidget::on_tableView_doubleClicked(const QModelIndex& index)
{
    if (index.column() == std::to_underlying(StatementPrimaryEnum::kSettlement)) {
        emit SStatementSecondary(partner_id_, unit_, start_, end_);
    }
}
