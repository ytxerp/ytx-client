#include "insertnodestakeholder.h"

#include "component/enumclass.h"
#include "component/signalblocker.h"
#include "ui_insertnodestakeholder.h"

InsertNodeStakeholder::InsertNodeStakeholder(CInsertNodeArgFIST& arg, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::InsertNodeStakeholder)
    , node_ { static_cast<NodeS*>(arg.node) }
    , parent_path_ { arg.parent_path }
    , name_list_ { arg.name_list }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniDialog(arg.unit_model);
    IniKindGroup();
    IniData();
    IniConnect();
}

InsertNodeStakeholder::~InsertNodeStakeholder() { delete ui; }

void InsertNodeStakeholder::IniDialog(ItemModel* unit_model)
{
    ui->lineEditName->setFocus();
    ui->lineEditName->setValidator(&LineEdit::kInputValidator);

    this->setWindowTitle(parent_path_);
    this->setFixedSize(300, 500);

    ui->comboUnit->setModel(unit_model);
    ui->spinPaymentPeriod->setRange(0, std::numeric_limits<int>::max());
}

void InsertNodeStakeholder::IniConnect()
{
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &InsertNodeStakeholder::RNameEdited);
    connect(kind_group_, &QButtonGroup::idClicked, this, &InsertNodeStakeholder::RKindGroupClicked);
}

void InsertNodeStakeholder::IniData()
{
    int unit_index { ui->comboUnit->findData(node_->unit) };
    ui->comboUnit->setCurrentIndex(unit_index);

    ui->rBtnLeaf->setChecked(true);
    ui->pBtnOk->setEnabled(false);
}

void InsertNodeStakeholder::IniKindGroup()
{
    kind_group_ = new QButtonGroup(this);
    kind_group_->addButton(ui->rBtnBranch, kBranch);
    kind_group_->addButton(ui->rBtnLeaf, kLeaf);
}

void InsertNodeStakeholder::RNameEdited(const QString& arg1)
{
    const auto& simplified { arg1.simplified() };
    this->setWindowTitle(parent_path_ + simplified);
    ui->pBtnOk->setEnabled(!simplified.isEmpty() && !name_list_.contains(simplified));
}

void InsertNodeStakeholder::on_lineEditName_editingFinished() { node_->name = ui->lineEditName->text(); }

void InsertNodeStakeholder::on_lineEditCode_editingFinished() { node_->code = ui->lineEditCode->text(); }

void InsertNodeStakeholder::on_lineEditDescription_editingFinished() { node_->description = ui->lineEditDescription->text(); }

void InsertNodeStakeholder::on_plainTextEdit_textChanged() { node_->note = ui->plainTextEdit->toPlainText(); }

void InsertNodeStakeholder::on_comboUnit_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    node_->unit = ui->comboUnit->currentData().toInt();
}

void InsertNodeStakeholder::on_spinPaymentPeriod_editingFinished() { node_->payment_term = ui->spinPaymentPeriod->value(); }

void InsertNodeStakeholder::RKindGroupClicked(int id) { node_->kind = id; }
