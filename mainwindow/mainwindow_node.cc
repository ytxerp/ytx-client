#include "component/arg/nodeinsertarg.h"
#include "dialog/editnodename.h"
#include "dialog/insertnode/insertnodefinance.h"
#include "dialog/insertnode/insertnodei.h"
#include "dialog/insertnode/insertnodep.h"
#include "dialog/insertnode/insertnodetask.h"
#include "global/nodepool.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

void MainWindow::InsertNodeFIPT(const QModelIndex& parent_index)
{
    Q_ASSERT_X(start_ != Section::kSale && start_ != Section::kPurchase, Q_FUNC_INFO, "InsertNodeFIPT must not be used in Sale or Purchase section");

    auto tree_model { sc_->tree_model };
    auto unit_model { sc_->info.unit_model };
    auto* parent_node { sc_->tree_model->GetNodeByIndex(parent_index) };

    auto parent_path { tree_model->Path(parent_node->id) };
    if (!parent_path.isEmpty())
        parent_path += app_config_.separator;

    auto* node { NodePool::Instance().Allocate(start_) };

    node->id = QUuid::createUuidV7();
    node->direction_rule = parent_node->direction_rule;
    node->unit = parent_index.isValid() ? parent_node->unit : sc_->shared_config.default_unit;
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
    case Section::kSale:
    case Section::kPurchase:
        break;
    }

    connect(dialog, &QDialog::accepted, this, [=, this]() {
        const auto message { JsonGen::NodeInsert(start_, node, parent_node->id) };
        WebSocket::Instance()->SendMessage(kNodeInsert, message);
    });

    connect(dialog, &QDialog::finished, this, [=, this]() { NodePool::Instance().Recycle(node, start_); });
    dialog->exec();
}

void MainWindow::RNodeLocation(Section section, const QUuid& node_id)
{
    // Ignore report widget
    if (node_id.isNull())
        return;

    auto* sc { GetSectionContex(section) };

    auto index { sc->tree_model->GetIndex(node_id) };
    if (!index.isValid())
        return;

    auto widget { sc->tree_widget };
    ui->tabWidget->setCurrentWidget(widget);
    widget->activateWindow();

    widget->View()->setCurrentIndex(index);
    widget->View()->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void MainWindow::EditNameFIPT()
{
    Q_ASSERT_X(start_ != Section::kSale && start_ != Section::kPurchase, Q_FUNC_INFO, "EditNameFIPT must not be used in Sale or Purchase section");
    Q_ASSERT_X(sc_ && sc_->tree_widget && sc_->tree_model, Q_FUNC_INFO, "SectionContext is not fully initialized");

    const auto index { sc_->tree_view->currentIndex() };
    if (!index.isValid())
        return;

    auto* node { static_cast<Node*>(index.internalPointer()) };
    const auto node_id { node->id };

    auto model { sc_->tree_model };

    auto* parent_node { node->parent };
    QString parent_path { model->Path(parent_node->id) };

    if (!parent_path.isEmpty())
        parent_path += app_config_.separator;

    CString name { model->Name(node_id) };
    const auto children_name { ChildrenName(parent_node) };

    auto* edit_name { new EditNodeName(name, parent_path, children_name, this) };
    connect(edit_name, &QDialog::accepted, this, [=, this]() {
        const auto message { JsonGen::NodeName(start_, node_id, edit_name->GetName()) };
        WebSocket::Instance()->SendMessage(kNodeName, message);
    });
    edit_name->exec();
}

void MainWindow::on_actionResetColor_triggered()
{
    Q_ASSERT_X(start_ == Section::kInventory || start_ == Section::kTask, Q_FUNC_INFO, "ResetColor action is only valid in Inventory or Task section");

    assert(sc_->tree_widget);

    const auto index { sc_->tree_view->currentIndex() };
    if (!index.isValid())
        return;

    auto model { sc_->tree_model };
    model->ResetColor(index);
}
