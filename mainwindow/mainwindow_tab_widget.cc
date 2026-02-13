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
    qInfo() << "[UI]" << "RTreeViewDoubleClicked";

    {
        const int expected_column { IsOrderSection(start_) ? std::to_underlying(NodeEnumO::kPartnerId) : std::to_underlying(NodeEnum::kName) };

        if (index.column() != expected_column)
            return;
    }

    {
        const int kind_column { Utils::KindColumn(start_) };
        const NodeKind kind { index.siblingAtColumn(kind_column).data().toInt() };
        if (kind == NodeKind::kBranch)
            return;
    }

    {
        const int unit_column { Utils::UnitColumn(start_) };
        const int unit { index.siblingAtColumn(unit_column).data().toInt() };
        if (start_ == Section::kInventory && unit == std::to_underlying(NodeUnit::IExternal))
            return;
    }

    {
        const auto node_id { index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() };
        Q_ASSERT(!node_id.isNull());

        ShowLeafWidget(node_id);
    }
}

// ShowLeafWidget - Show leaf widget (create and scroll as needed)
// -------------------------------
// Triggers:
//  • Node double-click: entry_id empty → no scroll
//  • Entry jump: entry_id valid → scroll to entry
//
// Flow:
//  1. If leaf exists → switch and scroll (if applicable)
//  2. If not exists → send request → create leaf → switch
//
// Note: Scrolling only occurs when entry_id is non-empty
void MainWindow::ShowLeafWidget(const QUuid& node_id, const QUuid& entry_id)
{
    {
        if (sc_->widget_hash.contains(node_id)) {
            FocusTabWidget(node_id);
            RSelectLeafEntry(node_id, entry_id);
            return;
        }
    }

    {
        if (start_ != Section::kPartner) {
            const auto message { JsonGen::LeafEntry(sc_->info.section, node_id, entry_id) };
            WebSocket::Instance()->SendMessage(kTableAcked, message);
        }

        if (IsOrderSection(start_)) {
            CreateLeafO(sc_, node_id);
        } else {
            CreateLeafFIPT(sc_, node_id);
        }

        FocusTabWidget(node_id);
    }
}

void MainWindow::tabWidget_currentChanged()
{
    qDebug() << "[UI]" << "tabWidget_currentChanged";

    if (!section_settings_)
        return;

    auto* widget { sc_->tab_widget->currentWidget() };
    if (!widget)
        return;

    const bool is_tree { IsTreeWidget(widget) };
    const bool is_table_fipt { IsTableWidgetFIPT(widget) };
    const bool is_table_o { IsTableWidgetO(widget) };
    const bool is_settlement { IsTreeWidgetSettlement(widget) };
    const bool is_order_section { IsOrderSection(start_) };
    const bool is_double_entry { IsDoubleEntry(start_) };

    ui->actionAppendNode->setEnabled(is_tree);
    ui->actionInsertNode->setEnabled(is_tree || is_table_o);

    ui->actionRename->setEnabled(is_tree);
    ui->actionClearColor->setEnabled(is_tree && !is_order_section);

    ui->actionMarkAll->setEnabled(is_table_fipt);
    ui->actionMarkNone->setEnabled(is_table_fipt);
    ui->actionMarkToggle->setEnabled(is_table_fipt);

    ui->actionJumpEntry->setEnabled(is_double_entry && is_table_fipt);

    ui->actionStatement->setEnabled(is_order_section);
    ui->actionSettlement->setEnabled(is_order_section);
    ui->actionNewBranch->setEnabled(is_order_section);

    ui->actionAppendEntry->setEnabled(is_table_fipt || is_table_o || is_settlement);
    ui->actionDelete->setEnabled(is_tree || is_table_fipt || is_table_o || is_settlement);
}

void MainWindow::tabWidget_tabBarDoubleClicked(int index)
{
    qInfo() << "[UI]" << "tabWidget_tabBarDoubleClicked";

    auto* tab_bar { sc_->tab_widget->tabBar() };
    const auto id { tab_bar->tabData(index).toUuid() };

    if (sc_->widget_hash.contains(id))
        RNodeLocation(start_, id);
}

