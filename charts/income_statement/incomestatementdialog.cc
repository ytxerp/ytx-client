#include "incomestatementdialog.h"

#include <QMessageBox>
#include <QUuid>

#include "component/constant.h"
#include "component/signalblocker.h"
#include "ui_incomestatementdialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

IncomeStatementDialog::IncomeStatementDialog(CTreeModel* tree_model, IncomeStatementModel* model, const QUuid& widget_id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::IncomeStatementDialog)
    , start_ { QDateTime(QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1), kStartTime) }
    , end_ { QDateTime(QDate::currentDate().addDays(1), kStartTime) }
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

void IncomeStatementDialog::on_dateTimeEditEnd_dateChanged(const QDate& date)
{
    const bool valid { date >= start_.date() };

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);

    end_ = QDateTime(date.addDays(1), kStartTime);
}

void IncomeStatementDialog::on_pushButtonFetch_clicked()
{
    if (!ui->pushButtonFetch->isEnabled()) {
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

    const auto start_date { ui->dateTimeEditStart->dateTime().date() };
    const auto end_date { ui->dateTimeEditEnd->dateTime().date() };

    const int years { end_date.year() - start_date.year() + 1 };
    const int months { (end_date.year() - start_date.year()) * 12 + end_date.month() - start_date.month() + 1 };
    const int days { static_cast<int>(start_date.daysTo(end_date) + 1) };

    QJsonObject duration {};
    duration.insert(income_statement::kYears, years);
    duration.insert(income_statement::kMonths, months);
    duration.insert(income_statement::kDays, days);

    QJsonObject date_range {};
    date_range.insert(kStart, start_.toUTC().toString(Qt::ISODate));
    date_range.insert(kEnd, end_.toUTC().toString(Qt::ISODate));

    auto message { JsonGen::IncomeStatementAck(widget_id_, income_id, expense_id, level) };

    message.insert(income_statement::kDuration, duration);
    message.insert(income_statement::kDateRange, date_range);

    WebSocket::Instance()->SendMessage(WsKey::kIncomeStatementAck, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void IncomeStatementDialog::InitDialog()
{
    {
        auto* leaf_path_branch_path_model = new ItemModel(this);
        tree_model_->LeafPathBranchPathModel(leaf_path_branch_path_model);
        leaf_path_branch_path_model->sort(0);

        ui->comboBoxIncome->setModel(leaf_path_branch_path_model);
        ui->comboBoxExpense->setModel(leaf_path_branch_path_model);

        ui->comboBoxIncome->setCurrentIndex(-1);
        ui->comboBoxExpense->setCurrentIndex(-1);
    }

    ui->dateTimeEditStart->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateTimeEditEnd->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateTimeEditStart->setDateTime(start_);
    ui->dateTimeEditEnd->setDateTime(end_.addDays(-1));

    ui->comboBoxIncome->setFocus();
}

void IncomeStatementDialog::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pushButtonFetch->setEnabled(true); });
}

void IncomeStatementDialog::on_dateTimeEditStart_dateChanged(const QDate& date)
{
    const bool valid { date < end_.date() };
    start_ = QDateTime(date, kStartTime);

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}
