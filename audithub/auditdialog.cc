#include "auditdialog.h"

#include "component/constantint.h"
#include "ui_auditdialog.h"

AuditDialog::AuditDialog(const audit_hub::AuditInfo& info, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AuditDialog)
    , info_ { info }
    , model_ { new audit_hub::AuditModel(info, this) }
{
    ui->setupUi(this);
    ui->tableView->setModel(model_);

    InitTimer();
}

AuditDialog::~AuditDialog() { delete ui; }

QTableView* AuditDialog::View() { return ui->tableView; }

void AuditDialog::on_pBtnFetch_clicked()
{
    ui->pBtnFetch->setEnabled(false);
    cooldown_timer_->start(TimeConst::kCooldownMs);
    emit SRefresh();
}

void AuditDialog::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}
