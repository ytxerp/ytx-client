#include "leafdeletedialog.h"

#include <QMessageBox>

#include "component/signalblocker.h"
#include "ui_leafdeletedialog.h"
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

void LeafDeleteDialog::on_pBtnOk_clicked()
{
    if (!ui->rBtnDelete->isChecked() && !ui->rBtnReplace->isChecked()) {
        return;
    }

    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Question);
    msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    const auto& path { model_->Path(node_id_) };
    QString text {};
    QString informative_text {};

    if (ui->rBtnDelete->isChecked()) {
        text = tr("Delete Node");
        informative_text = tr("Delete %1 and all its references. Are you sure").arg(path);
    }

    if (ui->rBtnReplace->isChecked()) {
        text = tr("Replace Node");
        informative_text = tr("Replace %1 with %2. This action is risky and cannot be undone. Are you sure?").arg(path, ui->comboBox->currentText());
    }

    msg.setText(text);
    msg.setInformativeText(informative_text);

    if (msg.exec() == QMessageBox::Ok) {
        if (ui->rBtnDelete->isChecked()) {
            emit SDeleteNode(node_id_);

            const auto message { JsonGen::LeafDelete(info_.section, node_id_) };
            WebSocket::Instance()->SendMessage(kLeafDelete, message);

            accept();
        }

        if (ui->rBtnReplace->isChecked()) {
            const auto new_node_id { ui->comboBox->currentData().toUuid() };

            const auto message { JsonGen::LeafReplace(info_.section, node_id_, new_node_id) };
            WebSocket::Instance()->SendMessage(kLeafReplace, message);
        }
    }
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
        QMessageBox::critical(
            this, tr("Replacement Conflict"), tr("The old node cannot be replaced because linked nodes or partner entries conflict with the new node."));
    }
}
