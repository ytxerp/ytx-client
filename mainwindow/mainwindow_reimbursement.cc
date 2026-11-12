#include "global/nodepool.h"
#include "global/tablesstation.h"
#include "global/userprofile.h"
#include "mainwindow.h"
#include "table/model/tablemodelr.h"
#include "table/widget/tablewidgetr.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

void MainWindow::InsertNodeR(SectionContext* sc, const QModelIndex& parent_index)
{
    // Extract frequently used shortcuts
    const auto& section_config { sc->section_config };
    const auto& info { sc->info };
    const Section section { sc->section };
    const int permissions { UserProfile::Instance().SectionPermissions(section) };

    auto tab_widget { sc->tab_widget };
    auto* tab_bar { tab_widget->tabBar() };
    auto* parent_node { sc->tree_model->GetNodeByIndex(parent_index) };

    NodeR* node { static_cast<NodeR*>(NodePool::Instance().Allocate(section)) };
    node->id = QUuid::createUuidV7();
    node->unit = NodeUnit::RDefault;
    node->parent = parent_node;
    node->issued_time = QDateTime::currentDateTime();

    const QUuid node_id { node->id };

    // Create model and widget
    TableModelArg table_model_arg { section, node_id, info.entry_header, node->direction_rule };
    auto* table_model { new TableModelR(table_model_arg, node, permissions, this) };

    auto* widget { new TableWidgetR(table_model, node, section_config, permissions, this) };

    // Setup tab
    const QString name { node->sync_state == SyncState::kSynced ? node->name : UserProfile::Instance().Name() };
    const int tab_index { tab_widget->addTab(widget, name) };
    tab_bar->setTabData(tab_index, node_id);
    tab_widget->setCurrentIndex(tab_index);

    // Configure view
    auto* view { widget->View() };
    TableConnectR(table_model, widget);
    SetTableView(view, section, std::to_underlying(EntryEnumR::kDescription));
    TableDelegateR(view, section_config);

    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view, &QWidget::customContextMenuRequested, this, &MainWindow::RTableViewCustomContextMenuRequested);

    RegisterWidget(sc, widget, node_id, WidgetRole::kNodeTabOR);
    TableSStation::Instance()->RegisterModel(node_id, table_model);
}

void MainWindow::CreateLeafR(SectionContext* sc, QUuid node_id)
{
    Q_ASSERT(sc);

    // Extract frequently used shortcuts
    const auto& section_config { sc->section_config };
    const auto& info { sc->info };

    auto* tab_widget { sc->tab_widget.data() };
    auto* tab_bar { tab_widget->tabBar() };

    // Validate node
    auto* node { static_cast<NodeR*>(sc->tree_model->GetNode(node_id)) };
    if (!node) {
        return;
    }

    const Section section { sc->section };
    const int permissions { UserProfile::Instance().SectionPermissions(section) };

    NodeR* tmp_node { static_cast<NodeR*>(NodePool::Instance().Allocate(section)) };
    *tmp_node = *node;

    // Create model and widget
    TableModelArg table_model_arg { section, node_id, info.entry_header, tmp_node->direction_rule };
    auto* table_model { new TableModelR(table_model_arg, tmp_node, permissions, this) };

    auto* widget { new TableWidgetR(table_model, tmp_node, section_config, permissions, this) };

    // Setup tab
    const QString name { tmp_node->sync_state == SyncState::kSynced ? tmp_node->name : UserProfile::Instance().Name() };
    const int tab_index { tab_widget->addTab(widget, name) };
    tab_bar->setTabData(tab_index, node_id);

    // Configure view
    auto* view { widget->View() };
    TableConnectR(table_model, widget);
    SetTableView(view, section, std::to_underlying(EntryEnumR::kDescription));
    TableDelegateR(view, section_config);

    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view, &QWidget::customContextMenuRequested, this, &MainWindow::RTableViewCustomContextMenuRequested);

    RegisterWidget(sc, widget, node_id, WidgetRole::kNodeTabOR);
    TableSStation::Instance()->RegisterModel(node_id, table_model);
}

void MainWindow::EntryLocateR(Section section, QUuid entry_id, QUuid lhs_node_id)
{
    // Only lhs_node_id is needed for Sale and Purchase sections.
    if (entry_id.isNull() || lhs_node_id.isNull())
        return;

    auto* sc { GetSectionContext(section) };

    if (sc->widget_hash.contains(lhs_node_id)) {
        LocateLeafWidget(sc, lhs_node_id, entry_id);
        return;
    }

    auto* tree_model { sc->tree_model.data() };
    if (!tree_model->Contains(lhs_node_id))
        tree_model->AckNode(lhs_node_id);

    CreateLeafR(sc, lhs_node_id);

    const auto message { JsonGen::TableFetch(section, lhs_node_id, entry_id) };
    WebSocket::Instance()->SendMessage(WsKey::kTableFetch, message);
}

void MainWindow::RReimbursementSync(Section section, QUuid node_id, int version)
{
    auto* sc { GetSectionContext(section) };

    auto widget { sc->widget_hash.value(node_id).widget };
    if (widget) {
        auto* ptr { widget.data() };

        Q_ASSERT(qobject_cast<TableWidgetR*>(ptr));
        auto* d_widget { static_cast<TableWidgetR*>(ptr) };

        d_widget->SyncSucceeded(version);
    }
}