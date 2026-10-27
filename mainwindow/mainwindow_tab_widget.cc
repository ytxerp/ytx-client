#include "component/constantwebsocket.h"
#include "entryhub/entryhubp.h"
#include "global/tablesstation.h"
#include "mainwindow.h"
#include "tree/model/treemodelo.h"
#include "ui_mainwindow.h"
#include "utils/entryutils.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

void MainWindow::SetTabWidget(QTabWidget* tab_widget)
{
    auto* tab_bar { tab_widget->tabBar() };

    tab_bar->setDocumentMode(true);
    tab_bar->setExpanding(false);

    tab_widget->setMovable(true);
    tab_widget->setTabsClosable(true);
    tab_widget->setElideMode(Qt::ElideNone);
}

void MainWindow::RTreeViewDoubleClicked(const QModelIndex& index)
{
    qInfo() << Q_FUNC_INFO;

    if (index.column() != std::to_underlying(NodeEnum::kName))
        return;

    auto* node { static_cast<Node*>(index.internalPointer()) };
    Q_ASSERT(node);

    if (!node)
        return;

    if (node->kind == NodeKind::kBranch)
        return;

    if (node->unit == NodeUnit::PEmployee)
        return;

    ShowLeafWidget(node->id);
}

// ShowLeafWidget - Open a leaf widget and optionally select an entry
// ---------------------------------------------------------------
// Triggers:
//  • Node double-click: entry_id empty → no entry selection
//  • Entry jump: entry_id valid → select the specified entry
//
// Flow:
//  1. If widget exists → focus tab and select entry
//  2. Otherwise → create widget and request entry data
//
// Note: Entry selection is only needed when entry_id is valid.
void MainWindow::ShowLeafWidget(QUuid node_id, QUuid entry_id)
{
    const Section section { sc_->section };

    if (sc_->widget_hash.contains(node_id)) {
        LocateLeafWidget(sc_, node_id, entry_id);
        return;
    }

    if (IsOrderSection(section)) {
        CreateLeafO(sc_, node_id);
    } else {
        CreateLeafFIPT(sc_, node_id);
    }

    if (section == Section::kPartner) {
        static_cast<EntryHubP*>(sc_p_.entry_hub.data())->PushEntry(node_id);
    } else {
        const auto message { JsonGen::TableAck(section, node_id, entry_id) };
        WebSocket::Instance()->SendMessage(WsKey::kTableAck, message);
    }
}

void MainWindow::tabWidget_currentChanged()
{
    qInfo() << Q_FUNC_INFO;

    if (!section_settings_)
        return;

    auto* widget { sc_->tab_widget->currentWidget() };
    if (!widget)
        return;

    const Section section { sc_->section };

    const bool is_tree { IsTreeWidget(widget) };
    const bool is_tree_o { IsTreeWidgetO(widget) };
    const bool is_table_fipt { IsTableWidgetFIPT(widget) };
    const bool is_statement_tertiary { IsStatementTertiaryWidget(widget) };
    const bool is_table_o { IsTableWidgetO(widget) };
    const bool is_order_section { IsOrderSection(section) };
    const bool is_finance_section { section == Section::kFinance };
    const bool is_inventory_section { section == Section::kInventory };

    ui->actionAppendNode->setEnabled(is_tree);
    ui->actionInsertNode->setEnabled(is_tree || is_table_o);

    ui->actionRename->setEnabled(is_tree);
    ui->actionClearColor->setEnabled(is_tree);

    ui->actionMarkAll->setEnabled(is_table_fipt || is_statement_tertiary);
    ui->actionMarkNone->setEnabled(is_table_fipt || is_statement_tertiary);
    ui->actionMarkToggle->setEnabled(is_table_fipt || is_statement_tertiary);

    ui->actionLockAll->setEnabled(is_table_fipt);
    ui->actionUnlockAll->setEnabled(is_table_fipt);

    ui->actionJumpEntry->setEnabled(is_table_fipt || is_table_o);

    ui->actionStatement->setEnabled(is_order_section);
    ui->actionSettlement->setEnabled(is_order_section);
    ui->actionSettlementView->setEnabled(is_order_section);
    ui->actionOrderOverview->setEnabled(is_order_section);
    ui->actionNewBranch->setEnabled(is_tree_o);

    ui->actionAppendEntry->setEnabled(is_table_fipt || is_table_o);
    ui->actionDelete->setEnabled(is_tree || is_table_fipt || is_table_o);

    ui->actionBalanceSheet->setEnabled(is_finance_section);
    ui->actionCashFlowStatement->setEnabled(is_finance_section);
    ui->actionIncomeStatement->setEnabled(is_finance_section);
    ui->actionPeriodClose->setEnabled(is_finance_section);

    ui->actionHeatInventory->setEnabled(is_inventory_section);
    ui->actionMergeInventory->setEnabled(is_inventory_section);
    ui->actionHeatPartner->setEnabled(section == Section::kPartner);
}

void MainWindow::tabWidget_tabBarDoubleClicked(int index)
{
    qInfo() << Q_FUNC_INFO;

    auto* tab_bar { sc_->tab_widget->tabBar() };
    const auto id { tab_bar->tabData(index).toUuid() };

    if (sc_->widget_hash.contains(id))
        RNodeLocate(sc_->section, id);
}

void MainWindow::tabWidget_tabCloseRequestedFIPT(int index)
{
    qInfo() << Q_FUNC_INFO;

    const auto node_id { sc_->tab_widget->tabBar()->tabData(index).toUuid() };

    utils::DeleteWidget(sc_->widget_hash, node_id);
    TableSStation::Instance()->DeregisterModel(node_id);
}

