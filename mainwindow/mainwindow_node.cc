#include "component/arg/nodeinsertarg.h"
#include "dialog/editnodename.h"
#include "dialog/insertnode/insertnodefinance.h"
#include "dialog/insertnode/insertnodei.h"
#include "dialog/insertnode/insertnodep.h"
#include "dialog/insertnode/insertnodetask.h"
#include "global/nodepool.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::InsertNodeFIPT(Node* parent_node, int row)
{
    auto tree_model { sc_->tree_model };
    auto unit_model { sc_->info.unit_model };

    auto parent_path { tree_model->Path(parent_node->id) };
    if (!parent_path.isEmpty())
        parent_path += app_config_.separator;

    auto* node { NodePool::Instance().Allocate(start_) };

    node->id = QUuid::createUuidV7();
    node->direction_rule = parent_node->direction_rule;
    node->unit = parent_node->unit;
    node->parent = parent_node;

    QDialog* dialog {};

    const auto children_name { ChildrenName(parent_node) };
    const auto arg { NodeInsertArg { node, unit_model, parent_path, children_name } };

    switch (start_) {
    case Section::kFinance:
        dialog = new InsertNodeFinance(arg, this);
        break;
    case Section::kTask:
        dialog = new InsertNodeTask(arg, this);
        break;
    case Section::kPartner:
        dialog = new InsertNodeP(arg, this);
        break;
    case Section::kInventory:
        dialog = new InsertNodeI(arg, sc_->section_config.rate_decimal, this);
        break;
    default:
        return NodePool::Instance().Recycle(node, start_);
    }

    connect(dialog, &QDialog::accepted, this, [=, this]() {
        if (tree_model->InsertNode(parent_node, node, row)) {
            auto index { tree_model->GetIndex(parent_node->id) };
            sc_->tree_view->setCurrentIndex(index);
        }
    });

    connect(dialog, &QDialog::rejected, this, [=, this]() { NodePool::Instance().Recycle(node, start_); });
    dialog->exec();
}

void MainWindow::RNodeLocation(const QUuid& node_id)
{
    // Ignore report widget
    if (node_id.isNull())
        return;

    auto index { sc_->tree_model->GetIndex(node_id) };
    if (!index.isValid())
        return;

    auto widget { sc_->tree_widget };
    ui->tabWidget->setCurrentWidget(widget);
    widget->activateWindow();
    widget->View()->setCurrentIndex(index);
}

void MainWindow::EditNameFIPT()
{
    assert(sc_->tree_widget);

    const auto index { sc_->tree_view->currentIndex() };
    if (!index.isValid())
        return;

    const auto node_id { index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() };
    auto model { sc_->tree_model };

    const auto parent { index.parent() };
    const auto* parent_node { model->GetNodeByIndex(parent) };
    auto parent_path { model->Path(parent_node->id) };

    if (!parent_path.isEmpty())
        parent_path += app_config_.separator;

    CString name { model->Name(node_id) };
    const auto children_name { ChildrenName(parent_node) };

    auto* edit_name { new EditNodeName(name, parent_path, children_name, this) };
    connect(edit_name, &QDialog::accepted, this, [=]() { model->UpdateName(node_id, edit_name->GetName()); });
    edit_name->exec();
}

void MainWindow::on_actionResetColor_triggered()
{
    if (start_ != Section::kInventory && start_ != Section::kTask)
        return;

    assert(sc_->tree_widget);

    const auto index { sc_->tree_view->currentIndex() };
    if (!index.isValid())
        return;

    auto model { sc_->tree_model };
    model->ResetColor(index);
}
