#include "insertnodetask.h"

#include <QColorDialog>

#include "component/signalblocker.h"
#include "enum/nodeenum.h"
#include "ui_insertnodetask.h"

InsertNodeTask::InsertNodeTask(CNodeInsertArg& arg, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::InsertNodeTask)
    , node_ { static_cast<NodeT*>(arg.node) }
    , parent_path_ { arg.parent_path }
    , name_list_ { arg.name_list }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniDialog(arg.unit_model);
    IniData(arg.node);
    IniRuleGroup();
    IniKindGroup();
    IniStatusGroup();
    IniConnect();
}

InsertNodeTask::~InsertNodeTask() { delete ui; }

void InsertNodeTask::IniDialog(ItemModel* unit_model)
{
    ui->lineEditName->setFocus();
    ui->lineEditName->setValidator(&LineEdit::kInputValidator);

    this->setWindowTitle(parent_path_);
    this->setFixedSize(360, 600);

    ui->comboUnit->setModel(unit_model);
    ui->issuedTime->setDisplayFormat(kDateFST);
    ui->issuedTime->setDateTime(node_->issued_time.toLocalTime());

    ui->rBtnBranch->setShortcut(QKeySequence(Qt::ALT | Qt::Key_B));
    ui->rBtnLeaf->setShortcut(QKeySequence(Qt::ALT | Qt::Key_L));
    ui->rBtnDICD->setShortcut(QKeySequence(Qt::ALT | Qt::Key_D));
    ui->rBtnDDCI->setShortcut(QKeySequence(Qt::ALT | Qt::Key_C));
    ui->rBtnRecalled->setShortcut(QKeySequence(Qt::ALT | Qt::Key_E));
    ui->rBtnReleased->setShortcut(QKeySequence(Qt::ALT | Qt::Key_R));
}

void InsertNodeTask::IniData(Node* node)
{
    int item_index { ui->comboUnit->findData(node->unit) };
    ui->comboUnit->setCurrentIndex(item_index);

    IniRule(node->direction_rule);

    ui->rBtnLeaf->setChecked(true);
    ui->rBtnRecalled->setChecked(true);
    ui->pBtnOk->setEnabled(false);
}

void InsertNodeTask::IniConnect()
{
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &InsertNodeTask::RNameEdited);
    connect(rule_group_, &QButtonGroup::idClicked, this, &InsertNodeTask::RRuleGroupClicked);
    connect(kind_group_, &QButtonGroup::idClicked, this, &InsertNodeTask::RKindGroupClicked);
    connect(status_group_, &QButtonGroup::idClicked, this, &InsertNodeTask::RStatusGroupClicked);
}

void InsertNodeTask::UpdateColor(QColor color)
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

void InsertNodeTask::IniStatusGroup()
{
    status_group_ = new QButtonGroup(this);
    status_group_->addButton(ui->rBtnRecalled, std::to_underlying(NodeStatus::kRecalled));
    status_group_->addButton(ui->rBtnReleased, std::to_underlying(NodeStatus::kReleased));
}

void InsertNodeTask::IniRule(bool rule) { (rule ? ui->rBtnDDCI : ui->rBtnDICD)->setChecked(true); }

void InsertNodeTask::RNameEdited(const QString& arg1)
{
    const auto& simplified { arg1.simplified() };
    this->setWindowTitle(parent_path_ + simplified);
    ui->pBtnOk->setEnabled(!simplified.isEmpty() && !name_list_.contains(simplified));
}

void InsertNodeTask::on_lineEditName_editingFinished() { node_->name = ui->lineEditName->text(); }

void InsertNodeTask::on_lineEditCode_editingFinished() { node_->code = ui->lineEditCode->text(); }

void InsertNodeTask::on_lineEditDescription_editingFinished() { node_->description = ui->lineEditDescription->text(); }

void InsertNodeTask::on_comboUnit_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    node_->unit = ui->comboUnit->currentData().toInt();
}

void InsertNodeTask::on_plainTextEdit_textChanged() { node_->note = ui->plainTextEdit->toPlainText(); }

void InsertNodeTask::on_pBtnColor_clicked()
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

void InsertNodeTask::RRuleGroupClicked(int id) { node_->direction_rule = static_cast<bool>(id); }

void InsertNodeTask::RKindGroupClicked(int id) { node_->kind = id; }

void InsertNodeTask::RStatusGroupClicked(int id) { node_->status = NodeStatus(id); }

void InsertNodeTask::on_issuedTime_dateTimeChanged(const QDateTime& dateTime) { node_->issued_time = dateTime.toUTC(); }
