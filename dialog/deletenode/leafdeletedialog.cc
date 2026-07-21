#include "leafdeletedialog.h"

#include "component/constantstring.h"
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
    , internal_ { obj.value(node_ref::kInternal).toBool() }
    , sale_ { obj.value(node_ref::kSale).toBool() }
    , purchase_ { obj.value(node_ref::kPurchase).toBool() }
    , partner_ { obj.value(node_ref::kPartner).toBool() }
    , settlement_ { obj.value(node_ref::kSettlement).toBool() }
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
    ui->chkBoxInternal->setChecked(internal_);
    ui->chkBoxInternal->setEnabled(false);

    ui->chkBoxPartner->hide();
    ui->chkBoxSale->hide();
    ui->chkBoxPurchase->hide();
    ui->chkBoxSettlement->hide();
    ui->rBtnReplace->hide();
    ui->comboBox->hide();

    switch (section_) {
    case Section::kInventory:
        ui->chkBoxSale->show();
        ui->chkBoxSale->setChecked(sale_);
        ui->chkBoxSale->setEnabled(false);

        ui->chkBoxPurchase->show();
        ui->chkBoxPurchase->setChecked(purchase_);
        ui->chkBoxPurchase->setEnabled(false);

        ui->chkBoxPartner->show();
        ui->chkBoxPartner->setChecked(partner_);
        ui->chkBoxPartner->setEnabled(false);

        ui->rBtnReplace->show();
        ui->comboBox->show();
        break;
    case Section::kPartner:
        ui->chkBoxSale->show();
        ui->chkBoxSale->setChecked(sale_);
        ui->chkBoxSale->setEnabled(false);

        ui->chkBoxPurchase->show();
        ui->chkBoxPurchase->setChecked(purchase_);
        ui->chkBoxPurchase->setEnabled(false);
        break;
    case Section::kSale:
    case Section::kPurchase:
        ui->chkBoxSettlement->show();
        ui->chkBoxSettlement->setChecked(settlement_);
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
        const QJsonObject value { JsonGen::LeafDelete(info_.section, node_id_) };

        switch (section_) {
        case Section::kSale:
        case Section::kPurchase:
            key = WsKey::kLeafDeleteO;
            break;
        case Section::kPartner:
            key = WsKey::kLeafDeleteP;
            break;
        case Section::kFinance:
        case Section::kTask:
        case Section::kInventory:
            key = WsKey::kLeafDelete;
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

    if (sale_ || partner_ || partner_cv_ || partner_emp_ || settlement_) {
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
        utils::ShowNotification(QMessageBox::Critical, tr("Replacement Conflict"),
            tr("The old node cannot be replaced because linked nodes or partner entries conflict with the new node."), time_const::kAutoCloseMs);
    }
}
