#include "treewidgetit.h"

#include "component/constant.h"
#include "component/signalblocker.h"
#include "ui_treewidgetit.h"

TreeWidgetIT::TreeWidgetIT(TreeModel* model, CSectionConfig& config, QWidget* parent)
    : TreeWidget(parent)
    , ui(new Ui::TreeWidgetIT)
    , model_ { model }
    , config_ { config }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->treeView->setModel(model);
    ui->dspin_box_dynamic_->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
    ui->dspin_box_static_->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
    InitStatus();
}

TreeWidgetIT::~TreeWidgetIT() { delete ui; }

void TreeWidgetIT::InitStatus()
{
    InitStaticStatus();
    InitDynamicStatus();
}

void TreeWidgetIT::InitStaticStatus()
{
    ui->dspin_box_static_->setDecimals(config_.quantity_decimal);
    ui->lable_static_->setText(config_.static_label);

    const auto static_node_id { config_.static_node };

    if (!model_->Contains(static_node_id)) {
        ui->dspin_box_static_->setValue(0.0);
        return;
    }

    UpdateStaticValue(static_node_id);
}

void TreeWidgetIT::InitDynamicStatus()
{
    ui->dspin_box_dynamic_->setDecimals(config_.quantity_decimal);
    ui->label_dynamic_->setText(config_.dynamic_label);

    const auto dynamic_node_id_lhs { config_.dynamic_node_lhs };
    const auto dynamic_node_id_rhs { config_.dynamic_node_rhs };

    if (!model_->Contains(dynamic_node_id_lhs) && !model_->Contains(dynamic_node_id_rhs)) {
        ui->dspin_box_dynamic_->setValue(0.0);
        return;
    }

    UpdateDynamicValue(dynamic_node_id_lhs, dynamic_node_id_rhs);
}

QTreeView* TreeWidgetIT::View() const { return ui->treeView; }

void TreeWidgetIT::RTotalsUpdated()
{
    UpdateStaticValue(config_.static_node);
    UpdateDynamicValue(config_.dynamic_node_lhs, config_.dynamic_node_rhs);
}

void TreeWidgetIT::UpdateDynamicValue(const QUuid& lhs_node_id, const QUuid& rhs_node_id)
{
    if (lhs_node_id.isNull() && rhs_node_id.isNull())
        return;

    const double lhs_total { model_->InitialTotal(lhs_node_id) };
    const double rhs_total { model_->InitialTotal(rhs_node_id) };

    const auto& operation { config_.operation.isEmpty() ? kPlus : config_.operation };
    const double total { Operate(lhs_total, rhs_total, operation) };

    ui->dspin_box_dynamic_->setValue(total);
}

void TreeWidgetIT::UpdateStaticValue(const QUuid& node_id)
{
    if (node_id.isNull())
        return;

    ui->dspin_box_static_->setValue(model_->InitialTotal(node_id));
}
