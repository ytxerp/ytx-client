#include "inventoryheatdialog.h"

#include <QTimer>

#include "component/constantint.h"
#include "component/constantstring.h"
#include "component/signalblocker.h"
#include "ui_inventoryheatdialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

InventoryHeatDialog::InventoryHeatDialog(inventory_heat::Model* model, const QUuid& widget_id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::InventoryHeatDialog)
    , range_ { DefaultRange() }
    , model_ { model }
    , widget_id_ { widget_id }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    InitTimer();
    InitDialog();

    ui->tableView->setModel(model);
    model->setParent(ui->tableView);

    QTimer::singleShot(0, this, &InventoryHeatDialog::on_pushButtonFetch_clicked);
}

InventoryHeatDialog::~InventoryHeatDialog() { delete ui; }

QTableView* InventoryHeatDialog::View() { return ui->tableView; }

void InventoryHeatDialog::InitDialog()
{
    ui->dateEditStart->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateEditEnd->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateEditStart->setDate(range_.start);
    ui->dateEditEnd->setDate(range_.end);
    ui->radioButtonSale->setChecked(true);
    ui->spinBoxMinOrderCount->setRange(1, INT_MAX);
    ui->spinBoxMinPartnerCount->setRange(1, INT_MAX);
    ui->spinBoxMinActiveMonths->setRange(1, INT_MAX);

    ui->pushButtonFetch->setFocus();
}

void InventoryHeatDialog::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pushButtonFetch->setEnabled(true); });
}

void InventoryHeatDialog::on_pushButtonFetch_clicked()
{
    if (!ui->pushButtonFetch->isEnabled()) {
        return;
    }

    if (!range_.IsValid()) {
        return;
    }

    ui->pushButtonFetch->setEnabled(false);

    qDebug() << Q_FUNC_INFO << "DateRange:" << range_.ToString();

    const auto query_range { range_.ToQueryRange() };

    qDebug() << Q_FUNC_INFO << "QueryRange:" << query_range.ToString();

    const Section section { ui->radioButtonSale->isChecked() ? Section::kSale : Section::kPurchase };
    const int moc { ui->spinBoxMinOrderCount->value() };
    const int mpc { ui->spinBoxMinPartnerCount->value() };
    const int mam { ui->spinBoxMinActiveMonths->value() };

    const auto message { JsonGen::InventoryHeadAck(section, widget_id_, query_range, moc, mpc, mam) };
    WebSocket::Instance()->SendMessage(WsKey::kInventoryHeatAck, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void InventoryHeatDialog::on_dateEditStart_dateChanged(const QDate& date)
{
    const bool valid { date <= range_.end };
    range_.start = date;

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}

void InventoryHeatDialog::on_dateEditEnd_dateChanged(const QDate& date)
{
    const bool valid { date >= range_.start };
    range_.end = date;

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}
