#include "statementprimarywidget.h"

#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "enum/nodeenum.h"
#include "statementenum.h"
#include "ui_statementprimarywidget.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

StatementPrimaryWidget::StatementPrimaryWidget(const QStringList& header, CUuid& widget_id, Section section, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StatementPrimaryWidget)
    , unit_ { std::to_underlying(NodeUnit::OMonthly) }
    , range_ { DefaultRange() }
    , section_ { section }
    , widget_id_ { widget_id }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniUnitGroup();
    IniWidget();
    InitTimer();
    InitModel(header);
    IniUnit(unit_);
    IniConnect();

    QTimer::singleShot(0, this, &StatementPrimaryWidget::on_pBtnFetch_clicked);
}

StatementPrimaryWidget::~StatementPrimaryWidget() { delete ui; }

QTableView* StatementPrimaryWidget::DataView() const { return ui->tableViewData; }

QTableView* StatementPrimaryWidget::FilterView() const { return ui->tableViewFilter; }

void StatementPrimaryWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= range_.end };
    range_.start = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void StatementPrimaryWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= range_.start };
    range_.end = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void StatementPrimaryWidget::on_pBtnFetch_clicked()
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

    const auto message { JsonGen::StatementPrimary(section_, widget_id_, unit_, query_range) };
    WebSocket::Instance()->SendMessage(WsKey::kStatementPrimary, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void StatementPrimaryWidget::RUnitGroupClicked(int id)
{
    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(range_.IsValid());

    unit_ = id;
}

void StatementPrimaryWidget::IniUnitGroup()
{
    unit_group_ = new QButtonGroup(this);
    unit_group_->addButton(ui->rBtnIS, std::to_underlying(NodeUnit::OImmediate));
    unit_group_->addButton(ui->rBtnMS, std::to_underlying(NodeUnit::OMonthly));
    unit_group_->addButton(ui->rBtnPEND, std::to_underlying(NodeUnit::OPending));
}

void StatementPrimaryWidget::IniConnect() { connect(unit_group_, &QButtonGroup::idClicked, this, &StatementPrimaryWidget::RUnitGroupClicked); }

void StatementPrimaryWidget::IniUnit(int unit)
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

void StatementPrimaryWidget::IniWidget()
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

void StatementPrimaryWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}

void StatementPrimaryWidget::InitModel(const QStringList& header)
{
    data_model_ = new statement::PrimaryModel(header, this);
    filter_model_ = new TableFilterModel(header, std::to_underlying(statement::PrimaryField::kPlaceholder), this);
    filter_proxy_ = new TableFilterProxyModel(this);

    filter_proxy_->setSourceModel(data_model_);

    ui->tableViewData->setModel(filter_proxy_);
    ui->tableViewFilter->setModel(filter_model_);

    connect(filter_model_, &TableFilterModel::SFilterChanged, filter_proxy_, &TableFilterProxyModel::RFilterChanged);
    connect(filter_model_, &TableFilterModel::SFiltersCleared, filter_proxy_, &TableFilterProxyModel::RFiltersCleared);
    connect(filter_model_, &TableFilterModel::SSortRequested, filter_proxy_, &TableFilterProxyModel::RSortRequested);

    ui->tableViewFilter->setSortingEnabled(true);
}

void StatementPrimaryWidget::on_tableViewData_doubleClicked(const QModelIndex& index)
{
    if (index.column() != std::to_underlying(statement::PrimaryField::kPartnerName))
        return;

    const auto source_index { filter_proxy_->mapToSource(index) };
    if (!source_index.isValid())
        return;

    const auto* row { static_cast<const statement::PrimaryRow*>(source_index.internalPointer()) };
    if (!row)
        return;

    emit SShowSecondaryStatement(row->partner_id, range_, unit_);
}

void StatementPrimaryWidget::on_pushButtonClear_clicked()
{
    filter_model_->ClearFilters();

    auto* view { ui->tableViewFilter };
    view->clearSelection();
    view->setCurrentIndex({});
}
