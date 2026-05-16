#include "inventoryheatdialog.h"

#include <QTimer>

#include "component/constant.h"
#include "component/constantint.h"
#include "component/signalblocker.h"
#include "ui_inventoryheatdialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

InventoryHeatDialog::InventoryHeatDialog(InventoryHeatModel* model, const QUuid& widget_id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::InventoryHeatDialog)
    , start_ { QDateTime(QDate::currentDate().addYears(-2), kStartTime) }
    , end_ { QDateTime(QDate::currentDate().addDays(1), kStartTime) }
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
    ui->dateTimeEditStart->setDisplayFormat(kDateFST);
    ui->dateTimeEditEnd->setDisplayFormat(kDateFST);
    ui->dateTimeEditStart->setDateTime(start_);
    ui->dateTimeEditEnd->setDateTime(end_.addDays(-1));
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

    ui->pushButtonFetch->setEnabled(false);

    const Section section { ui->radioButtonSale->isChecked() ? Section::kSale : Section::kPurchase };
    const int moc { ui->spinBoxMinOrderCount->value() };
    const int mpc { ui->spinBoxMinPartnerCount->value() };
    const int mam { ui->spinBoxMinActiveMonths->value() };

    const auto message { JsonGen::InventoryHeadAck(section, widget_id_, start_.toUTC(), end_.toUTC(), moc, mpc, mam) };
    WebSocket::Instance()->SendMessage(WsKey::kInventoryHeatAck, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void InventoryHeatDialog::on_dateTimeEditStart_dateChanged(const QDate& date)
{
    const bool valid { date < end_.date() };
    start_ = QDateTime(date, kStartTime);

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}

void InventoryHeatDialog::on_dateTimeEditEnd_dateChanged(const QDate& date)
{
    const bool valid { date >= start_.date() };

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);

    end_ = QDateTime(date.addDays(1), kStartTime);
}
