#include "leafdeletedialog.h"

#include "component/constantstring.h"
#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "exactmatchconfirmdialog.h"
#include "ui_leafdeletedialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

LeafDeleteDialog::LeafDeleteDialog(Section section, CString& path, CJsonObject& obj, Node* node, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LeafDeleteDialog)
    , node_id_ { node->id }
    , section_ { section }
    , path_ { path }
    , node_ { node }
    , internal_ { obj.value(node_ref::kInternal).toBool() }
    , sale_ { obj.value(node_ref::kSale).toBool() }
    , purchase_ { obj.value(node_ref::kPurchase).toBool() }
    , partner_ { obj.value(node_ref::kPartner).toBool() }
    , settlement_ { obj.value(node_ref::kSettlement).toBool() }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniData();
}

LeafDeleteDialog::~LeafDeleteDialog() { delete ui; }

void LeafDeleteDialog::InitCheckBoxGroup()
{
    ui->chkBoxInternal->setChecked(internal_);
    ui->chkBoxInternal->setEnabled(false);

    ui->chkBoxPartner->hide();
    ui->chkBoxSale->hide();
    ui->chkBoxPurchase->hide();
    ui->chkBoxSettlement->hide();

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

void LeafDeleteDialog::on_pBtnOk_clicked()
{
    if (!ui->chkBoxDelete->isEnabled() || !ui->chkBoxDelete->isChecked())
        return;

    QString info {};
    WsKey key {};

    switch (section_) {
    case Section::kSale:
    case Section::kPurchase:
        info = tr("Delete this order and all its entries."
                  "<br><br>"
                  "<span style='color:#d32f2f; font-weight:bold;'>"
                  "⚠️ This action is permanent and cannot be undone."
                  "</span>");
        key = WsKey::kOrderLeafDelete;
        break;
    case Section::kPartner:
        info = tr("Delete <b>%1</b> and all its entries."
                  "<br><br>"
                  "<span style='color:#d32f2f; font-weight:bold;'>"
                  "⚠️ This action is permanent and cannot be undone."
                  "</span>")
                   .arg(path_);
        key = WsKey::kPartnerLeafDelete;
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
                   .arg(path_);
        key = WsKey::kLeafDelete;
        break;
    }

    auto* dlg { new ExactMatchConfirmDialog(info, tr("Delete"), this) };
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    connect(dlg, &ExactMatchConfirmDialog::accepted, this, [this, key]() {
        if (!node_ || !node_->IsValid())
            return;

        const QJsonObject value { JsonGen::LeafDelete(section_, node_id_, node_->version) };
        WebSocket::Instance()->SendMessage(key, value);
        close();
    });

    dlg->open();
}

void LeafDeleteDialog::IniData()
{
    ui->label->setWordWrap(true);
    ui->pBtnCancel->setDefault(true);

    InitCheckBoxGroup();

    this->setWindowTitle(tr("Delete %1").arg(path_));

    if (sale_ || purchase_ || partner_ || settlement_) {
        ui->chkBoxDelete->setEnabled(false);
        ui->label->setText(tr("The node has external references, so it can’t be deleted."));
    }
}

void LeafDeleteDialog::on_chkBoxDelete_toggled(bool checked) { ui->pBtnOk->setEnabled(checked); }