void MainWindow::tabWidget_tabCloseRequestedFIT(int index)
{
    qInfo() << "[UI]" << "tabWidget_tabCloseRequested";

    const auto node_id { sc_->tab_widget->tabBar()->tabData(index).toUuid() };

    const auto& wc { sc_->widget_hash.value(node_id) };
    Q_ASSERT(wc.widget);

    Utils::CloseWidget(node_id, sc_->widget_hash);
    TableSStation::Instance()->DeregisterModel(node_id);
}

void MainWindow::tabWidget_tabCloseRequestedP(int index)
{
    qInfo() << "[UI]" << "tabWidget_tabCloseRequestedP";

    const auto node_id { sc_->tab_widget->tabBar()->tabData(index).toUuid() };

    const auto& wc { sc_->widget_hash.value(node_id) };
    Q_ASSERT(wc.widget);

    Utils::CloseWidget(node_id, sc_->widget_hash);
}

void MainWindow::tabWidget_tabCloseRequestedO(int index)
{
    qInfo() << "[UI]" << "tabWidget_tabCloseRequestedO";

    const auto node_id { sc_->tab_widget->tabBar()->tabData(index).toUuid() };

    const auto& wc { sc_->widget_hash.value(node_id) };
    Q_ASSERT(wc.widget);

    auto* widget { qobject_cast<TableWidgetO*>(wc.widget.data()) };
    Q_ASSERT(wc.widget);

    if (widget->HasPendingUpdate()) {
        auto* dlg
            = Utils::CreateMessageBox(QMessageBox::Warning, tr("Unsaved Data"), tr("This page contains unsaved data.\n\nDo you want to save before closing?"),
                true, QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);

        dlg->setDefaultButton(QMessageBox::Cancel);

        const int ret { dlg->exec() };

        if (ret == QMessageBox::Cancel)
            return;

        if (ret == QMessageBox::Save)
            widget->SaveOrder();
    }

    Utils::CloseWidget(node_id, sc_->widget_hash);
}

void MainWindow::on_actionJumpEntry_triggered()
{
    qInfo() << "[UI]" << "on_actionJumpEntry_triggered";

    if (IsSingleEntry(start_))
        return;

    auto* widget { sc_->tab_widget->currentWidget() };

    Q_ASSERT(qobject_cast<TableWidget*>(widget));
    auto* leaf_widget { static_cast<TableWidget*>(widget) };

    const auto index { leaf_widget->View()->currentIndex() };
    if (!index.isValid())
        return;

    const int row { index.row() };
    const int rhs_node_column { Utils::LinkedNodeColumn(start_) };

    const auto rhs_node_id { index.sibling(row, rhs_node_column).data().toUuid() };
    if (rhs_node_id.isNull())
        return;

    const auto entry_id { index.sibling(row, std::to_underlying(EntryEnum::kId)).data().toUuid() };
    ShowLeafWidget(rhs_node_id, entry_id);
}

void MainWindow::RUpdateName(const QUuid& node_id, const QString& name, bool branch)
{
    auto model { sc_->tree_model };
    auto widget { sc_->tab_widget };

    if (!model || !widget) {
        return;
    }

    QSet<QUuid> nodes;
    if (branch) {
        if (auto* node = model->GetNode(node_id)) {
            nodes = LeafChildrenId(node);
        }
    } else {
        if (sc_->widget_hash.contains(node_id)) {
            nodes.insert(node_id);
        }
    }

    if (start_ == Section::kPartner) {
        UpdatePartnerReference(sc_sale_, nodes, branch);
        UpdatePartnerReference(sc_purchase_, nodes, branch);
    }

    if (nodes.isEmpty())
        return;

    auto* tab_bar { widget->tabBar() };
    const int count { widget->count() };

    for (int index = 0; index != count; ++index) {
        const auto id { tab_bar->tabData(index).toUuid() };

        if (nodes.contains(id)) {
            const auto path { model->Path(id) };

            if (!branch) {
                tab_bar->setTabText(index, name);
            }

            tab_bar->setTabToolTip(index, path);
        }
    }
}

void MainWindow::UpdatePartnerReference(const SectionContext& sc, const QSet<QUuid>& partner_nodes, const bool branch) const
{
    auto widget { sc.tab_widget };
    auto partner_model { sc_p_.tree_model };
    auto* order_model { static_cast<TreeModelO*>(sc.tree_model.data()) };

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
