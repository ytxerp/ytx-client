#include "insertnodefit.h"

#include "component/constantbool.h"
#include "component/signalblocker.h"
#include "enum/nodeenum.h"
#include "ui_insertnodefit.h"

InsertNodeFIT::InsertNodeFIT(CNodeInsertArg& arg, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::InsertNodeFIT)
    , node_ { arg.node }
    , parent_path_ { arg.parent_path }
    , name_set_ { arg.name_set }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniDialog(arg.unit_model);
    IniData(arg.node);
    IniRuleGroup();
    IniKindGroup();
    IniConnect();
}

InsertNodeFIT::~InsertNodeFIT() { delete ui; }

void InsertNodeFIT::IniDialog(UnitModel* unit_model)
{
    ui->lineName->setFocus();
    ui->lineName->setValidator(&LineEdit::kInputValidator);

    this->setWindowTitle(parent_path_);
    this->setMinimumSize(270, 360);

    ui->comboUnit->setModel(unit_model);

    ui->rBtnBranch->setShortcut(QKeySequence(Qt::ALT | Qt::Key_B));
    ui->rBtnLeaf->setShortcut(QKeySequence(Qt::ALT | Qt::Key_L));
    ui->rBtnDICD->setShortcut(QKeySequence(Qt::ALT | Qt::Key_D));
    ui->rBtnDDCI->setShortcut(QKeySequence(Qt::ALT | Qt::Key_C));
}

void InsertNodeFIT::IniData(Node* node)
{
    int unit_index { ui->comboUnit->findData(std::to_underlying(node->unit)) };
    ui->comboUnit->setCurrentIndex(unit_index);

    IniDirectionRule(node->direction_rule);

    ui->rBtnLeaf->setChecked(true);
    ui->pBtnOk->setEnabled(false);
}

void InsertNodeFIT::IniConnect()
{
    connect(ui->lineName, &QLineEdit::textEdited, this, &InsertNodeFIT::RNameEdited);
    connect(rule_group_, &QButtonGroup::idClicked, this, &InsertNodeFIT::RDirectionRuleGroupClicked);
    connect(kind_group_, &QButtonGroup::idClicked, this, &InsertNodeFIT::RKindGroupClicked);
}

void InsertNodeFIT::IniKindGroup()
{
    kind_group_ = new QButtonGroup(this);
    kind_group_->addButton(ui->rBtnBranch, std::to_underlying(NodeKind::kBranch));
    kind_group_->addButton(ui->rBtnLeaf, std::to_underlying(NodeKind::kLeaf));
}

void InsertNodeFIT::IniRuleGroup()
{
    rule_group_ = new QButtonGroup(this);
    rule_group_->addButton(ui->rBtnDDCI, static_cast<int>(direction_rule::kDDCI));
    rule_group_->addButton(ui->rBtnDICD, static_cast<int>(direction_rule::kDICD));
}

void InsertNodeFIT::IniDirectionRule(bool rule) { (rule ? ui->rBtnDDCI : ui->rBtnDICD)->setChecked(true); }

void InsertNodeFIT::RNameEdited(const QString& arg1)
{
    const auto& simplified { arg1.simplified() };
    this->setWindowTitle(parent_path_ + simplified);
    ui->pBtnOk->setEnabled(!simplified.isEmpty() && !name_set_.contains(simplified));
}

void InsertNodeFIT::on_lineName_editingFinished() { node_->name = ui->lineName->text(); }

void InsertNodeFIT::on_lineCode_editingFinished() { node_->code = ui->lineCode->text(); }

void InsertNodeFIT::on_lineDescription_editingFinished() { node_->description = ui->lineDescription->text(); }

void InsertNodeFIT::on_comboUnit_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    node_->unit = NodeUnit(ui->comboUnit->currentData().toInt());
}

void InsertNodeFIT::RDirectionRuleGroupClicked(int id) { node_->direction_rule = static_cast<bool>(id); }

void InsertNodeFIT::RKindGroupClicked(int id) { node_->kind = NodeKind(id); }
