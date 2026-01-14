#include <QtConcurrent/qtconcurrentrun.h>
#include <QtWidgets/qmessagebox.h>

#include <QFutureWatcher>

#include "mainwindow.h"
#include "tree/model/treemodelo.h"
#include "ui_mainwindow.h"
#include "utils/entryutils.h"
#include "utils/templateutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

void MainWindow::SetTabWidget()
{
    auto* tab_widget { ui->tabWidget };
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
        const int expected_column { IsOrderSection(start_) ? std::to_underlying(NodeEnumO::kPartner) : std::to_underlying(NodeEnum::kName) };

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
        if (start_ == Section::kInventory && unit == std::to_underlying(UnitI::kExternal))
            return;
    }

    {
        const auto node_id { index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() };
        assert(!node_id.isNull());

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
        auto& table_wgt_hash { sc_->table_wgt_hash };

        if (table_wgt_hash.contains(node_id)) {
            FocusTableWidget(node_id);
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

        FocusTableWidget(node_id);
    }
}

void MainWindow::on_tabWidget_currentChanged(int /*index*/)
{
    qDebug() << "[UI]" << "on_tabWidget_currentChanged";

    if (!section_settings_)
        return;

    auto* widget { ui->tabWidget->currentWidget() };
    if (!widget)
        return;

    const bool is_tree { IsTreeWidget(widget) };
    const bool is_table_fipt { IsTableWidgetFIPT(widget) };
    const bool is_table_o { IsTableWidgetO(widget) };
    const bool is_settlement { IsTreeWidgetSettlement(widget) };
    const bool is_color_section { start_ == Section::kTask || start_ == Section::kInventory };
    const bool is_order_section { IsOrderSection(start_) };

    ui->actionAppendNode->setEnabled(is_tree);
    ui->actionInsertNode->setEnabled(is_tree || is_table_o);

    ui->actionEditName->setEnabled(is_tree);
    ui->actionResetColor->setEnabled(is_tree && is_color_section);

    ui->actionMarkAll->setEnabled(is_table_fipt);
    ui->actionMarkNone->setEnabled(is_table_fipt);
    ui->actionMarkToggle->setEnabled(is_table_fipt);
    ui->actionJump->setEnabled(is_table_fipt);

    ui->actionStatement->setEnabled(is_order_section);
    ui->actionSettlement->setEnabled(is_order_section);
    ui->actionNewGroup->setEnabled(is_order_section);

    ui->actionAppendEntry->setEnabled(is_table_fipt || is_table_o || is_settlement);
    ui->actionRemove->setEnabled(is_tree || is_table_fipt || is_table_o || is_settlement);
}

void MainWindow::on_tabWidget_tabBarDoubleClicked(int index)
{
    qInfo() << "[UI]" << "on_tabWidget_tabBarDoubleClicked";

    auto* tab_bar { ui->tabWidget->tabBar() };
    const auto tab_info { tab_bar->tabData(index).value<TabInfo>() };
    const QUuid id { tab_info.id };

    if (sc_->table_wgt_hash.contains(id))
        RNodeLocation(start_, id);
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    qInfo() << "[UI]" << "on_tabWidget_tabCloseRequested";

    if (index == 0)
        return;

    const auto node_id { ui->tabWidget->tabBar()->tabData(index).value<TabInfo>().id };

    {
        if (sc_->widget_hash.contains(node_id)) {
            Utils::CloseWidget(node_id, sc_->widget_hash);
            return;
        }
    }

    {
        if (IsOrderSection(start_)) {
            auto* widget { static_cast<TableWidgetO*>(sc_->table_wgt_hash.value(node_id).data()) };

            if (!widget)
                return;

            if (widget->IsDirty()) {
                auto ret = QMessageBox::warning(this, tr("Unsaved Data"), tr("This page contains unsaved data.\n\nDo you want to save before closing?"),
                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);

                if (ret == QMessageBox::Save) {
                    widget->SaveOrder();
                } else if (ret == QMessageBox::Cancel) {
                    return;
                }
            }
        }
    }

    {
        RFreeWidget(start_, node_id);
    }
}

void MainWindow::on_actionJump_triggered()
{
    qInfo() << "[UI]" << "on_actionJump_triggered";

    if (IsSingleEntry(start_))
        return;

    auto* leaf_widget { dynamic_cast<TableWidget*>(ui->tabWidget->currentWidget()) };
    if (!leaf_widget)
        return;

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
    auto* widget { ui->tabWidget };

    auto* tab_bar { widget->tabBar() };
    int count { widget->count() };

    QSet<QUuid> nodes;

    if (branch) {
        auto* node { model->GetNode(node_id) };
        nodes = LeafChildrenId(node);
    } else {
        nodes.insert(node_id);

        if (start_ == Section::kPartner)
            UpdatePartnerReference(nodes, branch);

        if (!sc_->table_wgt_hash.contains(node_id))
            return;
    }

    for (int index = 0; index != count; ++index) {
        const auto id { tab_bar->tabData(index).value<TabInfo>().id };

        if (widget->isTabVisible(index) && nodes.contains(id)) {
            const auto path { model->Path(id) };

            if (!branch) {
                tab_bar->setTabText(index, name);
            }

            tab_bar->setTabToolTip(index, path);
        }
    }

    if (start_ == Section::kPartner)
        UpdatePartnerReference(nodes, branch);
}

void MainWindow::UpdatePartnerReference(const QSet<QUuid>& partner_nodes, bool branch) const
{
    auto* widget { ui->tabWidget };
    auto partner_model { sc_->tree_model };
    auto* order_model { static_cast<TreeModelO*>(sc_sale_.tree_model.data()) };
    auto* tab_bar { widget->tabBar() };
    const int count { widget->count() };

    // 使用 QtConcurrent::run 启动后台线程
    auto future = QtConcurrent::run([=]() -> QVector<std::tuple<int, QString, QString>> {
        QVector<std::tuple<int, QString, QString>> updates;

        // 遍历所有选项卡，计算需要更新的项
        for (int index = 0; index != count; ++index) {
            const auto& data { tab_bar->tabData(index).value<TabInfo>() };
            bool update { data.section == Section::kSale || data.section == Section::kPurchase };

            if (!widget->isTabVisible(index) && update) {
                const auto order_node_id { data.id };
                if (order_node_id.isNull())
                    continue;

                const auto order_partner { order_model->Partner(order_node_id) };
                if (!partner_nodes.contains(order_partner))
                    continue;

                QString name { partner_model->Name(order_partner) };
                QString path { partner_model->Path(order_partner) };

                // 收集需要更新的信息
                updates.append(std::make_tuple(index, name, path));
            }
        }

        return updates;
    });

    auto watcher { std::make_unique<QFutureWatcher<QVector<std::tuple<int, QString, QString>>>>() };

    // 获取原始指针用于信号连接
    auto* raw_watcher = watcher.get();

    connect(
        raw_watcher, &QFutureWatcher<QVector<std::tuple<int, QString, QString>>>::finished, this, [watcher = std::move(watcher), tab_bar, branch]() mutable {
            const auto& updates = watcher->result();

            for (const auto& [index, name, path] : updates) {
                if (!branch)
                    tab_bar->setTabText(index, name);
                tab_bar->setTabToolTip(index, path);
            }
        });

    raw_watcher->setFuture(future);
}
