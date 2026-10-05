#include "settlementprimarywidget.h"

#include <QJsonArray>
#include <QTimer>

#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "ui_settlementprimarywidget.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

SettlementPrimaryWidget::SettlementPrimaryWidget(settlement::PrimaryModel* model, CUuid& widget_id, Section section, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettlementPrimaryWidget)
    , model_ { model }
    , range_ { DefaultRange() }
    , section_ { section }
    , widget_id_ { widget_id }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model);
    model->setParent(ui->tableView);

    InitWidget();
    InitTimer();

    QTimer::singleShot(0, this, &SettlementPrimaryWidget::on_pBtnFetch_clicked);
}

SettlementPrimaryWidget::~SettlementPrimaryWidget() { delete ui; }

QTableView* SettlementPrimaryWidget::View() const { return ui->tableView; }

void SettlementPrimaryWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= range_.end };
    range_.start = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void SettlementPrimaryWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= range_.start };
    range_.end = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void SettlementPrimaryWidget::on_pBtnFetch_clicked()
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

    const auto message { JsonGen::SettlementPrimary(section_, widget_id_, query_range) };
    WebSocket::Instance()->SendMessage(WsKey::kSettlementPrimary, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void SettlementPrimaryWidget::InitWidget()
{
    ui->start->setDisplayFormat(datetime_format::kDashedDate);
    ui->end->setDisplayFormat(datetime_format::kDashedDate);

    ui->pBtnFetch->setFocus();

    ui->start->setDate(range_.start);
    ui->end->setDate(range_.end);
}

void SettlementPrimaryWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}

void SettlementPrimaryWidget::on_pushButtonDelete_clicked()
{
    auto* view { ui->tableView };
    Q_ASSERT(view != nullptr);

    if (!utils::HasSelection(view))
        return;

    const QModelIndex index { view->currentIndex() };
    if (!index.isValid())
        return;

    auto* settlement { static_cast<settlement::PrimaryRow*>(index.internalPointer()) };

    if (settlement->status == SettlementStatus::kReleased) {
        utils::ShowMessage(QMessageBox::Information, tr("Operation Rejected"),
            tr("The released settlement cannot be deleted.\n"
               "Please recall it first and try again."),
            time_const::kAutoCloseMs);
        return;
    }

    model_->removeRows(index.row(), 1);
}

void SettlementPrimaryWidget::on_pushButtonInsert_clicked() { emit SCreateSettlementSecondaryWidget(model_); }
