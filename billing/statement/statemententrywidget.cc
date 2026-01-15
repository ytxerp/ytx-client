#include "statemententrywidget.h"

#include <QDir>
#include <QFileDialog>

#include "component/constant.h"
#include "component/signalblocker.h"
#include "global/exportexcel.h"
#include "ui_statemententrywidget.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

StatementEntryWidget::StatementEntryWidget(StatementEntryModel* model, Section section, CUuid& widget_id, CUuid& partner_id, int unit, CDateTime& start,
    CDateTime& end, CString& partner_name, CString& company_name, CUuidString& inventory_leaf, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StatementEntryWidget)
    , unit_ { unit }
    , start_ { start }
    , end_ { end }
    , model_ { model }
    , partner_name_ { partner_name }
    , company_name_ { company_name }
    , inventory_leaf_ { inventory_leaf }
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

    QTimer::singleShot(0, this, &StatementEntryWidget::on_pBtnFetch_clicked);
}

StatementEntryWidget::~StatementEntryWidget() { delete ui; }

QTableView* StatementEntryWidget::View() const { return ui->tableView; }

void StatementEntryWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= end_.date() };
    start_.setDate(date);

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void StatementEntryWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= start_.date() };

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
    end_ = QDateTime(date.addDays(1), kStartTime);
}

void StatementEntryWidget::on_pBtnFetch_clicked()
{
    if (!ui->pBtnFetch->isEnabled()) {
        return;
    }

    ui->pBtnFetch->setEnabled(false);

    const auto message { JsonGen::StatementEntryAcked(section_, widget_id_, partner_id_, unit_, start_.toUTC(), end_.toUTC()) };
    WebSocket::Instance()->SendMessage(kStatementEntryAcked, message);

    cooldown_timer_->start(kTwoThousand);
}

void StatementEntryWidget::RUnitGroupClicked(int id)
{
    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(start_ <= end_);

    unit_ = id;
}

void StatementEntryWidget::IniUnitGroup()
{
    unit_group_ = new QButtonGroup(this);
    unit_group_->addButton(ui->rBtnIMM, 0);
    unit_group_->addButton(ui->rBtnMON, 1);
    unit_group_->addButton(ui->rBtnPEN, 2);
}

void StatementEntryWidget::IniConnect() { connect(unit_group_, &QButtonGroup::idClicked, this, &StatementEntryWidget::RUnitGroupClicked); }

void StatementEntryWidget::IniUnit(int unit)
{
    const NodeUnit kUnit { unit };

    switch (kUnit) {
    case NodeUnit::OImmediate:
        ui->rBtnIMM->setChecked(true);
        break;
    case NodeUnit::OMonthly:
        ui->rBtnMON->setChecked(true);
        break;
    case NodeUnit::OPending:
        ui->rBtnPEN->setChecked(true);
        break;
    default:
        break;
    }
}

void StatementEntryWidget::IniWidget()
{
    ui->start->setDisplayFormat(kDateFST);
    ui->end->setDisplayFormat(kDateFST);

    ui->pBtnFetch->setFocus();

    ui->start->setDateTime(start_);
    ui->end->setDateTime(end_.addSecs(-1));
}

void StatementEntryWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}

void StatementEntryWidget::on_pBtnExport_clicked()
{
    // Adjust end time (make the range inclusive) ---
    const QDateTime adjust_end { end_.addSecs(-1).toLocalTime() };

    // Build default export file name ---
    QDir dir(QDir::homePath());
    const QString file_name { QString("%1-%2-%3.xlsx").arg(partner_name_, company_name_, adjust_end.toString(kMonthFST)) };
    const QString full_path { dir.filePath(file_name) };

    QString destination { QFileDialog::getSaveFileName(nullptr, tr("Export Excel"), full_path, "*.xlsx") };

    // Prepare the file (remove if exists)
    if (!Utils::PrepareNewFile(destination, kDotSuffixXLSX))
        return;

    auto& list { model_->EntryList() };
    const QString unit_string { Utils::UnitString(NodeUnit(unit_)) };

    ExportExcel::Instance().StatementAsync(destination, partner_name_, inventory_leaf_, unit_string, start_, adjust_end, total_, list);
}
