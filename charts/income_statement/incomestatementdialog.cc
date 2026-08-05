#include "incomestatementdialog.h"

#include <QMessageBox>
#include <QUuid>

#include "component/constant.h"
#include "component/signalblocker.h"
#include "ui_incomestatementdialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

IncomeStatementDialog::IncomeStatementDialog(CTreeModel* tree_model, income_statement::Model* model, const QUuid& widget_id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::IncomeStatementDialog)
    , range_ { DefaultRange() }
    , widget_id_ { widget_id }
    , model_ { model }
    , tree_model_ { tree_model }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    InitTimer();
    InitDialog();

    ui->treeView->setModel(model);
    model->setParent(ui->treeView);
}

IncomeStatementDialog::~IncomeStatementDialog() { delete ui; }

QTreeView* IncomeStatementDialog::View() { return ui->treeView; }

void IncomeStatementDialog::on_dateEditStart_dateChanged(const QDate& date)
{
    const bool valid { date <= range_.end };
    range_.start = date;

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}

void IncomeStatementDialog::on_dateEditEnd_dateChanged(const QDate& date)
{
    const bool valid { date >= range_.start };
    range_.end = date;

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}

void IncomeStatementDialog::on_pushButtonFetch_clicked()
{
    if (!ui->pushButtonFetch->isEnabled()) {
        return;
    }

    if (!range_.IsValid()) {
        return;
    }

    if (ui->comboBoxIncome->currentIndex() == -1 || ui->comboBoxExpense->currentIndex() == -1)
        return;

    const auto income_id { ui->comboBoxIncome->currentData().toUuid() };
    const auto expense_id { ui->comboBoxExpense->currentData().toUuid() };

    {
        if (income_id == expense_id) {
            QMessageBox::warning(this, tr("Warning"), tr("Income and expense nodes must be different."));
            return;
        }
    }

    const auto* income { tree_model_->GetNode(income_id) };
    const auto* expense { tree_model_->GetNode(expense_id) };

    {
        if (!income || !expense) {
            QMessageBox::warning(this, tr("Warning"), tr("Selected income statement node no longer exists."));
            return;
        }
    }

    {
        const bool overlap { node::IsDescendant(income, expense) || node::IsDescendant(expense, income) };

        if (overlap) {
            QMessageBox::warning(this, tr("Warning"), tr("Income and expense nodes must not have ancestor-descendant relationships."));
            return;
        }
    }

    ui->pushButtonFetch->setEnabled(false);

    const int level { ui->spinBoxLevel->value() };

    qDebug() << Q_FUNC_INFO << "DateRange:" << range_.ToString();

    const auto query_range { range_.ToQueryRange() };

    qDebug() << Q_FUNC_INFO << "QueryRange:" << query_range.ToString();

    const auto message { JsonGen::IncomeStatementAck(widget_id_, income_id, expense_id, query_range, level) };
    WebSocket::Instance()->SendMessage(WsKey::kIncomeStatementAck, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void IncomeStatementDialog::InitDialog()
{
    {
        auto* path_model { tree_model_->PathModel(this) };

        ui->comboBoxIncome->setModel(path_model);
        ui->comboBoxExpense->setModel(path_model);

        ui->comboBoxIncome->setCurrentIndex(-1);
        ui->comboBoxExpense->setCurrentIndex(-1);
    }

    ui->dateEditStart->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateEditEnd->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateEditStart->setDate(range_.start);
    ui->dateEditEnd->setDate(range_.end);

    ui->comboBoxIncome->setFocus();
}

void IncomeStatementDialog::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pushButtonFetch->setEnabled(true); });
}
