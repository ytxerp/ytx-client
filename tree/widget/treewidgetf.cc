#include "treewidgetf.h"

#include "component/constant.h"
#include "component/signalblocker.h"
#include "ui_treewidgetf.h"

TreeWidgetF::TreeWidgetF(TreeModel* model, CSectionInfo& info, CSharedConfig shared, CSectionConfig& section, QWidget* parent)
    : TreeWidget(parent)
    , ui(new Ui::TreeWidgetF)
    , model_ { model }
    , info_ { info }
    , section_ { section }
    , shared_ { shared }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->treeView->setModel(model);

    ui->dspin_box_dynamic_->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
    ui->dspin_box_static_->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());

    InitStatus();
}

TreeWidgetF::~TreeWidgetF() { delete ui; }

void TreeWidgetF::InitStatus()
{
    InitStaticStatus();
    InitDynamicStatus();
}

void TreeWidgetF::InitStaticStatus()
{
    ui->dspin_box_static_->setDecimals(section_.amount_decimal);
    ui->lable_static_->setText(section_.static_label);

    UpdateStaticValue(section_.static_node);
}

void TreeWidgetF::InitDynamicStatus()
{
    ui->dspin_box_dynamic_->setDecimals(section_.amount_decimal);
    ui->dspin_box_dynamic_->setPrefix(info_.unit_symbol_map.value(shared_.default_unit));
    ui->label_dynamic_->setText(section_.dynamic_label);

    const auto dynamic_node_id_lhs { section_.dynamic_node_lhs };
    const auto dynamic_node_id_rhs { section_.dynamic_node_rhs };

    if (!model_->Contains(dynamic_node_id_lhs) && !model_->Contains(dynamic_node_id_rhs)) {
        ui->dspin_box_dynamic_->setValue(0.0);
        return;
    }

    UpdateDynamicValue(dynamic_node_id_lhs, dynamic_node_id_rhs);
}

QTreeView* TreeWidgetF::View() const { return ui->treeView; }

void TreeWidgetF::RTotalsUpdated()
{
    UpdateStaticValue(section_.static_node);
    UpdateDynamicValue(section_.dynamic_node_lhs, section_.dynamic_node_rhs);
}

void TreeWidgetF::UpdateDynamicValue(const QUuid& lhs_node_id, const QUuid& rhs_node_id)
{
    if (lhs_node_id.isNull() && rhs_node_id.isNull())
        return;

    const double lhs_total { model_->FinalTotal(lhs_node_id) };
    const double rhs_total { model_->FinalTotal(rhs_node_id) };

    const auto& operation { section_.operation.isEmpty() ? kPlus : section_.operation };
    const double total { Operate(lhs_total, rhs_total, operation) };

    ui->dspin_box_dynamic_->setValue(total);
}

void TreeWidgetF::UpdateStaticValue(const QUuid& node_id)
{
    if (node_id.isNull())
        return;

    if (!model_->Contains(node_id)) {
        ui->dspin_box_static_->setValue(0.0);
        ui->dspin_box_static_->setPrefix(kEmptyString);
        return;
    }

    const int static_unit { model_->Unit(node_id) };

    ui->dspin_box_static_->setPrefix(info_.unit_symbol_map.value(static_unit));
    ui->dspin_box_static_->setValue(model_->InitialTotal(node_id));
}
