#include "statementsecondarywidget.h"

#include <QDir>
#include <QFileDialog>
#include <QTimer>

#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "enum/nodeenum.h"
#include "global/exportexcel.h"
#include "statementenum.h"
#include "ui_statementsecondarywidget.h"
#include "utils/mainwindowutils.h"
#include "utils/nodeutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

StatementSecondaryWidget::StatementSecondaryWidget(
    CStringList& header, CUuid& widget_id, CUuid& partner_id, CString& partner_name, const utils::DateRange& range, Section section, int unit, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StatementSecondaryWidget)
    , unit_ { unit }
    , range_ { range }
    , section_ { section }
    , widget_id_ { widget_id }
    , partner_id_ { partner_id }
    , partner_name_ { partner_name }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniUnitGroup();
    IniWidget();
    InitTimer();
    InitModel(header);
    IniUnit(unit);
    IniConnect();

    QTimer::singleShot(0, this, &StatementSecondaryWidget::on_pBtnFetch_clicked);
}

StatementSecondaryWidget::~StatementSecondaryWidget() { delete ui; }

QTableView* StatementSecondaryWidget::DataView() const { return ui->tableView; }

QTableView* StatementSecondaryWidget::SummaryView() const { return ui->tableViewSummary; }

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

void StatementSecondaryWidget::InitModel(const QStringList& header)
{
    data_model_ = new statement::SecondaryModel(header, this);
    summary_model_ = new TableSummaryModel(header, this);

    ui->tableView->setModel(data_model_);
    ui->tableViewSummary->setModel(summary_model_);

    connect(data_model_, &statement::SecondaryModel::SSummaryChanged, summary_model_, &TableSummaryModel::RSummaryChanged);
}

void StatementSecondaryWidget::on_tableView_doubleClicked(const QModelIndex& index)
{
    if (index.column() == std::to_underlying(statement::SecondaryField::kIssuedTime)) {
        emit SShowTertiaryStatement(partner_id_, range_, unit_);
    }
}

void StatementSecondaryWidget::on_pushButtonExport_clicked()
{
    // Build default export file name ---
    QDir dir(QDir::homePath());
    const QString file_name { QStringLiteral("%1-%2-%3.xlsx")
            .arg(partner_name_, range_.start.toString(datetime_format::kCompactDate), range_.end.toString(datetime_format::kCompactDate)) };
    const QString full_path { dir.filePath(file_name) };

    QString destination { QFileDialog::getSaveFileName(nullptr, tr("Export Excel"), full_path, "*.xlsx") };

    // Prepare the file (remove if exists)
    if (!utils::PrepareNewFile(destination, kDotSuffixXLSX))
        return;

    const auto list { data_model_->EntryList() };
    const auto header { data_model_->Header() };
    const auto summary { summary_model_->Values() };
    const QString unit_string { node::UnitString(NodeUnit(unit_)) };

    const auto lines { BuildExportLines(header, list, summary) };

    ExportExcel::Instance().StatementSecondaryAsync(destination, partner_name_, unit_string, range_, lines);
}

QList<QVariantList> StatementSecondaryWidget::BuildExportLines(CStringList& header, const QList<statement::SecondaryRow>& list, const QList<QVariant>& summary)
{
    const int column_count { static_cast<int>(header.size()) };

    Q_ASSERT(summary.size() == column_count);

    QList<QVariantList> lines {};
    lines.reserve(list.size() + 3);

    // Header row (CStringList -> QVariantList)
    QVariantList header_line {};
    header_line.reserve(column_count);

    for (const auto& title : header)
        header_line.append(title);

    lines.append(std::move(header_line));

    for (const auto& entry : list) {
        QVariantList line {};
        line.reserve(column_count);

        for (int column { 0 }; column != column_count; ++column) {
            switch (static_cast<statement::SecondaryField>(column)) {
            case statement::SecondaryField::kIssuedTime:
                line.append(entry.issued_time.toString(datetime_format::kDashedDate));
                break;
            case statement::SecondaryField::kCode:
                line.append(entry.code);
                break;
            case statement::SecondaryField::kCount:
                line.append(entry.count);
                break;
            case statement::SecondaryField::kMeasure:
                line.append(entry.measure);
                break;
            case statement::SecondaryField::kAmount:
                line.append(entry.amount);
                break;
            case statement::SecondaryField::kDescription:
                line.append(entry.description);
                break;
            case statement::SecondaryField::kStatus:
                line.append(QVariant {});
                break;
            case statement::SecondaryField::kEmployee:
                line.append(QVariant {});
                break;
            }
        }

        lines.append(std::move(line));
    }

    lines.append(QVariantList(column_count)); // spacer row: all empty cells
    lines.append(summary); // total row

    return lines;
}
