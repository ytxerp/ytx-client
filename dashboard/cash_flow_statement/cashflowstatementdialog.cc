#include "cashflowstatementdialog.h"

#include "component/constantint.h"
#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "ui_cashflowstatementdialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

CashFlowStatementDialog::CashFlowStatementDialog(cash_flow::Model* model, cash_flow::CarrierModel* carrier, cash_flow::SpecialModel* special,
    cash_flow::WrongModel* wrong, const QUuid& widget_id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::CashFlowStatementDialog)
    , range_ { DefaultRange() }
    , widget_id_ { widget_id }
    , model_ { model }
    , carrier_ { carrier }
    , special_ { special }
    , wrong_ { wrong }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    InitTimer();
    InitDialog();

    ui->treeView->setModel(model);
    model->setParent(ui->treeView);

    ui->treeViewCarrier->setModel(carrier);
    carrier->setParent(ui->treeViewCarrier);

    ui->treeViewSpecial->setModel(special);
    special->setParent(ui->treeViewSpecial);

    ui->tableView->setModel(wrong);
    wrong->setParent(ui->tableView);

    QTimer::singleShot(0, this, &::CashFlowStatementDialog::on_pushButtonFetch_clicked);
}

CashFlowStatementDialog::~CashFlowStatementDialog() { delete ui; }

QTreeView* CashFlowStatementDialog::View() { return ui->treeView; }

QTreeView* CashFlowStatementDialog::CarrierView() { return ui->treeViewCarrier; }

QTreeView* CashFlowStatementDialog::SpecialView() { return ui->treeViewSpecial; }

QTableView* CashFlowStatementDialog::WrongView() { return ui->tableView; }

void CashFlowStatementDialog::on_dateEditStart_dateChanged(const QDate& date)
{
    const bool valid { date <= range_.end };
    range_.start = date;

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}

void CashFlowStatementDialog::on_dateEditEnd_dateChanged(const QDate& date)
{
    const bool valid { date >= range_.start };
    range_.end = date;

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}

void CashFlowStatementDialog::on_pushButtonFetch_clicked()
{
    if (!ui->pushButtonFetch->isEnabled()) {
        return;
    }

    if (!range_.IsValid()) {
        return;
    }

    qDebug() << Q_FUNC_INFO << "DateRange:" << range_.ToString();

    const auto query_range { range_.ToQueryRange() };

    qDebug() << Q_FUNC_INFO << "QueryRange:" << query_range.ToString();

    const auto message { JsonGen::CashFlowStatementAck(widget_id_, query_range) };

    WebSocket::Instance()->SendMessage(WsKey::kCashFlowStatementAck, message);

    ui->pushButtonFetch->setEnabled(false);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void CashFlowStatementDialog::InitDialog()
{
    ui->dateEditStart->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateEditEnd->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateEditStart->setDate(range_.start);
    ui->dateEditEnd->setDate(range_.end);

    ui->pushButtonFetch->setFocus();

    ui->splitter_root->setSizes({ 800, 200 });
    ui->splitter_h->setSizes({ 700, 300 });
    ui->splitter_v->setSizes({ 600, 400 });
}

void CashFlowStatementDialog::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pushButtonFetch->setEnabled(true); });
}
