#include "balancesheetdialog.h"

#include <QMessageBox>
#include <QUuid>

#include "component/constantstring.h"
#include "component/signalblocker.h"
#include "ui_balancesheetdialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

BalanceSheetDialog::BalanceSheetDialog(CTreeModel* tree_model, balance_sheet::Model* model, const QUuid& widget_id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::BalanceSheetDialog)
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

BalanceSheetDialog::~BalanceSheetDialog() { delete ui; }

QTreeView* BalanceSheetDialog::View() { return ui->treeView; }

void BalanceSheetDialog::InitDialog()
{
    {
        auto* path_model { tree_model_->PathModel(this) };

        ui->comboBoxAsset->setModel(path_model);
        ui->comboBoxEquity->setModel(path_model);
        ui->comboBoxLiability->setModel(path_model);

        ui->comboBoxAsset->setCurrentIndex(-1);
        ui->comboBoxEquity->setCurrentIndex(-1);
        ui->comboBoxLiability->setCurrentIndex(-1);
    }

    ui->dateEditStart->setDisplayFormat(datetime_format::kDashedDate);
    ui->dateEditEnd->setDisplayFormat(datetime_format::kDashedDate);

    ui->dateEditStart->setDate(range_.start);
    ui->dateEditEnd->setDate(range_.end);

    ui->comboBoxAsset->setFocus();
}

void BalanceSheetDialog::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pushButtonFetch->setEnabled(true); });
}

void BalanceSheetDialog::on_dateEditStart_dateChanged(const QDate& date)
{
    const bool valid { date <= range_.end };
    range_.start = date;

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}

void BalanceSheetDialog::on_dateEditEnd_dateChanged(const QDate& date)
{
    const bool valid { date >= range_.start };
    range_.end = date;

    cooldown_timer_->stop();
    ui->pushButtonFetch->setEnabled(valid);
}

void BalanceSheetDialog::on_pushButtonFetch_clicked()
{
    if (!ui->pushButtonFetch->isEnabled()) {
        return;
    }

    if (!range_.IsValid()) {
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
        const bool overlap { node::IsDescendant(asset, liability) || node::IsDescendant(liability, asset) || node::IsDescendant(asset, equity)
            || node::IsDescendant(equity, asset) || node::IsDescendant(liability, equity) || node::IsDescendant(equity, liability) };

        if (overlap) {
            QMessageBox::warning(this, tr("Warning"), tr("Asset, liability, and equity nodes must not have ancestor-descendant relationships."));
            return;
        }
    }

    ui->pushButtonFetch->setEnabled(false);

    qDebug() << Q_FUNC_INFO << "DateRange:" << range_.ToString();

    const auto query_range { range_.ToQueryRange() };

    qDebug() << Q_FUNC_INFO << "QueryRange:" << query_range.ToString();

    const int level { ui->spinBoxLevel->value() };

    const auto message { JsonGen::BalanceSheetAck(widget_id_, asset_id, liability_id, equity_id, query_range, level) };

    WebSocket::Instance()->SendMessage(WsKey::kBalanceSheetAck, message);

    cooldown_timer_->start(time_const::kCooldownMs);
}
