#include <QMessageBox>

#include "dialog/editnodenameo.h"
#include "dialog/insertnode/insertnodebranch.h"
#include "global/nodepool.h"
#include "global/tablesstation.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

void MainWindow::EditNameO()
{
    const auto index { sc_->tree_view->currentIndex() };
    if (!index.isValid())
        return;

    auto* node { static_cast<Node*>(index.internalPointer()) };
    if (node->kind == NodeKind::kLeaf)
        return;

    auto model { sc_->tree_model };

    auto* edit_name { new EditNodeNameO(node->name, this) };
    connect(edit_name, &QDialog::accepted, this, [=, this]() {
        const auto message { JsonGen::NodeName(start_, node->id, edit_name->GetName()) };
        WebSocket::Instance()->SendMessage(kNodeName, message);
    });
    edit_name->exec();
}

void MainWindow::on_actionNewGroup_triggered()
{
    qInfo() << "[UI]" << "on_actionNewGroup_triggered";

    if (start_ != Section::kSale && start_ != Section::kPurchase)
        return;

    auto current_index { sc_->tree_view->currentIndex() };
    current_index = current_index.isValid() ? current_index : QModelIndex();

    auto parent_index { current_index.parent() };
    parent_index = parent_index.isValid() ? parent_index : QModelIndex();

    auto tree_model { sc_->tree_model };
    auto unit_model { sc_->info.unit_model };

    auto* parent_node { tree_model->GetNodeByIndex(parent_index) };

    const QUuid parent_id { parent_node->id };

    auto* node { NodePool::Instance().Allocate(start_) };

    node->id = QUuid::createUuidV7();
    node->unit = parent_node->unit;
    node->kind = NodeKind::kBranch;
    node->parent = parent_node;

    static_cast<NodeO*>(node)->issued_time = QDateTime::currentDateTimeUtc();

    auto parent_path { tree_model->Path(parent_id) };
    if (!parent_path.isEmpty())
        parent_path += app_config_.separator;

    const auto children_name { ChildrenName(parent_node) };

    QDialog* dialog { new InsertNodeBranch(node, unit_model, parent_path, children_name, this) };

    connect(dialog, &QDialog::accepted, this, [=, this]() {
        const auto message { JsonGen::NodeInsert(start_, node, node->parent->id) };
        WebSocket::Instance()->SendMessage(kNodeInsert, message);
    });

    connect(dialog, &QDialog::finished, this, [=, this]() { NodePool::Instance().Recycle(node, start_); });
    dialog->exec();
}

void MainWindow::ROrderReleased(Section section, const QUuid& node_id, int version)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->table_wgt_hash.value(node_id, nullptr) };
    if (widget) {
        auto* ptr { widget.data() };

        Q_ASSERT(qobject_cast<TableWidgetO*>(ptr));
        auto* d_widget { static_cast<TableWidgetO*>(ptr) };

        d_widget->ReleaseSucceeded(version);
    }
}

void MainWindow::ROrderRecalled(Section section, const QUuid& node_id, int version)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->table_wgt_hash.value(node_id, nullptr) };
    if (widget) {
        auto* ptr { widget.data() };

        Q_ASSERT(qobject_cast<TableWidgetO*>(ptr));
        auto* d_widget { static_cast<TableWidgetO*>(ptr) };

        d_widget->RecallSucceeded(version);
    }
}

void MainWindow::ROrderSaved(Section section, const QUuid& node_id, int version)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->table_wgt_hash.value(node_id, nullptr) };
    if (widget) {
        auto* ptr { widget.data() };

        Q_ASSERT(qobject_cast<TableWidgetO*>(ptr));
        auto* d_widget { static_cast<TableWidgetO*>(ptr) };

        d_widget->SaveSucceeded(version);
    }
}

void MainWindow::RInvalidOperation()
{
    QMessageBox::information(
        this, tr("Invalid Operation"), tr("The operation you attempted is invalid because your local data is outdated. Please refresh and try again."));
}

void MainWindow::RNodeSelected(Section section, const QUuid& node_id)
{
    auto* sc { GetSectionContex(section) };
    auto tree_model { sc->tree_model };

    const auto index { tree_model->GetIndex(node_id) };
    if (!index.isValid())
        return;

    sc->tree_view->setCurrentIndex(index);
}

