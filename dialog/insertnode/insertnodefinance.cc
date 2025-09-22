#include "insertnodefinance.h"

#include "component/enumclass.h"
#include "component/signalblocker.h"
#include "ui_insertnodefinance.h"

InsertNodeFinance::InsertNodeFinance(CInsertNodeArgFIST& arg, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::InsertNodeFinance)
    , node_ { static_cast<NodeF*>(arg.node) }
    , parent_path_ { arg.parent_path }
    , name_list_ { arg.name_list }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniDialog(arg.unit_model);
    IniData(arg.node);
    IniRuleGroup();
    IniKindGroup();
    IniConnect();
}

InsertNodeFinance::~InsertNodeFinance() { delete ui; }

void InsertNodeFinance::IniDialog(ItemModel* unit_model)
{
    ui->lineName->setFocus();
    ui->lineName->setValidator(&LineEdit::kInputValidator);

    this->setWindowTitle(parent_path_);
    this->setFixedSize(300, 500);

    ui->comboUnit->setModel(unit_model);
}

void InsertNodeFinance::IniData(Node* node)
{
    int unit_index { ui->comboUnit->findData(node->unit) };
    ui->comboUnit->setCurrentIndex(unit_index);

    IniDirectionRule(node->direction_rule);
    ui->rBtnLeaf->setChecked(true);

    ui->pBtnOk->setEnabled(false);
}

void InsertNodeFinance::IniConnect()
{
    connect(ui->lineName, &QLineEdit::textEdited, this, &InsertNodeFinance::RNameEdited);
    connect(rule_group_, &QButtonGroup::idClicked, this, &InsertNodeFinance::RDirectionRuleGroupClicked);
    connect(kind_group_, &QButtonGroup::idClicked, this, &InsertNodeFinance::RKindGroupClicked);
}

void InsertNodeFinance::IniKindGroup()
{
    kind_group_ = new QButtonGroup(this);
    kind_group_->addButton(ui->rBtnBranch, kBranch);
    kind_group_->addButton(ui->rBtnLeaf, kLeaf);
}

void InsertNodeFinance::IniRuleGroup()
{
    rule_group_ = new QButtonGroup(this);
    rule_group_->addButton(ui->rBtnDDCI, static_cast<int>(kDDCI));
    rule_group_->addButton(ui->rBtnDICD, static_cast<int>(kDICD));
}

void InsertNodeFinance::IniDirectionRule(bool rule) { (rule ? ui->rBtnDDCI : ui->rBtnDICD)->setChecked(true); }

void InsertNodeFinance::RNameEdited(const QString& arg1)
{
    const auto& simplified { arg1.simplified() };
    this->setWindowTitle(parent_path_ + simplified);
    ui->pBtnOk->setEnabled(!simplified.isEmpty() && !name_list_.contains(simplified));
}

void InsertNodeFinance::on_lineName_editingFinished() { node_->name = ui->lineName->text(); }

void InsertNodeFinance::on_lineCode_editingFinished() { node_->code = ui->lineCode->text(); }

void InsertNodeFinance::on_lineDescription_editingFinished() { node_->description = ui->lineDescription->text(); }

void InsertNodeFinance::on_comboUnit_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    node_->unit = ui->comboUnit->currentData().toInt();
}

void InsertNodeFinance::RDirectionRuleGroupClicked(int id) { node_->direction_rule = static_cast<bool>(id); }

void InsertNodeFinance::RKindGroupClicked(int id) { node_->kind = id; }

void InsertNodeFinance::on_plainNote_textChanged() { node_->note = ui->plainNote->toPlainText(); }
