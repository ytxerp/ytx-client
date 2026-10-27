#include "component/arg/nodeinsertarg.h"
#include "component/constantwebsocket.h"
#include "dialog/editnodename.h"
#include "dialog/insertnode/insertnodefit.h"
#include "dialog/insertnode/insertnodep.h"
#include "dialog/mergenode/leafmergedialog.h"
#include "global/nodepool.h"
#include "mainwindow.h"
#include "utils/mainwindowutils.h"
#include "utils/nodeutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

void MainWindow::InsertNodeFIPT(SectionContext* sc, const QModelIndex& parent_index)
{
    const Section section { sc->section };

    Q_ASSERT_X(section != Section::kSale && section != Section::kPurchase, Q_FUNC_INFO, "InsertNodeFIPT must not be used in Sale or Purchase section");

    auto tree_model { sc->tree_model };
    auto unit_model { sc->info.unit_model };
    auto* parent_node { sc->tree_model->GetNodeByIndex(parent_index) };

    auto parent_path { tree_model->Path(parent_node->id) };
    if (!parent_path.isEmpty())
        parent_path += app_config_.separator;

    auto* node { NodePool::Instance().Allocate(section) };

    node->id = QUuid::createUuidV7();
    node->direction_rule = parent_node->direction_rule;
    node->unit = parent_index.isValid() ? parent_node->unit : NodeUnit(sc->shared_config.default_unit);
    node->parent = parent_node;

    QDialog* dialog {};
    WsKey key { WsKey::kNodeInsert };

    const auto children_name { ChildrenName(parent_node) };
    const auto arg { NodeInsertArg { node, unit_model, parent_path, children_name } };

    switch (section) {
    case Section::kFinance:
    case Section::kTask:
    case Section::kInventory:
        dialog = new InsertNodeFIT(arg, nullptr);
        break;
    case Section::kPartner:
        dialog = new InsertNodeP(arg, nullptr);
        key = WsKey::kPartnerNodeInsert;
        break;
    case Section::kSale:
    case Section::kPurchase:
        return;
    }

    utils::ManageDialog(sc->widget_hash, dialog);
    dialog->setModal(true);

    connect(dialog, &QDialog::accepted, this, [node, parent_node, key, section]() {
        const auto message { JsonGen::NodeInsert(section, node, parent_node->id) };
        WebSocket::Instance()->SendMessage(key, message);
    });
    connect(dialog, &QDialog::destroyed, this, [node, section]() { NodePool::Instance().Recycle(node, section); });

    dialog->show();
}

void MainWindow::RNodeLocate(Section section, QUuid node_id)
{
    if (node_id.isNull())
        return;

    auto* sc { GetSectionContext(section) };
    auto* tree_model { sc->tree_model.data() };

    // Only the Sale and Purchase sections load tree nodes on demand; all other
    // sections load the full tree up front. If the node is not in the local
    // tree yet, request it from the server (AckNode) and return. The locate
    // request is dropped for this call.
    if (!tree_model->Contains(node_id)) {
        if (section == Section::kSale || section == Section::kPurchase)
            tree_model->AckNode(node_id);

        return;
    }

    auto index { tree_model->GetIndex(node_id) };
    if (!index.isValid())
        return;

    auto widget { sc->tree_widget };
    sc->tab_widget->setCurrentWidget(widget);

    widget->View()->setCurrentIndex(index);
    widget->View()->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    widget->window()->raise();
    widget->window()->activateWindow();
}

void MainWindow::RNodeSelect(Section section, QUuid node_id)
{
    auto* sc { GetSectionContext(section) };
    auto tree_model { sc->tree_model };

    const auto index { tree_model->GetIndex(node_id) };
    if (!index.isValid())
        return;

    sc->tree_view->setCurrentIndex(index);
}

void MainWindow::RMessage(QMessageBox::Icon icon, const QString& message)
{
    utils::ShowMessage(icon, tr("Constraint Violation"), message, time_const::kAutoCloseMs, QMessageBox::NoButton, this);
}

void MainWindow::EditNameFIPT()
{
    const Section section { sc_->section };

    Q_ASSERT_X(section != Section::kSale && section != Section::kPurchase, Q_FUNC_INFO, "EditNameFIPT must not be used in Sale or Purchase section");
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

    auto* dialog { new EditNodeName(name, parent_path, children_name, nullptr) };

    utils::ManageDialog(sc_->widget_hash, dialog);
    dialog->setModal(true);

    connect(dialog, &QDialog::accepted, this, [node_id, dialog, node, section]() {
        const auto message { JsonGen::NodeName(section, node_id, dialog->GetName(), node->version) };
        WebSocket::Instance()->SendMessage(WsKey::kNodeNameUpdate, message);
    });

    dialog->show();
}

void MainWindow::on_actionClearColor_triggered()
{
    qInfo() << Q_FUNC_INFO;

    const auto index { sc_->tree_view->currentIndex() };
    if (!index.isValid())
        return;

    auto model { sc_->tree_model };
    const Section section { sc_->section };

    const int color_column { node::ColorColumn(section) };
    const QModelIndex color_index { index.siblingAtColumn(color_column) };

    model->setData(color_index, QVariant());
}

void MainWindow::on_actionMergeInventory_triggered()
{
    qInfo() << Q_FUNC_INFO;
    auto* dialog { new LeafMergeDialog(sc_i_.tree_model, this) };

    utils::ManageDialog(sc_i_.widget_hash, dialog);
    dialog->show();
}
