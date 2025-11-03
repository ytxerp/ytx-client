#include "leafwidgeto.h"

#include <QMessageBox>

#include "component/signalblocker.h"
#include "global/nodepool.h"
#include "ui_leafwidgeto.h"

LeafWidgetO::LeafWidgetO(CNodeOpArgO& arg, QWidget* parent)
    : LeafWidget(parent)
    , ui(new Ui::LeafWidgetO)
    , node_ { static_cast<NodeO*>(arg.node) }
    , sql_ { qobject_cast<EntryHubO*>(arg.entry_hub) }
    , leaf_model_order_ { qobject_cast<LeafModelO*>(arg.leaf_model) }
    , tree_model_partner_ { arg.tree_model_partner }
    , config_ { arg.section_config }
    , is_new_ { arg.is_new }
    , node_id_ { arg.node->id }
    , partner_unit_ { arg.section == Section::kSale ? std::to_underlying(UnitP::kCustomer) : std::to_underlying(UnitP::kVendor) }
    , print_template_ { arg.print_template }
    , print_manager_ { arg.app_config, arg.tree_model_inventory, arg.tree_model_partner }
{
    ui->setupUi(this);
    leaf_model_order_->setParent(this);
    SignalBlocker blocker(this);

    IniWidget();
    IniUnit(arg.node->unit);
    IniRuleGroup();
    IniUnitGroup();
    IniRule(arg.node->direction_rule);
    IniData(node_->partner, node_->employee);
    IniConnect();

    const bool released { node_->status == std::to_underlying(NodeStatus::kReleased) };
    ui->pBtnStatus->setChecked(released);

    IniStatus(released);
    LockWidgets(released);
}

LeafWidgetO::~LeafWidgetO()
{
    if (is_new_) {
        NodePool::Instance().Recycle(node_, Section::kSale);
    }

    delete leaf_model_order_;
    delete ui;
}

QTableView* LeafWidgetO::View() const { return ui->tableViewO; }

void LeafWidgetO::RSyncDelta(const QUuid& node_id, double initial_delta, double final_delta, double count_delta, double measure_delta, double discount_delta)
{
    if (node_id_ != node_id)
        return;

    const double adjusted_final_delta { node_->unit == std::to_underlying(UnitO::kImmediate) ? final_delta : 0.0 };

    node_->count_total += count_delta;
    node_->measure_total += measure_delta;
    node_->initial_total += initial_delta;
    node_->discount_total += discount_delta;
    node_->final_total += adjusted_final_delta;

    if (!is_new_) {
        count_delta_ += count_delta;
        measure_delta_ += measure_delta;
        initial_delta_ += initial_delta;
        discount_delta_ += discount_delta;
        final_delta_ += adjusted_final_delta;
    }

    IniUiValue();
}

