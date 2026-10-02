#include "auditdialog.h"

#include "audit/auditenum.h"
#include "component/constantint.h"
#include "component/constantstring.h"
#include "component/signalblocker.h"
#include "global/logininfo.h"
#include "ui_auditdialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

AuditDialog::AuditDialog(const audit::Info& info, const QStringList& header, CUuid& widget_id, Section section, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AuditDialog)
    , section_ { section }
    , info_ { info }
    , range_ { DefaultRange() }
    , widget_id_ { widget_id }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    InitTimer();
    InitDialog();
    InitModel(header);

    setWindowTitle(tr("Audit") + QStringLiteral(" - ") + info.section_map.value(std::to_underlying(section)));

    QTimer::singleShot(0, this, &AuditDialog::on_pBtnFetch_clicked);
}

AuditDialog::~AuditDialog() { delete ui; }

QTableView* AuditDialog::DataView() { return ui->tableViewData; }

QTableView* AuditDialog::FilterView() const { return ui->tableViewFilter; }

void AuditDialog::on_pBtnFetch_clicked()
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

    const auto message { JsonGen::AuditLogAck(widget_id_, LoginInfo::Instance().Workspace(), query_range, section_) };
    WebSocket::Instance()->SendMessage(WsKey::kAuditLogAck, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void AuditDialog::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}

void AuditDialog::InitModel(const QStringList& header)
{
    data_model_ = new audit::Model(info_, header, this);
    filter_model_ = new TableFilterModel(header, std::to_underlying(audit::RowField::kPlaceholder), this);
    filter_proxy_ = new TableFilterProxyModel(this);

    filter_proxy_->setSourceModel(data_model_);

    ui->tableViewData->setModel(filter_proxy_);
    ui->tableViewFilter->setModel(filter_model_);

    connect(filter_model_, &TableFilterModel::SFilterChanged, filter_proxy_, &TableFilterProxyModel::RFilterChanged);
    connect(filter_model_, &TableFilterModel::SFiltersCleared, filter_proxy_, &TableFilterProxyModel::RFiltersCleared);
    connect(filter_model_, &TableFilterModel::SSortRequested, filter_proxy_, &TableFilterProxyModel::RSortRequested);

    ui->tableViewFilter->setSortingEnabled(true);
}

void AuditDialog::on_dateEditStart_dateChanged(const QDate& date)
{
    const bool valid { date <= range_.end };
    range_.start = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void AuditDialog::on_dateEditEnd_dateChanged(const QDate& date)
{
    const bool valid { date >= range_.start };
    range_.end = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void AuditDialog::InitDialog()
{
    ui->dateEditStart->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateEditEnd->setDisplayFormat(datetime_format::kDashedDate);

    ui->pBtnFetch->setFocus();

    ui->dateEditStart->setDate(range_.start);
    ui->dateEditEnd->setDate(range_.end);
}

void AuditDialog::on_pushButton_clicked()
{
    filter_model_->ClearFilters();

    auto* view { ui->tableViewFilter };
    view->clearSelection();
    view->setCurrentIndex({});
}
