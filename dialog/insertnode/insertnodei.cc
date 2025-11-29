#include "insertnodei.h"

#include <QColorDialog>

#include "component/signalblocker.h"
#include "enum/nodeenum.h"
#include "ui_insertnodei.h"

InsertNodeI::InsertNodeI(CNodeInsertArg& arg, int rate_decimal, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::InsertNodeI)
    , node_ { static_cast<NodeI*>(arg.node) }
    , parent_path_ { arg.parent_path }
    , name_list_ { arg.name_list }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniDialog(arg.unit_model, rate_decimal);
    IniRuleGroup();
    IniKindGroup();
    IniData(arg.node);
    IniConnect();
}

InsertNodeI::~InsertNodeI() { delete ui; }

void InsertNodeI::IniDialog(ItemModel* unit_model, int amount_decimal)
{
    ui->lineEditName->setFocus();
    ui->lineEditName->setValidator(&LineEdit::kInputValidator);

    this->setWindowTitle(parent_path_);
    this->setFixedSize(360, 600);

    ui->comboUnit->setModel(unit_model);
    ui->dSpinBoxUnitPrice->setRange(0.0, kDoubleMax);
    ui->dSpinBoxCommission->setRange(0.0, kDoubleMax);
    ui->dSpinBoxUnitPrice->setDecimals(amount_decimal);
    ui->dSpinBoxCommission->setDecimals(amount_decimal);

    ui->rBtnBranch->setShortcut(QKeySequence(Qt::ALT | Qt::Key_B));
    ui->rBtnLeaf->setShortcut(QKeySequence(Qt::ALT | Qt::Key_L));
    ui->rBtnDICD->setShortcut(QKeySequence(Qt::ALT | Qt::Key_D));
    ui->rBtnDDCI->setShortcut(QKeySequence(Qt::ALT | Qt::Key_C));
}

void InsertNodeI::IniConnect()
{
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &InsertNodeI::RNameEdited);
    connect(rule_group_, &QButtonGroup::idClicked, this, &InsertNodeI::RRuleGroupClicked);
    connect(kind_group_, &QButtonGroup::idClicked, this, &InsertNodeI::RKindGroupClicked);
}

void InsertNodeI::IniData(Node* node)
{
    int unit_index { ui->comboUnit->findData(node->unit) };
    ui->comboUnit->setCurrentIndex(unit_index);

    IniDirectionRule(node->direction_rule);
    ui->rBtnLeaf->setChecked(true);

    ui->pBtnOk->setEnabled(false);
}

void InsertNodeI::UpdateColor(QColor color)
{
    if (color.isValid()) {
        ui->pBtnColor->setStyleSheet(QString(R"(
        background-color: %1;
        border: 1px solid %1;
        border-radius: 4px;
        )")
                .arg(node_->color)

        );

        ui->pBtnColor->setText(QString());
    }
}

void InsertNodeI::IniKindGroup()
{
    kind_group_ = new QButtonGroup(this);
    kind_group_->addButton(ui->rBtnBranch, std::to_underlying(NodeKind::kBranch));
    kind_group_->addButton(ui->rBtnLeaf, std::to_underlying(NodeKind::kLeaf));
}

void InsertNodeI::IniRuleGroup()
{
    rule_group_ = new QButtonGroup(this);
    rule_group_->addButton(ui->rBtnDDCI, static_cast<int>(Rule::kDDCI));
    rule_group_->addButton(ui->rBtnDICD, static_cast<int>(Rule::kDICD));
}

void InsertNodeI::IniDirectionRule(bool rule) { (rule ? ui->rBtnDDCI : ui->rBtnDICD)->setChecked(true); }

void InsertNodeI::RNameEdited(const QString& arg1)
{
    const auto& simplified { arg1.simplified() };
    this->setWindowTitle(parent_path_ + simplified);
    ui->pBtnOk->setEnabled(!simplified.isEmpty() && !name_list_.contains(simplified));
}

void InsertNodeI::on_lineEditName_editingFinished() { node_->name = ui->lineEditName->text(); }

void InsertNodeI::on_lineEditCode_editingFinished() { node_->code = ui->lineEditCode->text(); }

void InsertNodeI::on_lineEditDescription_editingFinished() { node_->description = ui->lineEditDescription->text(); }

void InsertNodeI::on_comboUnit_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    node_->unit = ui->comboUnit->currentData().toInt();
}

void InsertNodeI::on_plainTextEdit_textChanged() { node_->note = ui->plainTextEdit->toPlainText(); }

void InsertNodeI::on_dSpinBoxUnitPrice_editingFinished() { node_->unit_price = ui->dSpinBoxUnitPrice->value(); }

void InsertNodeI::on_dSpinBoxCommission_editingFinished() { node_->commission = ui->dSpinBoxCommission->value(); }

void InsertNodeI::on_pBtnColor_clicked()
{
    QColor color(node_->color);
    if (!color.isValid())
        color = Qt::white;

    QColor selected_color { QColorDialog::getColor(color, nullptr, tr("Choose Color"), QColorDialog::ShowAlphaChannel) };
    if (selected_color.isValid()) {
        node_->color = selected_color.name(QColor::HexArgb);
        UpdateColor(selected_color);
    }
}

void InsertNodeI::RRuleGroupClicked(int id) { node_->direction_rule = static_cast<bool>(id); }

void InsertNodeI::RKindGroupClicked(int id) { node_->kind = id; }
