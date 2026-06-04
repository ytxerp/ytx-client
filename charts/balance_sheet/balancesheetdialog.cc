#include "balancesheetdialog.h"

#include <QMessageBox>
#include <QUuid>

#include "component/constant.h"
#include "component/constantstring.h"
#include "component/signalblocker.h"
#include "ui_balancesheetdialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

BalanceSheetDialog::BalanceSheetDialog(CTreeModel* tree_model, BalanceSheetModel* model, const QUuid& widget_id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::BalanceSheetDialog)
    , widget_id_ { widget_id }
    , model_ { model }
    , tree_model_ { tree_model }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    const QDate today = QDate::currentDate();

    start_ = QDateTime(QDate(today.year(), today.month(), 1), kStartTime);
    end_ = QDateTime(today.addDays(1), kStartTime);

    InitTimer();
    InitDialog();

    ui->treeView->setModel(model);
    model->setParent(ui->treeView);
}

BalanceSheetDialog::~BalanceSheetDialog() { delete ui; }

QTreeView* BalanceSheetDialog::View() { return ui->treeView; }

void BalanceSheetDialog::InitDialog()
{
    {
        auto* leaf_path_branch_path_model = new ItemModel(this);
        tree_model_->LeafPathBranchPathModel(leaf_path_branch_path_model);
        leaf_path_branch_path_model->sort(0);

        ui->comboBoxAsset->setModel(leaf_path_branch_path_model);
        ui->comboBoxEquity->setModel(leaf_path_branch_path_model);
        ui->comboBoxLiability->setModel(leaf_path_branch_path_model);

        ui->comboBoxAsset->setCurrentIndex(-1);
        ui->comboBoxEquity->setCurrentIndex(-1);
        ui->comboBoxLiability->setCurrentIndex(-1);
    }

    ui->dateTimeEditStart->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateTimeEditStart->setDateTime(start_);

    ui->dateTimeEditEnd->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateTimeEditEnd->setDateTime(end_.addDays(-1));

    ui->comboBoxAsset->setFocus();
}

void BalanceSheetDialog::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pushButtonFetch->setEnabled(true); });
}

void BalanceSheetDialog::on_dateTimeEditEnd_dateChanged(const QDate& date)
{
    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(true);
    end_ = QDateTime(date.addDays(1), kStartTime);
}

void BalanceSheetDialog::on_pushButtonFetch_clicked()
{
    if (!ui->pushButtonFetch->isEnabled()) {
        return;
    }

    if (ui->comboBoxAsset->currentIndex() == -1 || ui->comboBoxEquity->currentIndex() == -1 || ui->comboBoxLiability->currentIndex() == -1)
        return;

    const auto asset_id { ui->comboBoxAsset->currentData().toUuid() };
    const auto liability_id { ui->comboBoxLiability->currentData().toUuid() };
    const auto equity_id { ui->comboBoxEquity->currentData().toUuid() };

    {
        if (asset_id == liability_id || asset_id == equity_id || liability_id == equity_id) {
            QMessageBox::warning(this, tr("Warning"), tr("Asset, liability, and equity nodes must be different."));
            return;
        }
    }

    const auto* asset { tree_model_->GetNode(asset_id) };
    const auto* liability { tree_model_->GetNode(liability_id) };
    const auto* equity { tree_model_->GetNode(equity_id) };

    {
        if (!asset || !liability || !equity) {
            QMessageBox::warning(this, tr("Warning"), tr("Selected balance sheet node no longer exists."));
            return;
        }
    }

    {
        const bool overlap { utils::IsDescendant(asset, liability) || utils::IsDescendant(liability, asset) || utils::IsDescendant(asset, equity)
            || utils::IsDescendant(equity, asset) || utils::IsDescendant(liability, equity) || utils::IsDescendant(equity, liability) };

        if (overlap) {
            QMessageBox::warning(this, tr("Warning"), tr("Asset, liability, and equity nodes must not have ancestor-descendant relationships."));
            return;
        }
    }

    ui->pushButtonFetch->setEnabled(false);

    const int level { ui->spinBoxLevel->value() };

    const auto message { JsonGen::BalanceSheetAck(widget_id_, asset_id, liability_id, equity_id, start_.toUTC(), end_.toUTC(), level) };

    WebSocket::Instance()->SendMessage(WsKey::kBalanceSheetAck, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}

void BalanceSheetDialog::on_dateTimeEditStart_dateChanged(const QDate& date)
{
    const bool valid { date < end_.date() };
    start_ = QDateTime(date, kStartTime);

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}
