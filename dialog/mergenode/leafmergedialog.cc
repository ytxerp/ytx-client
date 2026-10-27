#include "leafmergedialog.h"

#include "component/signalblocker.h"
#include "dialog/deletenode/exactmatchconfirmdialog.h"
#include "ui_leafmergedialog.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

LeafMergeDialog::LeafMergeDialog(TreeModel* tree_model, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LeafMergeDialog)
    , tree_model_ { tree_model }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    InitData();
}

LeafMergeDialog::~LeafMergeDialog() { delete ui; }

void LeafMergeDialog::on_comboBoxSource_currentIndexChanged(int /*index*/) { UpdateState(); }

void LeafMergeDialog::on_comboBoxTarget_currentIndexChanged(int /*index*/) { UpdateState(); }

void LeafMergeDialog::InitData()
{
    auto* item_model { tree_model_->LeafModel() };

    ui->comboBoxSource->setModel(item_model);
    ui->comboBoxTarget->setModel(item_model);

    ui->comboBoxSource->setCurrentIndex(-1);
    ui->comboBoxTarget->setCurrentIndex(-1);
}

void LeafMergeDialog::UpdateState()
{
    const int source_index { ui->comboBoxSource->currentIndex() };
    const int target_index { ui->comboBoxTarget->currentIndex() };

    if (source_index < 0 || target_index < 0 || source_index == target_index) {
        ui->pBtnOk->setEnabled(false);
        return;
    }

    const QUuid source_id { ui->comboBoxSource->currentData(Qt::UserRole).toUuid() };
    const QUuid target_id { ui->comboBoxTarget->currentData(Qt::UserRole).toUuid() };

    const Node* source { tree_model_->GetNode(source_id) };
    const Node* target { tree_model_->GetNode(target_id) };

    const bool valid { source && target && source->kind == NodeKind::kLeaf && target->kind == NodeKind::kLeaf && source->unit == target->unit
        && source->direction_rule == target->direction_rule };

    ui->pBtnOk->setEnabled(valid);
}

void LeafMergeDialog::on_pBtnOk_clicked()
{
    const QUuid source_id { ui->comboBoxSource->currentData(Qt::UserRole).toUuid() };
    const QUuid target_id { ui->comboBoxTarget->currentData(Qt::UserRole).toUuid() };

    const QString source_path { tree_model_->Path(source_id) };
    const QString target_path { tree_model_->Path(target_id) };

    const QString info { tr("Merge <b>%1</b> into <b>%2</b>."
                            "<br><br>"
                            "<span style='color:#d32f2f; font-weight:bold;'>"
                            "The source node will be permanently deleted."
                            "</span>")
            .arg(source_path.toHtmlEscaped(), target_path.toHtmlEscaped()) };

    auto* dlg { new ExactMatchConfirmDialog(info, tr("Merge"), this) };
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    connect(dlg, &ExactMatchConfirmDialog::accepted, this, [source_id, target_id, this]() {
        const Node* source { tree_model_->GetNode(source_id) };
        const Node* target { tree_model_->GetNode(target_id) };

        if (!source || !source->IsValid() || !target || !target->IsValid())
            return;

        const auto message { JsonGen::LeafMerge(Section::kInventory, source_id, target_id, target->version) };
        WebSocket::Instance()->SendMessage(WsKey::kLeafMerge, message);
    });

    dlg->open();
}
