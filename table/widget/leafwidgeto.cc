#include "leafwidgeto.h"

#include "component/signalblocker.h"
#include "global/nodepool.h"
#include "ui_leafwidgeto.h"

LeafWidgetO::LeafWidgetO(
    CInsertNodeArgO& arg, bool is_insert, const QMap<QString, QString>& print_template, QSharedPointer<PrintManager> print_manager, QWidget* parent)
    : LeafWidget(parent)
    , ui(new Ui::LeafWidgetO)
    , node_ { static_cast<NodeO*>(arg.node) }
    , sql_ { qobject_cast<EntryHubO*>(arg.dbhub) }
    , leaf_model_order_ { qobject_cast<LeafModelO*>(arg.leaf_model) }
    , tree_model_stakeholder_ { arg.tree_model_stakeholder }
    , config_ { arg.section_config }
    , is_insert_ { is_insert }
    , node_id_ { arg.node->id }
    , party_unit_ { arg.section == Section::kSale ? std::to_underlying(UnitS::kCustomer) : std::to_underlying(UnitS::kVendor) }
    , print_template_ { print_template }
    , print_manager_ { print_manager }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniWidget();
    IniText(arg.section);
    IniUnit(arg.node->unit);
    IniRuleGroup();
    IniUnitGroup();
    IniRule(arg.node->direction_rule);
    IniData(node_->party, node_->employee);
    IniConnect();

    const bool finished { node_->is_finished };
    IniFinished(finished);
    LockWidgets(finished);
}

LeafWidgetO::~LeafWidgetO()
{
    if (is_insert_) {
        NodePool::Instance().Recycle(node_, Section::kSale);
    }

    delete leaf_model_order_;
    delete ui;
}

QTableView* LeafWidgetO::View() const { return ui->tableViewO; }

void LeafWidgetO::RSyncDelta(const QUuid& node_id, double initial_delta, double final_delta, double first_delta, double second_delta, double discount_delta)
{
    if (node_id_ != node_id)
        return;

    const double adjusted_final_delta { node_->unit == std::to_underlying(UnitO::kImmediate) ? final_delta : 0.0 };

    node_->first_total += first_delta;
    node_->second_total += second_delta;
    node_->initial_total += initial_delta;
    node_->discount_total += discount_delta;
    node_->final_total += adjusted_final_delta;

    IniLeafValue();
}

void LeafWidgetO::IniWidget()
{
    pmodel_ = tree_model_stakeholder_->IncludeUnitModel(party_unit_);
    ui->comboParty->setModel(pmodel_);
    ui->comboParty->setCurrentIndex(-1);

    emodel_ = tree_model_stakeholder_->IncludeUnitModel(std::to_underlying(UnitS::kEmployee));
    ui->comboEmployee->setModel(emodel_);
    ui->comboEmployee->setCurrentIndex(-1);

    ui->tableViewO->setModel(leaf_model_order_);
    ui->dateTimeEdit->setDisplayFormat(kDateTimeFST);

    ui->dSpinDiscountTotal->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    ui->dSpinInitialTotal->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    ui->dSpinFinalTotal->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    ui->dSpinSecondTotal->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    ui->dSpinFirstTotal->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

    ui->dSpinDiscountTotal->setDecimals(config_.amount_decimal);
    ui->dSpinInitialTotal->setDecimals(config_.amount_decimal);
    ui->dSpinFinalTotal->setDecimals(config_.amount_decimal);
    ui->dSpinSecondTotal->setDecimals(config_.amount_decimal);
    ui->dSpinFirstTotal->setDecimals(config_.amount_decimal);

    ui->dSpinDiscountTotal->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    ui->dSpinFinalTotal->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    ui->dSpinFirstTotal->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->dSpinSecondTotal->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    ui->tableViewO->setFocus();

    for (auto it = print_template_.constBegin(); it != print_template_.constEnd(); ++it) {
        ui->comboTemplate->addItem(it.key(), it.value());
    }
}

void LeafWidgetO::IniConnect()
{
    connect(rule_group_, &QButtonGroup::idClicked, this, &LeafWidgetO::RRuleGroupClicked);
    connect(unit_group_, &QButtonGroup::idClicked, this, &LeafWidgetO::RUnitGroupClicked);
}

