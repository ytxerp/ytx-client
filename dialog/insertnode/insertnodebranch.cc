#include "insertnodebranch.h"

#include "component/signalblocker.h"
#include "ui_insertnodebranch.h"

InsertNodeBranch::InsertNodeBranch(Node* node, ItemModel* unit_model, CString& parent_path, const QSet<QString>& name_set, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::InsertNodeBranch)
    , node_ { node }
    , parent_path_ { parent_path }
    , name_set_ { name_set }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniDialog(unit_model);
    IniData(node);
    IniConnect();
}

InsertNodeBranch::~InsertNodeBranch() { delete ui; }

void InsertNodeBranch::IniDialog(ItemModel* unit_model)
{
    ui->lineName->setFocus();
    ui->lineName->setValidator(&LineEdit::kInputValidator);

    this->setWindowTitle(parent_path_);
    ui->comboUnit->setModel(unit_model);
}

void InsertNodeBranch::IniData(Node* node)
{
    int unit_index { ui->comboUnit->findData(std::to_underlying(node->unit)) };
    ui->comboUnit->setCurrentIndex(unit_index);

    ui->pBtnOk->setEnabled(false);
}

void InsertNodeBranch::IniConnect() { connect(ui->lineName, &QLineEdit::textEdited, this, &InsertNodeBranch::RNameEdited); }

void InsertNodeBranch::RNameEdited(const QString& arg1)
{
    const auto& simplified { arg1.simplified() };
    this->setWindowTitle(parent_path_ + simplified);
    ui->pBtnOk->setEnabled(!simplified.isEmpty() && !name_set_.contains(simplified));
}

void InsertNodeBranch::on_lineName_editingFinished() { node_->name = ui->lineName->text(); }

void InsertNodeBranch::on_lineDescription_editingFinished() { node_->description = ui->lineDescription->text(); }

void InsertNodeBranch::on_comboUnit_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    node_->unit = NodeUnit(ui->comboUnit->currentData().toInt());
}
