#include "removeleafnodedialog.h"

#include <QMessageBox>

#include "component/signalblocker.h"
#include "global/websocket.h"
#include "tree/excludeonefiltermodel.h"
#include "ui_removeleafnodedialog.h"
#include "utils/jsongen.h"

RemoveLeafNodeDialog::RemoveLeafNodeDialog(CTreeModel* model, CSectionInfo& info, const QUuid& node_id, int unit, bool exteral_reference, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::RemoveLeafNodeDialog)
    , node_id_ { node_id }
    , node_unit_ { unit }
    , external_reference_ { exteral_reference }
    , model_ { model }
    , info_ { info }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniData(info.section, exteral_reference);
    IniOptionGroup();
    IniConnect();
}

RemoveLeafNodeDialog::~RemoveLeafNodeDialog() { delete ui; }

void RemoveLeafNodeDialog::IniConnect()
{
    connect(&WebSocket::Instance(), &WebSocket::SReplaceResult, this, &RemoveLeafNodeDialog::RReplaceResult);
    connect(option_group_, &QButtonGroup::idClicked, this, &RemoveLeafNodeDialog::RButtonGroup);
    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &RemoveLeafNodeDialog::RcomboBoxCurrentIndexChanged);
}

void RemoveLeafNodeDialog::IniOptionGroup()
{
    option_group_ = new QButtonGroup(this);
    option_group_->addButton(ui->rBtnRemove, 0);
    option_group_->addButton(ui->rBtnReplace, 1);
}

void RemoveLeafNodeDialog::on_pBtnOk_clicked()
{
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

            const auto message { JsonGen::RemoveLeafNode(info_.section_str, node_id_) };
            WebSocket::Instance().SendMessage(kLeafRemove, message);

            accept();
        }

        if (ui->rBtnReplace->isChecked()) {
            const auto new_node_id { ui->comboBox->currentData().toUuid() };

            const auto message { JsonGen::ReplaceLeafNode(info_.section_str, node_id_, new_node_id, external_reference_) };
            WebSocket::Instance().SendMessage(kLeafReplace, message);
        }
    }
}

void RemoveLeafNodeDialog::IniData(Section section, bool exteral_reference)
{
    ui->label->setWordWrap(true);
    ui->pBtnCancel->setDefault(true);
    this->setWindowTitle(tr("Remove %1").arg(model_->Path(node_id_)));

    if (section == Section::kSale || section == Section::kPurchase || section == Section::kStakeholder) {
        ui->rBtnReplace->setEnabled(false);
        return;
    }

    if (exteral_reference) {
        ui->rBtnRemove->setEnabled(false);
        ui->label->setText(tr("The node has external references, so it can’t be removed."));
    }

    auto* filter_model { new ExcludeOneFilterModel(node_id_, this) };
    filter_model->setSourceModel(model_->LeafModel());
    ui->comboBox->setModel(filter_model);

    ui->rBtnReplace->setChecked(true);
    RcomboBoxCurrentIndexChanged(0);
}

void RemoveLeafNodeDialog::RcomboBoxCurrentIndexChanged(int /*index*/)
{
    const auto new_node_id { ui->comboBox->currentData().toUuid() };
    ui->pBtnOk->setEnabled(!new_node_id.isNull() && model_->Unit(new_node_id) == node_unit_);
}

void RemoveLeafNodeDialog::RButtonGroup(int id)
{
    switch (id) {
    case 0:
        ui->pBtnOk->setEnabled(true);
        break;
    case 1:
        RcomboBoxCurrentIndexChanged(0);
        break;
    default:
        break;
    }
}

void RemoveLeafNodeDialog::RReplaceResult(bool result)
{
    if (result) {
        this->accept();
    } else
        QMessageBox::critical(this, tr("Replacement Conflict"),
            tr("A reference conflict exists between the related nodes of the old node and the new node. The replacement operation has been canceled."));
}
