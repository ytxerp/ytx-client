#include "statementsecondarywidget.h"

#include <QTimer>

#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "enum/nodeenum.h"
#include "statementenum.h"
#include "ui_statementsecondarywidget.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

StatementSecondaryWidget::StatementSecondaryWidget(
    statement::SecondaryModel* model, CUuid& widget_id, CUuid& partner_id, const utils::DateRange& range, Section section, int unit, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StatementSecondaryWidget)
    , unit_ { unit }
    , range_ { range }
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

    QTimer::singleShot(0, this, &StatementSecondaryWidget::on_pBtnFetch_clicked);
}

StatementSecondaryWidget::~StatementSecondaryWidget() { delete ui; }

QTableView* StatementSecondaryWidget::View() const { return ui->tableView; }

void StatementSecondaryWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= range_.end };
    range_.start = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void StatementSecondaryWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= range_.start };
    range_.end = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void StatementSecondaryWidget::on_pBtnFetch_clicked()
{
    if (!ui->pBtnFetch->isEnabled()) {
        return;
    }

    if (!range_.IsValid()) {
        return;
    }

    ui->pBtnFetch->setEnabled(false);

    qDebug() << Q_FUNC_INFO << "DateRange:" << range_.ToString();

    const auto query_range { range_.ToQueryRange() };

    qDebug() << Q_FUNC_INFO << "QueryRange:" << query_range.ToString();

    const auto message { JsonGen::StatementSecondary(section_, widget_id_, partner_id_, unit_, query_range) };
    WebSocket::Instance()->SendMessage(WsKey::kStatementSecondary, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void StatementSecondaryWidget::RUnitGroupClicked(int id)
{
    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(range_.IsValid());

    unit_ = id;
}

void StatementSecondaryWidget::IniUnitGroup()
{
    unit_group_ = new QButtonGroup(this);
    unit_group_->addButton(ui->rBtnIS, std::to_underlying(NodeUnit::OImmediate));
    unit_group_->addButton(ui->rBtnMS, std::to_underlying(NodeUnit::OMonthly));
    unit_group_->addButton(ui->rBtnPEND, std::to_underlying(NodeUnit::OPending));
}

void StatementSecondaryWidget::IniConnect() { connect(unit_group_, &QButtonGroup::idClicked, this, &StatementSecondaryWidget::RUnitGroupClicked); }

void StatementSecondaryWidget::IniUnit(int unit)
{
    const NodeUnit kUnit { unit };

    switch (kUnit) {
    case NodeUnit::OImmediate:
        ui->rBtnIS->setChecked(true);
        break;
    case NodeUnit::OMonthly:
        ui->rBtnMS->setChecked(true);
        break;
    case NodeUnit::OPending:
        ui->rBtnPEND->setChecked(true);
        break;
    default:
        break;
    }
}

void StatementSecondaryWidget::IniWidget()
{
    ui->start->setDisplayFormat(datetime_format::kDashedDate);
    ui->end->setDisplayFormat(datetime_format::kDashedDate);

    ui->pBtnFetch->setFocus();

    ui->start->setDate(range_.start);
    ui->end->setDate(range_.end);

    utils::SetRadioButton(ui->rBtnIS, QKeySequence(Qt::CTRL | Qt::Key_1));
    utils::SetRadioButton(ui->rBtnMS, QKeySequence(Qt::CTRL | Qt::Key_2));
    utils::SetRadioButton(ui->rBtnPEND, QKeySequence(Qt::CTRL | Qt::Key_3));
}

void StatementSecondaryWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}

void StatementSecondaryWidget::on_tableView_doubleClicked(const QModelIndex& index)
{
    if (index.column() == std::to_underlying(statement::SecondaryField::kIssuedTime)) {
        emit SShowTertiaryStatement(partner_id_, range_, unit_);
    }
}