void MainWindow::InsertNodeO(const QModelIndex& parent_index)
{
    // Extract frequently used shortcuts
    auto& section_config = sc_->section_config;
    auto* tab_widget = ui->tabWidget;
    auto* tab_bar = tab_widget->tabBar();
    auto* parent_node { sc_->tree_model->GetNodeByIndex(parent_index) };

    NodeO node {};
    node.id = QUuid::createUuidV7();
    node.direction_rule = parent_node->direction_rule;
    node.unit = parent_index.isValid() ? parent_node->unit : NodeUnit(sc_->shared_config.default_unit);
    node.parent = parent_node;
    node.issued_time = QDateTime::currentDateTimeUtc();
    node.code = Utils::UuidToShortCode(node.id);

    const QUuid node_id { node.id };

    // Prepare dependencies
    auto tree_model_p { sc_p_.tree_model };
    auto tree_model_i { sc_i_.tree_model };

    // Create model and widget
    TableModelArg table_model_arg { sc_->info, node_id, true };
    auto* table_model { new TableModelO(table_model_arg, tree_model_i, sc_p_.entry_hub, this) };

    OrderWidgetArg order_arg {
        table_model,
        tree_model_p,
        tree_model_i,
        app_config_,
        section_config,
        start_,
    };
    auto* widget { new TableWidgetO(order_arg, node, SyncState::kLocalOnly, this) };

    // Setup tab
    const int tab_index { tab_widget->addTab(widget, QString()) };
    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, node_id }));

    // Configure view
    auto* view { widget->View() };
    TableConnectO(view, table_model, widget);
    SetTableView(view, sc_->info.section, std::to_underlying(EntryEnumO::kDescription), std::to_underlying(EntryEnumO::kLhsNode));
    TableDelegateO(view, section_config);

    sc_->table_wgt_hash.insert(node_id, widget);
    FocusTableWidget(node_id);
}

void MainWindow::CreateLeafO(SectionContext* sc, const QUuid& node_id)
{
    // Extract frequently used shortcuts
    auto& section_config = sc->section_config;
    auto* tab_widget = ui->tabWidget;
    auto* tab_bar = tab_widget->tabBar();

    // Validate node
    auto* node { static_cast<NodeO*>(sc_->tree_model->GetNode(node_id)) };
    if (!node) {
        return;
    }

    const auto partner_id { node->partner_id };
    Q_ASSERT(!partner_id.isNull());

    // Prepare dependencies
    auto tree_model_p { sc_p_.tree_model };
    auto tree_model_i { sc_i_.tree_model };

    // Create model and widget
    TableModelArg table_model_arg { sc->info, node_id, node->direction_rule };
    auto* table_model = new TableModelO(table_model_arg, tree_model_i, sc_p_.entry_hub, nullptr);

    OrderWidgetArg order_arg {
        table_model,
        tree_model_p,
        tree_model_i,
        app_config_,
        section_config,
        start_,
    };
    auto* widget = new TableWidgetO(order_arg, *node, SyncState::kSynced, this);

    // Setup tab
    const int tab_index { tab_widget->addTab(widget, tree_model_p->Name(partner_id)) };
    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, node_id }));

    // Configure view
    auto* view = widget->View();
    SetTableView(view, sc->info.section, std::to_underlying(EntryEnumO::kDescription), std::to_underlying(EntryEnumO::kLhsNode));
    TableConnectO(view, table_model, widget);
    TableDelegateO(view, section_config);

    sc->table_wgt_hash.insert(node_id, widget);
    TableSStation::Instance()->RegisterModel(node_id, table_model);
}

void MainWindow::RSyncPartner(const QUuid& node_id, const QUuid& value)
{
    auto model { sc_p_.tree_model };
    auto* widget { ui->tabWidget };
    auto* tab_bar { widget->tabBar() };
    int count { widget->count() };

    for (int index = 0; index != count; ++index) {
        if (widget->isTabVisible(index) && tab_bar->tabData(index).value<TabInfo>().id == node_id) {
            tab_bar->setTabText(index, model->Name(value));
        }
    }
}
