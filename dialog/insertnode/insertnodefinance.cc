#include "insertnodefinance.h"

#include "component/signalblocker.h"
#include "enum/nodeenum.h"
#include "ui_insertnodefinance.h"

InsertNodeFinance::InsertNodeFinance(CNodeInsertArg& arg, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::InsertNodeFinance)
    , node_ { static_cast<NodeF*>(arg.node) }
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

InsertNodeFinance::~InsertNodeFinance() { delete ui; }

void InsertNodeFinance::IniDialog(ItemModel* unit_model)
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

void InsertNodeFinance::IniData(Node* node)
{
    int unit_index { ui->comboUnit->findData(std::to_underlying(node->unit)) };
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
    kind_group_->addButton(ui->rBtnBranch, std::to_underlying(NodeKind::kBranch));
    kind_group_->addButton(ui->rBtnLeaf, std::to_underlying(NodeKind::kLeaf));
}

void InsertNodeFinance::IniRuleGroup()
{
    rule_group_ = new QButtonGroup(this);
    rule_group_->addButton(ui->rBtnDDCI, static_cast<int>(Rule::kDDCI));
    rule_group_->addButton(ui->rBtnDICD, static_cast<int>(Rule::kDICD));
}

void InsertNodeFinance::IniDirectionRule(bool rule) { (rule ? ui->rBtnDDCI : ui->rBtnDICD)->setChecked(true); }

void InsertNodeFinance::RNameEdited(const QString& arg1)
{
    const auto& simplified { arg1.simplified() };
    this->setWindowTitle(parent_path_ + simplified);
    ui->pBtnOk->setEnabled(!simplified.isEmpty() && !name_set_.contains(simplified));
}

void InsertNodeFinance::on_lineName_editingFinished() { node_->name = ui->lineName->text(); }

void InsertNodeFinance::on_lineCode_editingFinished() { node_->code = ui->lineCode->text(); }

void InsertNodeFinance::on_lineDescription_editingFinished() { node_->description = ui->lineDescription->text(); }

void InsertNodeFinance::on_comboUnit_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    node_->unit = NodeUnit(ui->comboUnit->currentData().toInt());
}

void InsertNodeFinance::RDirectionRuleGroupClicked(int id) { node_->direction_rule = static_cast<bool>(id); }

void InsertNodeFinance::RKindGroupClicked(int id) { node_->kind = NodeKind(id); }