void LeafWidgetO::IniData(const QUuid& party, const QUuid& employee)
{
    if (is_insert_)
        return;

    IniLeafValue();
    ui->lineDescription->setText(node_->description);
    ui->dateTimeEdit->setDateTime(node_->issued_time.toLocalTime());

    int party_index { ui->comboParty->findData(party) };
    ui->comboParty->setCurrentIndex(party_index);

    int employee_index { ui->comboEmployee->findData(employee) };
    ui->comboEmployee->setCurrentIndex(employee_index);
}

void LeafWidgetO::LockWidgets(bool finished)
{
    const bool enable { !finished };

    ui->labParty->setEnabled(enable);
    ui->comboParty->setEnabled(enable);

    ui->labelFinalTotal->setEnabled(enable);
    ui->dSpinFinalTotal->setEnabled(enable);

    ui->dSpinInitialTotal->setEnabled(enable);

    ui->labelDiscountTotal->setEnabled(enable);
    ui->dSpinDiscountTotal->setEnabled(enable);

    ui->labelEmployee->setEnabled(enable);
    ui->comboEmployee->setEnabled(enable);
    ui->tableViewO->setEnabled(enable);

    ui->rBtnIS->setEnabled(enable);
    ui->rBtnMS->setEnabled(enable);
    ui->rBtnPEND->setEnabled(enable);
    ui->dateTimeEdit->setEnabled(enable);

    ui->dSpinFirstTotal->setEnabled(enable);
    ui->labelFirstTotal->setEnabled(enable);
    ui->dSpinSecondTotal->setEnabled(enable);
    ui->labelSecondTotal->setEnabled(enable);

    ui->rBtnRO->setEnabled(enable);
    ui->rBtnTO->setEnabled(enable);
    ui->lineDescription->setEnabled(enable);

    ui->pBtnPrint->setEnabled(finished);
}

void LeafWidgetO::IniUnit(int unit)
{
    const UnitO kUnit { unit };

    switch (kUnit) {
    case UnitO::kImmediate:
        ui->rBtnIS->setChecked(true);
        break;
    case UnitO::kMonthly:
        ui->rBtnMS->setChecked(true);
        break;
    case UnitO::kPending:
        ui->rBtnPEND->setChecked(true);
        break;
    default:
        break;
    }
}

void LeafWidgetO::IniLeafValue()
{
    ui->dSpinFinalTotal->setValue(node_->final_total);
    ui->dSpinDiscountTotal->setValue(node_->discount_total);
    ui->dSpinFirstTotal->setValue(node_->first_total);
    ui->dSpinSecondTotal->setValue(node_->second_total);
    ui->dSpinInitialTotal->setValue(node_->initial_total);
}

void LeafWidgetO::IniText(Section section)
{
    const bool is_sale_section { section == Section::kSale };

    setWindowTitle(is_sale_section ? tr("Sale") : tr("Purchase"));
    ui->rBtnTO->setText(is_sale_section ? tr("SO") : tr("PO"));
    ui->labParty->setText(is_sale_section ? tr("CUST") : tr("VEND"));
}

void LeafWidgetO::IniRule(bool rule) { (rule ? ui->rBtnRO : ui->rBtnTO)->setChecked(true); }

void LeafWidgetO::IniFinished(bool finished)
{
    ui->pBtnFinishOrder->setChecked(finished);
    ui->pBtnFinishOrder->setText(finished ? tr("Edit") : tr("Finish"));
    ui->pBtnFinishOrder->setEnabled(node_->unit != std::to_underlying(UnitO::kPending));
    emit SEnableAction(finished);

    if (finished) {
        ui->pBtnPrint->setFocus();
        ui->pBtnPrint->setDefault(true);
        ui->tableViewO->clearSelection();
    }

    // 需要修复
    // if (sql_->ExternalReference(node_id_))
    //     ui->pBtnFinishOrder->setEnabled(false);
}

void LeafWidgetO::IniRuleGroup()
{
    rule_group_ = new QButtonGroup(this);
    rule_group_->addButton(ui->rBtnRO, static_cast<int>(Rule::kRO));
    rule_group_->addButton(ui->rBtnTO, static_cast<int>(Rule::kTO));
}

void LeafWidgetO::IniUnitGroup()
{
    unit_group_ = new QButtonGroup(this);
    unit_group_->addButton(ui->rBtnIS, std::to_underlying(UnitO::kImmediate));
    unit_group_->addButton(ui->rBtnMS, std::to_underlying(UnitO::kMonthly));
    unit_group_->addButton(ui->rBtnPEND, std::to_underlying(UnitO::kPending));
}

