#include "insertnodetask.h"

#include "component/signalblocker.h"
#include "enum/nodeenum.h"
#include "ui_insertnodetask.h"

InsertNodeTask::InsertNodeTask(CNodeInsertArg& arg, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::InsertNodeTask)
    , node_ { static_cast<NodeT*>(arg.node) }
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

InsertNodeTask::~InsertNodeTask() { delete ui; }

void InsertNodeTask::IniDialog(ItemModel* unit_model)
{
    ui->lineEditName->setFocus();
    ui->lineEditName->setValidator(&LineEdit::kInputValidator);

    this->setWindowTitle(parent_path_);
    this->setMinimumSize(270, 360);

    ui->comboUnit->setModel(unit_model);

    ui->rBtnBranch->setShortcut(QKeySequence(Qt::ALT | Qt::Key_B));
    ui->rBtnLeaf->setShortcut(QKeySequence(Qt::ALT | Qt::Key_L));
    ui->rBtnDICD->setShortcut(QKeySequence(Qt::ALT | Qt::Key_D));
    ui->rBtnDDCI->setShortcut(QKeySequence(Qt::ALT | Qt::Key_C));
}

void InsertNodeTask::IniData(Node* node)
{
    int item_index { ui->comboUnit->findData(std::to_underlying(node->unit)) };
    ui->comboUnit->setCurrentIndex(item_index);

    IniRule(node->direction_rule);

    ui->rBtnLeaf->setChecked(true);
    ui->pBtnOk->setEnabled(false);
}

void InsertNodeTask::IniConnect()
{
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &InsertNodeTask::RNameEdited);
    connect(rule_group_, &QButtonGroup::idClicked, this, &InsertNodeTask::RRuleGroupClicked);
    connect(kind_group_, &QButtonGroup::idClicked, this, &InsertNodeTask::RKindGroupClicked);
}

void InsertNodeTask::IniKindGroup()
{
    kind_group_ = new QButtonGroup(this);
    kind_group_->addButton(ui->rBtnBranch, std::to_underlying(NodeKind::kBranch));
    kind_group_->addButton(ui->rBtnLeaf, std::to_underlying(NodeKind::kLeaf));
}

void InsertNodeTask::IniRuleGroup()
{
    rule_group_ = new QButtonGroup(this);
    rule_group_->addButton(ui->rBtnDDCI, static_cast<int>(Rule::kDDCI));
    rule_group_->addButton(ui->rBtnDICD, static_cast<int>(Rule::kDICD));
}

void InsertNodeTask::IniRule(bool rule) { (rule ? ui->rBtnDDCI : ui->rBtnDICD)->setChecked(true); }

void InsertNodeTask::RNameEdited(const QString& arg1)
{
    const auto simplified { arg1.simplified() };
    this->setWindowTitle(parent_path_ + simplified);
    ui->pBtnOk->setEnabled(!simplified.isEmpty() && !name_set_.contains(simplified));
}

void InsertNodeTask::on_lineEditName_editingFinished() { node_->name = ui->lineEditName->text(); }

void InsertNodeTask::on_lineEditCode_editingFinished() { node_->code = ui->lineEditCode->text(); }

void InsertNodeTask::on_lineEditDescription_editingFinished() { node_->description = ui->lineEditDescription->text(); }

void InsertNodeTask::on_comboUnit_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    node_->unit = NodeUnit(ui->comboUnit->currentData().toInt());
}

void InsertNodeTask::RRuleGroupClicked(int id) { node_->direction_rule = static_cast<bool>(id); }

void InsertNodeTask::RKindGroupClicked(int id) { node_->kind = NodeKind(id); }
