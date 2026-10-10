#include "statementtertiarywidget.h"

#include <QDir>
#include <QFileDialog>

#include "component/constant.h"
#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "global/exporthub.h"
#include "global/masterdataregistry.h"
#include "global/partner_inventory_registry.h"
#include "statementenum.h"
#include "ui_statementtertiarywidget.h"
#include "utils/mainwindowutils.h"
#include "utils/nodeutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

StatementTertiaryWidget::StatementTertiaryWidget(CStringList& header, CUuid& widget_id, CUuid& partner_id, const utils::DateRange& range, CString& partner_name,
    CString& company_name, Section section, int unit, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StatementTertiaryWidget)
    , unit_ { unit }
    , range_ { range }
    , partner_name_ { partner_name }
    , company_name_ { company_name }
    , section_ { section }
    , widget_id_ { widget_id }
    , partner_id_ { partner_id }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniUnitGroup();
    IniWidget();
    InitTimer();
    InitModel(header, partner_id);
    IniUnit(unit);
    IniConnect();

    QTimer::singleShot(0, this, &StatementTertiaryWidget::on_pBtnFetch_clicked);
}

StatementTertiaryWidget::~StatementTertiaryWidget() { delete ui; }

QTableView* StatementTertiaryWidget::DataView() const { return ui->tableView; }

QTableView* StatementTertiaryWidget::SummaryView() const { return ui->tableViewSummary; }

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

void StatementTertiaryWidget::InitModel(const QStringList& header, CUuid& partner_id)
{
    data_model_ = new statement::TertiaryModel(header, partner_id, this);
    summary_model_ = new TableSummaryModel(header, this);

    ui->tableView->setModel(data_model_);
    ui->tableViewSummary->setModel(summary_model_);

    connect(data_model_, &statement::TertiaryModel::SSummaryChanged, summary_model_, &TableSummaryModel::RSummaryChanged);
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

    const auto list { data_model_->EntryList() };
    const auto header { data_model_->Header() };
    const auto summary { summary_model_->Values() };
    const QString unit_string { node::UnitString(NodeUnit(unit_)) };

    const auto lines { BuildExportLines(header, list, summary) };

    ExportHub::Instance().StatementTertiaryAsync(destination, partner_name_, unit_string, range_, lines);
}

QList<QVariantList> StatementTertiaryWidget::BuildExportLines(CStringList& header, const QList<statement::TertiaryRow>& list, const QList<QVariant>& summary)
{
    const int column_count { static_cast<int>(header.size()) };

    Q_ASSERT(summary.size() == column_count);

    QList<QVariantList> lines {};
    lines.reserve(list.size() + 3);

    // ===========================
    // Table Header
    // ===========================
    QVariantList header_line {};
    header_line.reserve(column_count);

    for (const auto& title : header)
        header_line.append(title);

    lines.append(std::move(header_line));

    // ===========================
    // Table Data
    // ===========================

    const auto& master { MasterDataRegistry::Instance() };
    const auto& partner { PartnerInventoryRegistry::Instance() };

    for (const auto& entry : list) {
        QVariantList line {};
        line.reserve(column_count);

        for (int column {}; column != column_count; ++column) {
            switch (static_cast<statement::TertiaryField>(column)) {
            case statement::TertiaryField::kIssuedTime:
                line.append(entry.issued_time.toString(datetime_format::kDashedDate));
                break;
            case statement::TertiaryField::kCode:
                line.append(entry.code);
                break;
            case statement::TertiaryField::kInternalSku:
                line.append(master.InventoryPath(entry.internal_sku));
                break;
            case statement::TertiaryField::kCount:
                line.append(entry.count);
                break;
            case statement::TertiaryField::kMeasure:
                line.append(entry.measure);
                break;
            case statement::TertiaryField::kUnitPrice:
                line.append(entry.unit_price);
                break;
            case statement::TertiaryField::kAmount:
                line.append(entry.amount);
                break;
            case statement::TertiaryField::kDescription:
                line.append(entry.description);
                break;
            case statement::TertiaryField::kStatus:
                line.append(QVariant {});
                break;
            case statement::TertiaryField::kExternalSku:
                line.append(partner.ExternalSku(partner_id_, entry.internal_sku));
                break;
            }
        }

        lines.append(std::move(line));
    }

    // ===========================
    // Empty Row + Summary
    // ===========================
    lines.append(QVariantList(column_count)); // spacer row: all empty cells
    lines.append(summary);

    return lines;
}
