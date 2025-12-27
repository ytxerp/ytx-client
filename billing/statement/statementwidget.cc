#include "statementwidget.h"

#include "component/constant.h"
#include "component/signalblocker.h"
#include "enum/nodeenum.h"
#include "enum/statementenum.h"
#include "ui_statementwidget.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

StatementWidget::StatementWidget(StatementModel* model, Section section, CUuid& widget_id, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StatementWidget)
    , unit_ { std::to_underlying(UnitO::kMonthly) }
    , model_ { model }
    , section_ { section }
    , widget_id_ { widget_id }

{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model);
    model->setParent(ui->tableView);

    IniUnitGroup();
    IniWidget();
    InitTimer();
    IniUnit(unit_);
    IniConnect();

    QTimer::singleShot(0, this, &StatementWidget::on_pBtnFetch_clicked);
}

StatementWidget::~StatementWidget() { delete ui; }

QTableView* StatementWidget::View() const { return ui->tableView; }

void StatementWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= end_.date() };
    start_.setDate(date);

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void StatementWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= start_.date() };

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
    end_ = QDateTime(date.addDays(1), kStartTime);
}

void StatementWidget::on_pBtnFetch_clicked()
{
    if (!ui->pBtnFetch->isEnabled()) {
        return;
    }

    ui->pBtnFetch->setEnabled(false);

    const auto message { JsonGen::StatementAcked(section_, widget_id_, unit_, start_.toUTC(), end_.toUTC()) };
    WebSocket::Instance()->SendMessage(kStatementAcked, message);

    cooldown_timer_->start(kTwoThousand);
}

void StatementWidget::RUnitGroupClicked(int id)
{
    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(start_ <= end_);

    unit_ = id;
}

void StatementWidget::IniUnitGroup()
{
    unit_group_ = new QButtonGroup(this);
    unit_group_->addButton(ui->rBtnIMM, 0);
    unit_group_->addButton(ui->rBtnMON, 1);
    unit_group_->addButton(ui->rBtnPEN, 2);
}

void StatementWidget::IniConnect() { connect(unit_group_, &QButtonGroup::idClicked, this, &StatementWidget::RUnitGroupClicked); }

void StatementWidget::IniUnit(int unit)
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

void StatementWidget::IniWidget()
{
    start_ = QDateTime(QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1), kStartTime);

    const QDate first_of_next_month { QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1).addMonths(1) };
    end_ = QDateTime(first_of_next_month, kStartTime);

    ui->start->setDisplayFormat(kDateFST);
    ui->end->setDisplayFormat(kDateFST);

    ui->pBtnFetch->setFocus();

    ui->start->setDateTime(start_);
    ui->end->setDateTime(end_.addSecs(-1));
}

void StatementWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}

void StatementWidget::on_tableView_doubleClicked(const QModelIndex& index)
{
    if (index.column() == std::to_underlying(StatementEnum::kCBalance)) {
        const auto partner { index.siblingAtColumn(std::to_underlying(StatementEnum::kPartner)).data().toUuid() };
        emit SStatementNode(partner, unit_, start_, end_);
    }
}
