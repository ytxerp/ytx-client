#include "leafdeletedialog.h"

#include "component/constantwebsocket.h"
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
    , section_ { info.section }
    , within_ { obj.value(NodeRef::kWithin).toBool() }
    , inventory_int_ { obj.value(NodeRef::kInventoryInt).toBool() }
    , inventory_ext_ { obj.value(NodeRef::kInventoryExt).toBool() }
    , partner_cv_ { obj.value(NodeRef::kPartnerCV).toBool() }
    , partner_emp_ { obj.value(NodeRef::kPartnerEmp).toBool() }
    , order_ { obj.value(NodeRef::kOrder).toBool() }
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
    ui->chkBoxWithin->setChecked(within_);
    ui->chkBoxWithin->setEnabled(false);

    ui->chkBoxInventoryInt->hide();
    ui->chkBoxInventoryExt->hide();
    ui->chkBoxPartnerCV->hide();
    ui->chkBoxPartnerEmp->hide();
    ui->chkBoxOrder->hide();
    ui->rBtnReplace->hide();
    ui->comboBox->hide();

    switch (section_) {
    case Section::kInventory:
        ui->chkBoxInventoryInt->show();
        ui->chkBoxInventoryInt->setChecked(inventory_int_);
        ui->chkBoxInventoryInt->setEnabled(false);

        ui->chkBoxInventoryExt->show();
        ui->chkBoxInventoryExt->setChecked(inventory_ext_);
        ui->chkBoxInventoryExt->setEnabled(false);

        ui->rBtnReplace->show();
        ui->comboBox->show();
        break;
    case Section::kPartner:
        if (node_unit_ == NodeUnit::PEmployee) {
            ui->chkBoxPartnerEmp->show();
            ui->chkBoxPartnerEmp->setChecked(partner_emp_);
            ui->chkBoxPartnerEmp->setEnabled(false);
        } else {
            ui->chkBoxPartnerCV->show();
            ui->chkBoxPartnerCV->setChecked(partner_cv_);
            ui->chkBoxPartnerCV->setEnabled(false);
        }
        break;
    case Section::kSale:
    case Section::kPurchase:
        ui->chkBoxOrder->show();
        ui->chkBoxOrder->setChecked(order_);
        ui->chkBoxOrder->setEnabled(false);
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
    const QString info { tr("Replace <b>%1</b> with <b>%2</b>."
                            "<br><br>"
                            "<span style='color:#d32f2f; font-weight:bold;'>"
                            "⚠️ This action is permanent and cannot be undone."
                            "</span>")
            .arg(path, new_path) };

    auto* dlg { new ExactMatchConfirmDialog(info, tr("Replace"), this) };
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    connect(dlg, &ExactMatchConfirmDialog::accepted, this, [this, path]() {
        if (!model_->Contains(node_id_))
            return;

        if (model_->Path(node_id_) != path)
            return;

        const auto new_node_id { ui->comboBox->currentData().toUuid() };
        const auto message { JsonGen::LeafReplace(info_.section, node_id_, new_node_id) };
        WebSocket::Instance()->SendMessage(WsKey::kLeafReplace, message);
    });

    dlg->show();
}

void LeafDeleteDialog::DeleteNode()
{
    if (!ui->rBtnDelete->isChecked())
        return;

    const auto& path { model_->Path(node_id_) };
    QString info {};

    switch (section_) {
    case Section::kSale:
    case Section::kPurchase:
        info = tr("Delete this order and all its entries."
                  "<br><br>"
                  "<span style='color:#d32f2f; font-weight:bold;'>"
                  "⚠️ This action is permanent and cannot be undone."
                  "</span>");
        break;
    case Section::kPartner:
        info = tr("Delete <b>%1</b> and all its entries."
                  "<br><br>"
                  "<span style='color:#d32f2f; font-weight:bold;'>"
                  "⚠️ This action is permanent and cannot be undone."
                  "</span>")
                   .arg(path);
        break;
    case Section::kFinance:
    case Section::kTask:
    case Section::kInventory:
        info = tr("Delete <b>%1</b> and all its entries."
                  "<br><br>"
                  "<span style='color:#d32f2f; font-weight:bold;'>"
                  "⚠️ This action is permanent and cannot be undone."
                  "</span>"
                  "<br><br>"
                  "<i>💡 Tip: Consider relocating entries before deleting.</i>")
                   .arg(path);
        break;
    }

    auto* dlg { new ExactMatchConfirmDialog(info, tr("Delete"), this) };
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    connect(dlg, &ExactMatchConfirmDialog::accepted, this, [this, path]() {
        if (!model_->Contains(node_id_))
            return;

        if (model_->Path(node_id_) != path)
            return;

        WsKey key {};
        QJsonObject value {};

        switch (section_) {
        case Section::kSale:
        case Section::kPurchase:
            key = WsKey::kLeafDeleteO;
            value = JsonGen::LeafDeleteO(info_.section, node_id_);
            break;
        case Section::kPartner:
            key = WsKey::kLeafDeleteP;
            value = JsonGen::LeafDeleteP(info_.section, node_id_);
            break;
        case Section::kFinance:
        case Section::kTask:
        case Section::kInventory:
            key = WsKey::kLeafDelete;
            value = JsonGen::LeafDelete(info_.section, node_id_);
            break;
        }

        WebSocket::Instance()->SendMessage(key, value);
        close();
    });

    dlg->show();
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

    if (inventory_int_ || inventory_ext_ || partner_cv_ || partner_emp_ || order_) {
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
        close();
    } else {
        Utils::ShowNotification(QMessageBox::Critical, tr("Replacement Conflict"),
            tr("The old node cannot be replaced because linked nodes or partner entries conflict with the new node."), TimeConst::kAutoCloseMs);
    }
}
