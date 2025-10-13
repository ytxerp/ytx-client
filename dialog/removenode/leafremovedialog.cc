#include "leafremovedialog.h"

#include <QMessageBox>

#include "component/signalblocker.h"
#include "tree/excludeonefiltermodel.h"
#include "ui_leafremovedialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

LeafRemoveDialog::LeafRemoveDialog(CTreeModel* model, CSectionInfo& info, CJsonObject& obj, const QUuid& node_id, int unit, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LeafRemoveDialog)
    , node_id_ { node_id }
    , node_unit_ { unit }
    , internally_ { obj.value(kInternallyRef).toBool() }
    , inventory_internal_ { obj.value(kInventoryInternalRef).toBool() }
    , inventory_external_ { obj.value(kInventoryExternalRef).toBool() }
    , partner_ { obj.value(kPartnerRef).toBool() }
    , employee_ { obj.value(kEmployeeRef).toBool() }
    , settlement_ { obj.value(kSettlementRef).toBool() }
    , model_ { model }
    , info_ { info }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniData(info.section);
    IniOptionGroup();
    IniConnect();
}

LeafRemoveDialog::~LeafRemoveDialog() { delete ui; }

void LeafRemoveDialog::IniConnect()
{
    connect(WebSocket::Instance(), &WebSocket::SReplaceResult, this, &LeafRemoveDialog::RReplaceResult);
    connect(option_group_, &QButtonGroup::idClicked, this, &LeafRemoveDialog::RButtonGroup);
    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &LeafRemoveDialog::RcomboBoxCurrentIndexChanged);
}

void LeafRemoveDialog::IniOptionGroup()
{
    option_group_ = new QButtonGroup(this);
    option_group_->addButton(ui->rBtnRemove, 0);
    option_group_->addButton(ui->rBtnReplace, 1);
}

void LeafRemoveDialog::InitCheckBoxGroup()
{
    ui->chkBoxInternally->setChecked(internally_);
    ui->chkBoxInternally->setEnabled(false);

    ui->chkBoxInventoryINT->setChecked(inventory_internal_);
    ui->chkBoxInventoryINT->setEnabled(false);

    ui->chkBoxInventoryEXT->setChecked(inventory_external_);
    ui->chkBoxInventoryEXT->setEnabled(false);

    ui->chkBoxPartner->setChecked(partner_);
    ui->chkBoxPartner->setEnabled(false);

    ui->chkBoxEmployee->setChecked(employee_);
    ui->chkBoxEmployee->setEnabled(false);

    ui->chkBoxSettlement->setChecked(settlement_);
    ui->chkBoxSettlement->setEnabled(false);
}

void LeafRemoveDialog::on_pBtnOk_clicked()
{
    if (!ui->rBtnRemove->isChecked() && !ui->rBtnReplace->isChecked()) {
        return;
    }

    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Question);
    msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    const auto& path { model_->Path(node_id_) };
    QString text {};
    QString informative_text {};

    if (ui->rBtnRemove->isChecked()) {
        text = tr("Remove Node");
        informative_text = tr("Remove %1 and all its references. Are you sure").arg(path);
    }

    if (ui->rBtnReplace->isChecked()) {
        text = tr("Replace Node");
        informative_text = tr("Replace %1 with %2. This action is risky and cannot be undone. Are you sure?").arg(path, ui->comboBox->currentText());
    }

    msg.setText(text);
    msg.setInformativeText(informative_text);

    if (msg.exec() == QMessageBox::Ok) {
        if (ui->rBtnRemove->isChecked()) {
            emit SRemoveNode(node_id_);

            const auto message { JsonGen::LeafRemove(info_.section, node_id_) };
            WebSocket::Instance()->SendMessage(kLeafRemove, message);

            accept();
        }

        if (ui->rBtnReplace->isChecked()) {
            const auto new_node_id { ui->comboBox->currentData().toUuid() };

            const bool inventory_external_ref { inventory_internal_ || inventory_external_ };

            const auto message { JsonGen::LeafReplace(info_.section, node_id_, new_node_id, inventory_external_ref) };
            WebSocket::Instance()->SendMessage(kLeafReplace, message);
        }
    }
}

void LeafRemoveDialog::IniData(Section section)
{
    ui->label->setWordWrap(true);
    ui->pBtnCancel->setDefault(true);

    InitCheckBoxGroup();

    this->setWindowTitle(tr("Remove %1").arg(model_->Path(node_id_)));

    if (inventory_internal_ || inventory_external_ || partner_ || employee_ || settlement_) {
        ui->rBtnRemove->setEnabled(false);
        ui->label->setText(tr("The node has external references, so it can’t be removed."));
    }

    if (section == Section::kSale || section == Section::kPurchase || section == Section::kPartner) {
        ui->rBtnReplace->setEnabled(false);
        ui->comboBox->setEnabled(false);
        return;
    }

    auto* filter_model { new ExcludeOneFilterModel(node_id_, this) };
    filter_model->setSourceModel(model_->LeafModel());

    ui->comboBox->setModel(filter_model);
    ui->comboBox->setCurrentIndex(-1);
}

void LeafRemoveDialog::RcomboBoxCurrentIndexChanged(int /*index*/)
{
    const auto new_node_id { ui->comboBox->currentData().toUuid() };
    ui->pBtnOk->setEnabled(!new_node_id.isNull() && model_->Unit(new_node_id) == node_unit_);
}

void LeafRemoveDialog::RButtonGroup(int /*id*/) { ui->pBtnOk->setEnabled(true); }

void LeafRemoveDialog::RReplaceResult(bool result)
{
    if (result) {
        this->accept();
    } else
        QMessageBox::critical(this, tr("Replacement Conflict"),
            tr("A reference conflict exists between the linked nodes of the old node and the new node. The replacement operation has been canceled."));
}
