#include "partnerheatdialog.h"

#include <QTimer>

#include "component/constantint.h"
#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "enum/section.h"
#include "ui_partnerheatdialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

PartnerHeatDialog::PartnerHeatDialog(partner_heat::Model* model, const QUuid& widget_id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PartnerHeatDialog)
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

    QTimer::singleShot(0, this, &PartnerHeatDialog::on_pushButtonFetch_clicked);
}

PartnerHeatDialog::~PartnerHeatDialog() { delete ui; }

QTableView* PartnerHeatDialog::View() { return ui->tableView; }

void PartnerHeatDialog::InitDialog()
{
    ui->dateEditStart->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateEditEnd->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateEditStart->setDate(range_.start);
    ui->dateEditEnd->setDate(range_.end);
    ui->radioButtonSale->setChecked(true);
    ui->spinBoxMinOrderCount->setRange(1, INT_MAX);
    ui->spinBoxMinInventoryDiversity->setRange(1, INT_MAX);
    ui->spinBoxMinActiveMonths->setRange(1, INT_MAX);

    ui->pushButtonFetch->setFocus();
}

void PartnerHeatDialog::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pushButtonFetch->setEnabled(true); });
}

void PartnerHeatDialog::on_pushButtonFetch_clicked()
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
    const int mid { ui->spinBoxMinInventoryDiversity->value() };
    const int mam { ui->spinBoxMinActiveMonths->value() };

    const auto message { JsonGen::PartnerHeadAck(section, widget_id_, query_range, moc, mid, mam) };
    WebSocket::Instance()->SendMessage(WsKey::kPartnerHeatAck, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void PartnerHeatDialog::on_dateEditStart_dateChanged(const QDate& date)
{
    const bool valid { date <= range_.end };
    range_.start = date;

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}

void PartnerHeatDialog::on_dateEditEnd_dateChanged(const QDate& date)
{
    const bool valid { date >= range_.start };
    range_.end = date;

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}