void MainWindow::tabWidget_tabCloseRequestedO(int index)
{
    qInfo() << Q_FUNC_INFO;

    const auto node_id { sc_->tab_widget->tabBar()->tabData(index).toUuid() };

    auto* widget { qobject_cast<TableWidgetO*>(sc_->widget_hash.value(node_id).widget.data()) };
    if (!widget) {
        qWarning() << Q_FUNC_INFO << "Invalid widget:" << node_id;
        return;
    }

    if (widget->HasPendingUpdate()) {
        auto* dlg = utils::CreateMessage(QMessageBox::Warning, tr("Unsaved Changes"),
            tr("This page contains unsaved changes.\n\nDo you want to save them before closing?"), true,
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);

        dlg->setDefaultButton(QMessageBox::Cancel);

        const int ret { dlg->exec() };

        if (ret == QMessageBox::Cancel)
            return;

        if (ret == QMessageBox::Save)
            widget->SaveOrder();
    }

    utils::DeleteWidget(sc_->widget_hash, node_id);
    TableSStation::Instance()->DeregisterModel(node_id);
}

void MainWindow::on_actionJumpEntry_triggered()
{
    qInfo() << Q_FUNC_INFO;

    auto* widget { sc_->tab_widget->currentWidget() };

    Q_ASSERT(qobject_cast<TableWidget*>(widget));
    auto* leaf_widget { static_cast<TableWidget*>(widget) };

    const auto index { leaf_widget->View()->currentIndex() };
    if (!index.isValid())
        return;

    const int row { index.row() };
    const Section section { sc_->section };
    const int linked_node_column { entry::LinkedNodeColumn(section) };

    const auto linked_node_id { index.sibling(row, linked_node_column).data().toUuid() };
    if (linked_node_id.isNull()) {
        const QModelIndex linked_node_index { index.sibling(row, linked_node_column) };
        if (index == linked_node_index) // already on the linked node column, already editing
            return;

        leaf_widget->View()->setCurrentIndex(linked_node_index);
        leaf_widget->View()->edit(linked_node_index);
        return;
    }

    if (IsSingleEntry(section))
        return;

    auto* entry { leaf_widget->Model()->GetEntry(index) };
    ShowLeafWidget(linked_node_id, entry->id);
}

void MainWindow::RUpdateName(QUuid node_id, const QString& name, bool branch)
{
    auto model { sc_->tree_model };
    auto widget { sc_->tab_widget };

    if (!model || !widget) {
        return;
    }

    if (branch) {
        UpdateBranchNodeName(node_id);
        return;
    }

    UpdateLeafNodeName(node_id, name);
}

void MainWindow::UpdateLeafNodeName(QUuid node_id, const QString& name)
{
    if (!sc_->widget_hash.contains(node_id)) {
        return;
    }

    const QSet<QUuid> nodes { node_id };

    if (sc_->section == Section::kPartner) {
        UpdatePartnerReference(&sc_sale_, nodes, false);
        UpdatePartnerReference(&sc_purchase_, nodes, false);
    }

    UpdateOneTab(node_id, name);
}

void MainWindow::UpdateOneTab(QUuid node_id, const QString& name)
{
    auto widget { sc_->tab_widget };
    auto model { sc_->tree_model };
    auto* tab_bar { widget->tabBar() };
    const int count { widget->count() };

    for (int index = 0; index != count; ++index) {
        const QUuid id { tab_bar->tabData(index).toUuid() };
        if (id != node_id)
            continue;

        tab_bar->setTabText(index, name);
        tab_bar->setTabToolTip(index, model->Path(id));
        break;
    }
}

void MainWindow::UpdateBranchNodeName(QUuid node_id)
{
    auto model { sc_->tree_model };

    if (auto* node = model->GetNode(node_id)) {
        const QSet<QUuid> nodes { LeafChildrenId(node) };

        if (sc_->section == Section::kPartner) {
            UpdatePartnerReference(&sc_sale_, nodes, true);
            UpdatePartnerReference(&sc_purchase_, nodes, true);
        }

        UpdateMultiTabs(nodes);
    }
}

void MainWindow::UpdateMultiTabs(const QSet<QUuid>& nodes)
{
    auto widget { sc_->tab_widget };
    auto model { sc_->tree_model };
    auto* tab_bar { widget->tabBar() };
    const int count { widget->count() };

    for (int index = 0; index != count; ++index) {
        const QUuid id { tab_bar->tabData(index).toUuid() };
        if (!nodes.contains(id))
            continue;

        tab_bar->setTabToolTip(index, model->Path(id));
    }
}

void MainWindow::UpdatePartnerReference(const SectionContext* sc, const QSet<QUuid>& partner_nodes, const bool branch) const
{
    auto widget { sc->tab_widget };
    auto partner_model { sc_p_.tree_model };
    auto* order_model { static_cast<TreeModelO*>(sc->tree_model.data()) };

    if (!widget || !partner_model || !order_model) {
        return;
    }

    auto* tab_bar { widget->tabBar() };
    const int count { widget->count() };

    for (int index = 0; index != count; ++index) {
        const QUuid node_id { tab_bar->tabData(index).toUuid() };
        if (node_id.isNull()) {
            continue;
        }

        const QUuid partner_id { order_model->Partner(node_id) };
        if (!partner_nodes.contains(partner_id)) {
            continue;
        }

        const QString path { partner_model->Path(partner_id) };

        if (!branch) {
            const QString name { partner_model->Name(partner_id) };
            tab_bar->setTabText(index, name);
        }

        tab_bar->setTabToolTip(index, path);
    }
}