void LeafWidgetO::IniWidget()
{
    pmodel_ = tree_model_partner_->IncludeUnitModel(partner_unit_);
    ui->comboPartner->setModel(pmodel_);
    ui->comboPartner->setCurrentIndex(-1);

    emodel_ = tree_model_partner_->IncludeUnitModel(std::to_underlying(UnitP::kEmployee));
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

void LeafWidgetO::IniData(const QUuid& partner, const QUuid& employee)
{
    if (is_new_) {
        const auto date_time { QDateTime::currentDateTimeUtc() };
        ui->dateTimeEdit->setDateTime(date_time.toLocalTime());
        node_->issued_time = date_time;
        return;
    }

    IniUiValue();
    ui->lineDescription->setText(node_->description);
    ui->dateTimeEdit->setDateTime(node_->issued_time.toLocalTime());

    int partner_index { ui->comboPartner->findData(partner) };
    ui->comboPartner->setCurrentIndex(partner_index);

    int employee_index { ui->comboEmployee->findData(employee) };
    ui->comboEmployee->setCurrentIndex(employee_index);
}

void LeafWidgetO::LockWidgets(bool released)
{
    const bool enable { !released };

    ui->comboPartner->setEnabled(enable);
    ui->comboEmployee->setEnabled(enable);

    ui->rBtnRO->setEnabled(enable);
    ui->rBtnTO->setEnabled(enable);
    ui->rBtnIS->setEnabled(enable);
    ui->rBtnMS->setEnabled(enable);
    ui->rBtnPEND->setEnabled(enable);

    ui->lineDescription->setReadOnly(released);
    ui->dateTimeEdit->setReadOnly(released);

    ui->pBtnPrint->setEnabled(released);
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

void LeafWidgetO::IniUiValue()
{
    ui->dSpinFinalTotal->setValue(node_->final_total);
    ui->dSpinDiscountTotal->setValue(node_->discount_total);
    ui->dSpinFirstTotal->setValue(node_->count_total);
    ui->dSpinSecondTotal->setValue(node_->measure_total);
    ui->dSpinInitialTotal->setValue(node_->initial_total);
}

void LeafWidgetO::IniRule(bool rule) { (rule ? ui->rBtnRO : ui->rBtnTO)->setChecked(true); }

void LeafWidgetO::IniStatus(bool released)
{
    ui->pBtnStatus->setText(released ? tr("Recall") : tr("Released"));
    ui->pBtnStatus->setEnabled(node_->unit != std::to_underlying(UnitO::kPending));

    if (released) {
        ui->pBtnPrint->setFocus();
        ui->pBtnPrint->setDefault(true);
        ui->tableViewO->clearSelection();
    }
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

void LeafWidgetO::on_comboPartner_currentIndexChanged(int /*index*/)
{
    const QUuid partner_id { ui->comboPartner->currentData().toUuid() };
    if (partner_id.isNull())
        return;

    node_->partner = partner_id;
    emit SSyncPartner(node_id_, partner_id);

    if (!is_new_) {
        update_cache_.insert(kPartner, partner_id.toString(QUuid::WithoutBraces));
    }
}

void LeafWidgetO::on_comboEmployee_currentIndexChanged(int /*index*/)
{
    const QUuid employee_id { ui->comboEmployee->currentData().toUuid() };
    node_->employee = employee_id;

    if (!is_new_) {
        update_cache_.insert(kEmployee, employee_id.toString(QUuid::WithoutBraces));
    }
}

void LeafWidgetO::on_dateTimeEdit_dateTimeChanged(const QDateTime& date_time)
{
    node_->issued_time = date_time.toUTC();

    if (!is_new_) {
        update_cache_.insert(kIssuedTime, node_->issued_time.toString(Qt::ISODate));
    }
}

void LeafWidgetO::on_lineDescription_editingFinished()
{
    node_->description = ui->lineDescription->text();

    if (!is_new_) {
        update_cache_.insert(kDescription, node_->description);
    }
}

void LeafWidgetO::RRuleGroupClicked(int id)
{
    node_->direction_rule = static_cast<bool>(id);

    node_->count_total *= -1;
    node_->measure_total *= -1;
    node_->initial_total *= -1;
    node_->discount_total *= -1;
    node_->final_total *= -1;

    IniUiValue();

    if (!is_new_) {
        update_cache_.insert(kDirectionRule, node_->direction_rule);
        count_delta_ *= -2;
        measure_delta_ *= -2;
        initial_delta_ *= -2;
        discount_delta_ *= -2;
        final_delta_ *= -2;
    }
}

void LeafWidgetO::RUnitGroupClicked(int id)
{
    const UnitO unit { id };

    switch (unit) {
    case UnitO::kImmediate:
        node_->final_total = node_->initial_total - node_->discount_total;
        final_delta_ += node_->final_total;
        ui->pBtnStatus->setEnabled(true);
        break;
    case UnitO::kMonthly:
        final_delta_ += -node_->final_total;
        node_->final_total = 0.0;
        ui->pBtnStatus->setEnabled(true);
        break;
    case UnitO::kPending:
        final_delta_ += -node_->final_total;
        node_->final_total = 0.0;
        ui->pBtnStatus->setEnabled(false);
        break;
    default:
        break;
    }

    node_->unit = id;
    ui->dSpinFinalTotal->setValue(node_->final_total);

    if (!is_new_) {
        update_cache_.insert(kUnit, id);
    }
}

void LeafWidgetO::on_pBtnStatus_toggled(bool checked)
{
    if (!node_->settlement.isNull()) {
        QMessageBox::information(this, tr("Order Locked"), tr("This order has already been settled and cannot be modified."));
        return;
    }

    node_->status = checked;
    emit SSyncStatus(node_id_, checked);

    IniStatus(checked);
    LockWidgets(checked);
}

void LeafWidgetO::on_pBtnPrint_clicked()
{
    PreparePrint();
    print_manager_.Print();
}

void LeafWidgetO::on_pBtnPreview_clicked()
{
    PreparePrint();
    print_manager_.Preview();
}

void LeafWidgetO::PreparePrint()
{
    print_manager_.LoadIni(ui->comboTemplate->currentData().toString());

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

    PrintData data { tree_model_partner_->Name(node_->partner), node_->issued_time.toLocalTime().toString(kDateTimeFST),
        tree_model_partner_->Name(node_->employee), unit, node_->initial_total };
    print_manager_.SetData(data, leaf_model_order_->GetEntryShadowList());
}

void LeafWidgetO::on_pBtnSave_clicked()
{
    if (is_new_) {
        emit SInsertOrder();
        is_new_ = false;
    }

    emit SSaveOrder();
}
