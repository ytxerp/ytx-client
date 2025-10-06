#include "treewidgetf.h"

#include "component/constant.h"
#include "component/signalblocker.h"
#include "ui_treewidgetf.h"

TreeWidgetF::TreeWidgetF(TreeModel* model, CSectionInfo& info, CGlobalConfig global, CSectionConfig& section, QWidget* parent)
    : TreeWidget(parent)
    , ui(new Ui::TreeWidgetF)
    , model_ { model }
    , info_ { info }
    , section_ { section }
    , global_ { global }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->treeViewF->setModel(model);
    ui->dspin_box_dynamic_->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    ui->dspin_box_static_->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    UpdateStatus();
}

TreeWidgetF::~TreeWidgetF() { delete ui; }

void TreeWidgetF::UpdateStatus()
{
    UpdateStaticStatus();
    UpdateDynamicStatus();
}

void TreeWidgetF::UpdateStaticStatus()
{
    ui->dspin_box_static_->setDecimals(section_.amount_decimal);
    ui->lable_static_->setText(section_.static_label);

    const auto static_node_id { section_.static_node };

    if (!model_->Contains(static_node_id)) {
        ResetStatus(ui->dspin_box_static_, static_unit_is_default_);
        return;
    }

    const int static_unit { model_->Unit(static_node_id) };
    static_unit_is_default_ = static_unit == global_.default_unit;

    ui->dspin_box_static_->setPrefix(info_.unit_symbol_map.value(static_unit, kEmptyString));
    UpdateStaticValue(static_node_id);
}

void TreeWidgetF::UpdateDynamicStatus()
{
    const int default_unit { global_.default_unit };

    ui->dspin_box_dynamic_->setDecimals(section_.amount_decimal);
    ui->label_dynamic_->setText(section_.dynamic_label);

    const auto dynamic_node_id_lhs { section_.dynamic_node_lhs };
    const auto dynamic_node_id_rhs { section_.dynamic_node_rhs };

    if (!model_->Contains(dynamic_node_id_lhs) && !model_->Contains(dynamic_node_id_rhs)) {
        ResetStatus(ui->dspin_box_dynamic_, dynamic_unit_is_not_default_but_equal_);
        return;
    }

    const int lhs_unit { model_->Unit(dynamic_node_id_lhs) };
    const int rhs_unit { model_->Unit(dynamic_node_id_rhs) };
    dynamic_unit_is_not_default_but_equal_ = (lhs_unit == rhs_unit && lhs_unit != default_unit);

    ui->dspin_box_dynamic_->setPrefix(info_.unit_symbol_map.value((dynamic_unit_is_not_default_but_equal_ ? lhs_unit : default_unit), kEmptyString));
    UpdateDynamicValue(dynamic_node_id_lhs, dynamic_node_id_rhs);
}

QTreeView* TreeWidgetF::View() const { return ui->treeViewF; }

void TreeWidgetF::RTotalsUpdated()
{
    UpdateStaticValue(section_.static_node);
    UpdateDynamicValue(section_.dynamic_node_lhs, section_.dynamic_node_rhs);
}

void TreeWidgetF::UpdateDynamicValue(const QUuid& lhs_node_id, const QUuid& rhs_node_id)
{
    if (lhs_node_id.isNull() && rhs_node_id.isNull())
        return;

    const double lhs_total { dynamic_unit_is_not_default_but_equal_ ? model_->InitialTotal(lhs_node_id) : model_->FinalTotal(lhs_node_id) };
    const double rhs_total { dynamic_unit_is_not_default_but_equal_ ? model_->InitialTotal(rhs_node_id) : model_->FinalTotal(rhs_node_id) };

    const auto& operation { section_.operation.isEmpty() ? kPlus : section_.operation };
    const double total { Operate(lhs_total, rhs_total, operation) };

    ui->dspin_box_dynamic_->setValue(total);
}

void TreeWidgetF::UpdateStaticValue(const QUuid& node_id)
{
    if (node_id.isNull())
        return;

    ui->dspin_box_static_->setValue(static_unit_is_default_ ? model_->FinalTotal(node_id) : model_->InitialTotal(node_id));
}

double TreeWidgetF::Operate(double lhs, double rhs, const QString& operation)
{
    switch (operation.at(0).toLatin1()) {
    case '+':
        return lhs + rhs;
    case '-':
        return lhs - rhs;
    default:
        return 0.0;
    }
}

void TreeWidgetF::ResetStatus(QDoubleSpinBox* spin_box, bool& flags)
{
    spin_box->setPrefix(kEmptyString);
    spin_box->setValue(0.0);
    flags = false;
}