void LeafWidgetO::on_comboParty_currentIndexChanged(int /*index*/)
{
    const QUuid party_id { ui->comboParty->currentData().toUuid() };
    if (party_id.isNull())
        return;

    node_->party = party_id;
    // sql_->WriteField(party_info_, kParty, party_id.toString(QUuid::WithoutBraces), node_id_);
    emit SSyncParty(node_id_, std::to_underlying(NodeEnumO::kParty), party_id);
}

void LeafWidgetO::on_comboEmployee_currentIndexChanged(int /*index*/)
{
    node_->employee = ui->comboEmployee->currentData().toUuid();
    // sql_->WriteField(party_info_, kEmployee, node_->employee.toString(QUuid::WithoutBraces), node_id_);
}

#if 0
void TransWidgetO::on_pBtnInsert_clicked()
{
    const auto& name { ui->comboParty->currentText() };
    if (name.isEmpty() || ui->comboParty->currentIndex() != -1)
        return;

    auto* node { ResourcePool<Node>::Instance().Allocate() };
    stakeholder_node_->SetParent(node, {});
    node->name = name;

    node->unit = party_unit_;

    stakeholder_node_->InsertNode(0, QModelIndex(), node);

    int party_index { ui->comboParty->findData(node->id) };
    ui->comboParty->setCurrentIndex(party_index);
}
#endif

void LeafWidgetO::on_dateTimeEdit_dateTimeChanged(const QDateTime& date_time)
{
    node_->issued_time = date_time.toUTC();
    // sql_->WriteField(party_info_, kIssuedTime, node_->issued_time, node_id_);
}

void LeafWidgetO::on_lineDescription_editingFinished()
{
    node_->description = ui->lineDescription->text();
    // sql_->WriteField(party_info_, kDescription, node_->description, node_id_);
}

void LeafWidgetO::RRuleGroupClicked(int id)
{
    node_->direction_rule = static_cast<bool>(id);

    node_->first_total *= -1;
    node_->second_total *= -1;
    node_->initial_total *= -1;
    node_->discount_total *= -1;
    node_->final_total *= -1;

    IniLeafValue();

    // sql_->WriteField(party_info_, kDirectionRule, node_->direction_rule, node_id_);
    // sql_->UpdateLeafValue(node_);
    // sql_->InvertTransValue(node_id_);
}

void LeafWidgetO::RUnitGroupClicked(int id)
{
    const UnitO unit { id };
    node_->final_total = 0.0;

    switch (unit) {
    case UnitO::kImmediate:
        node_->final_total = node_->initial_total - node_->discount_total;
        [[fallthrough]];
    case UnitO::kMonthly:
        ui->pBtnFinishOrder->setEnabled(true);
        break;
    case UnitO::kPending:
        ui->pBtnFinishOrder->setEnabled(false);
        break;
    default:
        break;
    }

    node_->unit = id;
    ui->dSpinFinalTotal->setValue(node_->final_total);

    // sql_->WriteField(party_info_, kUnit, id, node_id_);
    // sql_->WriteField(party_info_, kFinalTotal, node_->final_total, node_id_);
}

void LeafWidgetO::on_pBtnFinishOrder_toggled(bool checked)
{
    node_->is_finished = checked;
    // sql_->WriteField(party_info_, kIsFinished, checked, node_id_);

    emit SSyncFinished(node_id_, checked);

    IniFinished(checked);
    LockWidgets(checked);

    if (checked)
        sql_->SyncPrice(node_id_);
}

void LeafWidgetO::on_pBtnPrint_clicked()
{
    PreparePrint();
    print_manager_->Print();
}

void LeafWidgetO::on_pBtnPreview_clicked()
{
    PreparePrint();
    print_manager_->Preview();
}

void LeafWidgetO::PreparePrint()
{
    print_manager_->LoadIni(ui->comboTemplate->currentData().toString());

    QString unit {};
    switch (UnitO(node_->unit)) {
    case UnitO::kMonthly:
        unit = tr("MS");
        break;
    case UnitO::kImmediate:
        unit = tr("IS");
        break;
    case UnitO::kPending:
        unit = tr("PEND");
        break;
    default:
        break;
    }

    PrintData data { tree_model_stakeholder_->Name(node_->party), node_->issued_time.toLocalTime().toString(kDateTimeFST),
        tree_model_stakeholder_->Name(node_->employee), unit, node_->initial_total };
    print_manager_->SetData(data, leaf_model_order_->GetEntryShadowList());
}

void LeafWidgetO::on_pBtnSaveOrder_clicked()
{
    if (is_insert_)
        emit SSaveOrder();

    is_insert_ = false;
}
