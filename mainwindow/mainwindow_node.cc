#include "component/arg/nodeinsertarg.h"
#include "dialog/editnodename.h"
#include "dialog/insertnode/insertnodefinance.h"
#include "dialog/insertnode/insertnodei.h"
#include "dialog/insertnode/insertnodep.h"
#include "dialog/insertnode/insertnodetask.h"
#include "global/nodepool.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils/mainwindowutils.h"
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
    node->unit = parent_index.isValid() ? parent_node->unit : NodeUnit(sc_->shared_config.default_unit);
    node->parent = parent_node;

    QDialog* dialog {};

    const auto children_name { ChildrenName(parent_node) };
    const auto arg { NodeInsertArg { node, unit_model, parent_path, children_name } };

    switch (start_) {
    case Section::kFinance:
        dialog = new InsertNodeFinance(arg, this);
        break;
    case Section::kTask:
        static_cast<NodeT*>(node)->issued_time = QDateTime::currentDateTimeUtc();
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
        return;
    }

    Utils::ManageDialog(sc_->view_hash, dialog);
    dialog->setModal(true);

    connect(dialog, &QDialog::accepted, this, [=, this]() {
        const auto message { JsonGen::NodeInsert(start_, node, parent_node->id) };
        WebSocket::Instance()->SendMessage(kNodeInsert, message);
    });
    connect(dialog, &QDialog::destroyed, this, [=, this]() { NodePool::Instance().Recycle(node, start_); });

    dialog->show();
}

void MainWindow::RNodeLocation(Section section, const QUuid& node_id)
{
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

    auto* dialog { new EditNodeName(name, parent_path, children_name, this) };

    Utils::ManageDialog(sc_->view_hash, dialog);
    dialog->setModal(true);

    connect(dialog, &QDialog::accepted, this, [=, this]() {
        const auto message { JsonGen::NodeName(start_, node_id, dialog->GetName()) };
        WebSocket::Instance()->SendMessage(kNodeName, message);
    });

    dialog->show();
}

void MainWindow::on_actionClearColor_triggered()
{
    qInfo() << "[UI]" << "on_actionClearColor_triggered";

    Q_ASSERT_X(start_ != Section::kSale && start_ != Section::kPurchase, Q_FUNC_INFO, "ClearColor action is not allowed in Sale or Purchase sections");

    const auto index { sc_->tree_view->currentIndex() };
    if (!index.isValid())
        return;

    auto model { sc_->tree_model };

    const int color_column { Utils::NodeColorColumn(start_) };
    const QModelIndex color_index { index.siblingAtColumn(color_column) };

    model->setData(color_index, QVariant());
}
