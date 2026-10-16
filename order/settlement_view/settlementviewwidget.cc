#include "settlementviewwidget.h"

#include "component/constantstring.h"
#include "component/signalblocker.h"
#include "ui_settlementviewwidget.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

SettlementViewWidget::SettlementViewWidget(
    const QHash<QUuid, QString>& partner_leaf_path, CUuid& widget_id, const int& amount_decimal, Section section, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettlementViewWidget)
    , range_(DefaultRange())
    , widget_id_ { widget_id }
    , section_ { section }
    , amount_dlg_ { new DoubleNoneZeroR(amount_decimal, string_const::kEightDigits, this) }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniWidget();
    InitTimer();
    InitModel(partner_leaf_path);

    QTimer::singleShot(0, this, &SettlementViewWidget::on_pBtnFetch_clicked);
}

SettlementViewWidget::~SettlementViewWidget() { delete ui; }

QTableView* SettlementViewWidget::DataView() const { return ui->tableView; }

QTableView* SettlementViewWidget::SummaryView() const { return ui->tableViewSummary; }

void SettlementViewWidget::IniWidget()
{
    ui->start->setDisplayFormat(datetime_format::kDashedDate);
    ui->end->setDisplayFormat(datetime_format::kDashedDate);

    ui->pBtnFetch->setFocus();

    ui->start->setDate(range_.start);
    ui->end->setDate(range_.end);
}

void SettlementViewWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}

void SettlementViewWidget::RebuildColumns()
{
    if (!data_model_->RebuildHeader(range_))
        return;

    const int column_count { data_model_->columnCount() };

    summary_model_->RebuildHeader(column_count);

    for (int column = 1; column != column_count; ++column) {
        ui->tableView->setItemDelegateForColumn(column, amount_dlg_);
        ui->tableViewSummary->setItemDelegateForColumn(column, amount_dlg_);
    }

    auto* header { ui->tableView->horizontalHeader() };

    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(column_count - 4, QHeaderView::Stretch);
}

void SettlementViewWidget::InitModel(const QHash<QUuid, QString>& partner_leaf_path)
{
    data_model_ = new settlement_view::Model(partner_leaf_path, this);
    summary_model_ = new DynamicSummaryModel(data_model_->columnCount(), this);

    ui->tableView->setModel(data_model_);
    ui->tableViewSummary->setModel(summary_model_);

    RebuildColumns();

    connect(data_model_, &settlement_view::Model::SSummaryChanged, summary_model_, &DynamicSummaryModel::RSummaryChanged);
}

void SettlementViewWidget::on_pBtnFetch_clicked()
{
    if (!ui->pBtnFetch->isEnabled()) {
        return;
    }

    if (!range_.IsValid()) {
        return;
    }

    ui->pBtnFetch->setEnabled(false);

    RebuildColumns();

    qDebug() << Q_FUNC_INFO << "DateRange:" << range_.ToString();

    const auto query_range { range_.ToQueryRange() };

    qDebug() << Q_FUNC_INFO << "QueryRange:" << query_range.ToString();

    const auto message { JsonGen::SettlementView(section_, widget_id_, query_range) };
    WebSocket::Instance()->SendMessage(WsKey::kSettlementView, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void SettlementViewWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= range_.end };
    range_.start = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void SettlementViewWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= range_.start };
    range_.end = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}
