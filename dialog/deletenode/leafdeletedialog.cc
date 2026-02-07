#include "leafdeletedialog.h"

#include "component/signalblocker.h"
#include "exactmatchconfirmdialog.h"
#include "ui_leafdeletedialog.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

LeafDeleteDialog::LeafDeleteDialog(TreeModel* model, CSectionInfo& info, CJsonObject& obj, const QUuid& node_id, NodeUnit unit, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LeafDeleteDialog)
    , node_id_ { node_id }
    , node_unit_ { unit }
    , inside_ref_ { obj.value(kInsideRef).toBool() }
    , inventory_int_ref_ { obj.value(kInventoryIntRef).toBool() }
    , inventory_ext_ref_ { obj.value(kInventoryExtRef).toBool() }
    , partner_ref_ { obj.value(kPartnerRef).toBool() }
    , employee_ref_ { obj.value(kEmployeeRef).toBool() }
    , settlement_ref_ { obj.value(kSettlementRef).toBool() }
    , model_ { model }
    , info_ { info }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniData(info.section);
    IniOptionGroup();
    IniConnect();
}

LeafDeleteDialog::~LeafDeleteDialog() { delete ui; }

void LeafDeleteDialog::IniConnect()
{
    connect(WebSocket::Instance(), &WebSocket::SReplaceResult, this, &LeafDeleteDialog::RReplaceResult);
    connect(option_group_, &QButtonGroup::idClicked, this, &LeafDeleteDialog::RButtonGroup);
    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &LeafDeleteDialog::RcomboBoxCurrentIndexChanged);
}

void LeafDeleteDialog::IniOptionGroup()
{
    option_group_ = new QButtonGroup(this);
    option_group_->addButton(ui->rBtnDelete, 0);
    option_group_->addButton(ui->rBtnReplace, 1);
}

void LeafDeleteDialog::InitCheckBoxGroup()
{
    ui->chkBoxInside->setChecked(inside_ref_);
    ui->chkBoxInside->setEnabled(false);

    ui->chkBoxInventoryInt->hide();
    ui->chkBoxInventoryExt->hide();
    ui->chkBoxPartner->hide();
    ui->chkBoxEmployee->hide();
    ui->chkBoxSettlement->hide();
    ui->rBtnReplace->hide();
    ui->comboBox->hide();

    switch (info_.section) {
    case Section::kInventory:
        ui->chkBoxInventoryInt->show();
        ui->chkBoxInventoryInt->setChecked(inventory_int_ref_);
        ui->chkBoxInventoryInt->setEnabled(false);

        ui->chkBoxInventoryExt->show();
        ui->chkBoxInventoryExt->setChecked(inventory_ext_ref_);
        ui->chkBoxInventoryExt->setEnabled(false);

        ui->rBtnReplace->show();
        ui->comboBox->show();
        break;
    case Section::kPartner:
        if (node_unit_ == NodeUnit::PEmployee) {
            ui->chkBoxEmployee->show();
            ui->chkBoxEmployee->setChecked(employee_ref_);
            ui->chkBoxEmployee->setEnabled(false);
        } else {
            ui->chkBoxPartner->show();
            ui->chkBoxPartner->setChecked(partner_ref_);
            ui->chkBoxPartner->setEnabled(false);
        }
        break;
    case Section::kSale:
    case Section::kPurchase:
        ui->chkBoxSettlement->show();
        ui->chkBoxSettlement->setChecked(settlement_ref_);
        ui->chkBoxSettlement->setEnabled(false);
        break;
    case Section::kFinance:
    case Section::kTask:
        break;
    default:
        break;
    }
}

void LeafDeleteDialog::ReplaceNode()
{
    if (!ui->rBtnReplace->isChecked())
        return;

    const QString path { model_->Path(node_id_) };
    const QString new_path { ui->comboBox->currentText() };
    const QString info { tr("Replace <b>%1</b> with <b>%2</b>.<br>"
                            "<span style='color:#d32f2f; font-weight:bold;'><br>⚠️ This action is risky and cannot be undone!</span>")
            .arg(path, new_path) };

    auto* dlg { new ExactMatchConfirmDialog(info, path, tr("Replace"), this) };
    dlg->setModal(true);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();

    connect(dlg, &ExactMatchConfirmDialog::accepted, this, [=, this]() {
        const auto new_node_id { ui->comboBox->currentData().toUuid() };
        const auto message { JsonGen::LeafReplace(info_.section, node_id_, new_node_id) };
        WebSocket::Instance()->SendMessage(kLeafReplace, message);
    });
}

void LeafDeleteDialog::DeleteNode()
{
    if (!ui->rBtnDelete->isChecked())
        return;

    const auto& path { model_->Path(node_id_) };
    const QString info { tr("Delete <b>%1</b> and all its references.<br>"
                            "<span style='color:#d32f2f; font-weight:bold;'><br>⚠️ Permanent deletion! Cannot be undone!</span>"
                            "<br><br><i>Tip: It is recommended to move all entries referencing this node before deletion.</i>")
            .arg(path) };

    auto* dlg { new ExactMatchConfirmDialog(info, path, tr("Delete"), this) };
    dlg->setModal(true);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();

    connect(dlg, &ExactMatchConfirmDialog::accepted, this, [=, this]() {
        const auto message { JsonGen::LeafDelete(info_.section, node_id_) };
        WebSocket::Instance()->SendMessage(kLeafDelete, message);
        accept();
    });
}

void LeafDeleteDialog::on_pBtnOk_clicked()
{
    DeleteNode();
    ReplaceNode();
}

void LeafDeleteDialog::IniData(Section section)
{
    ui->label->setWordWrap(true);
    ui->pBtnCancel->setDefault(true);

    InitCheckBoxGroup();

    this->setWindowTitle(tr("Delete %1").arg(model_->Path(node_id_)));

    if (inventory_int_ref_ || inventory_ext_ref_ || partner_ref_ || employee_ref_ || settlement_ref_) {
        ui->rBtnDelete->setEnabled(false);
        ui->label->setText(tr("The node has external references, so it can’t be deleted."));
    }

    if (section != Section::kInventory)
        return;

    auto* filter_model { model_->ReplaceSelf(node_id_, node_unit_, this) };

    ui->comboBox->setModel(filter_model);
    ui->comboBox->setCurrentIndex(-1);
}

void LeafDeleteDialog::RcomboBoxCurrentIndexChanged(int /*index*/)
{
    const auto new_node_id { ui->comboBox->currentData().toUuid() };
    ui->pBtnOk->setEnabled(!new_node_id.isNull());
}

void LeafDeleteDialog::RButtonGroup(int id)
{
    ui->pBtnOk->setEnabled(id == 0 || ui->comboBox->currentIndex() != -1);
    ui->comboBox->setEnabled(id == 1);
}

void LeafDeleteDialog::RReplaceResult(bool result)
{
    if (result) {
        accept();
    } else {
        Utils::ShowNotification(QMessageBox::Critical, tr("Replacement Conflict"),
            tr("The old node cannot be replaced because linked nodes or partner entries conflict with the new node."), kThreeThousand);
    }
}
