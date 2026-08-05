#include "statementtertiarywidget.h"

#include <QDir>
#include <QFileDialog>

#include "component/constant.h"
#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "global/exportexcel.h"
#include "ui_statementtertiarywidget.h"
#include "utils/mainwindowutils.h"
#include "utils/nodeutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

StatementTertiaryWidget::StatementTertiaryWidget(statement::TertiaryModel* model, CUuid& widget_id, CUuid& partner_id, const utils::DateRange& range,
    CString& partner_name, CString& company_name, Section section, int unit, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StatementTertiaryWidget)
    , unit_ { unit }
    , range_ { range }
    , model_ { model }
    , partner_name_ { partner_name }
    , company_name_ { company_name }
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

    QTimer::singleShot(0, this, &StatementTertiaryWidget::on_pBtnFetch_clicked);
}

StatementTertiaryWidget::~StatementTertiaryWidget() { delete ui; }

QTableView* StatementTertiaryWidget::View() const { return ui->tableView; }

void StatementTertiaryWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= range_.end };
    range_.start = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void StatementTertiaryWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= range_.start };
    range_.end = date;

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void StatementTertiaryWidget::on_pBtnFetch_clicked()
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

    const auto message { JsonGen::StatementTertiary(section_, widget_id_, partner_id_, unit_, query_range) };
    WebSocket::Instance()->SendMessage(WsKey::kStatementTertiary, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void StatementTertiaryWidget::RUnitGroupClicked(int id)
{
    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(range_.IsValid());

    unit_ = id;
}

void StatementTertiaryWidget::IniUnitGroup()
{
    unit_group_ = new QButtonGroup(this);
    unit_group_->addButton(ui->rBtnIS, std::to_underlying(NodeUnit::OImmediate));
    unit_group_->addButton(ui->rBtnMS, std::to_underlying(NodeUnit::OMonthly));
    unit_group_->addButton(ui->rBtnPEND, std::to_underlying(NodeUnit::OPending));
}

void StatementTertiaryWidget::IniConnect() { connect(unit_group_, &QButtonGroup::idClicked, this, &StatementTertiaryWidget::RUnitGroupClicked); }

void StatementTertiaryWidget::IniUnit(int unit)
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

void StatementTertiaryWidget::IniWidget()
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

void StatementTertiaryWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}

void StatementTertiaryWidget::on_pBtnExport_clicked()
{
    // Build default export file name ---
    QDir dir(QDir::homePath());
    const QString file_name { QStringLiteral("%1-%2-%3-%4.xlsx")
            .arg(company_name_, partner_name_, range_.start.toString(datetime_format::kCompactDate), range_.end.toString(datetime_format::kCompactDate)) };
    const QString full_path { dir.filePath(file_name) };

    QString destination { QFileDialog::getSaveFileName(nullptr, tr("Export Excel"), full_path, "*.xlsx") };

    // Prepare the file (remove if exists)
    if (!utils::PrepareNewFile(destination, kDotSuffixXLSX))
        return;

    auto& list { model_->EntryList() };
    const QString unit_string { node::UnitString(NodeUnit(unit_)) };

    ExportExcel::Instance().StatementAsync(destination, partner_name_, partner_id_, unit_string, range_, total_, list);
}
