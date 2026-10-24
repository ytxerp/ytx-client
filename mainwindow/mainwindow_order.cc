#include "component/constantwebsocket.h"
#include "dialog/editnodenameo.h"
#include "dialog/insertnode/insertnodebranch.h"
#include "global/nodepool.h"
#include "global/tablesstation.h"
#include "mainwindow.h"
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
    const Section section { sc_->info.section };

    auto* dialog { new EditNodeNameO(node->name, nullptr) };

    utils::ManageDialog(sc_->widget_hash, dialog);
    dialog->setModal(true);

    connect(dialog, &QDialog::accepted, this, [dialog, node, section]() {
        const auto message { JsonGen::NodeName(section, node->id, dialog->GetName(), node->version) };
        WebSocket::Instance()->SendMessage(WsKey::kNodeNameUpdate, message);
    });

    dialog->show();
}

void MainWindow::on_actionNewBranch_triggered()
{
    qInfo() << Q_FUNC_INFO;

    const Section section { sc_->info.section };
    if (section != Section::kSale && section != Section::kPurchase)
        return;

    auto current_index { sc_->tree_view->currentIndex() };
    current_index = current_index.isValid() ? current_index : QModelIndex();

    auto parent_index { current_index.parent() };
    parent_index = parent_index.isValid() ? parent_index : QModelIndex();

    auto tree_model { sc_->tree_model };
    auto unit_model { sc_->info.unit_model };

    auto* parent_node { tree_model->GetNodeByIndex(parent_index) };

    const QUuid parent_id { parent_node->id };

    auto* node { NodePool::Instance().Allocate(section) };
    auto* d_node { static_cast<NodeO*>(node) };

    node->id = QUuid::createUuidV7();
    node->unit = parent_index.isValid() ? parent_node->unit : NodeUnit(sc_->shared_config.default_unit);
    node->kind = NodeKind::kBranch;
    d_node->order_status = OrderStatus::kUnreleased; // Branch nodes are always kUnreleased
    node->parent = parent_node;

    static_cast<NodeO*>(node)->issued_time = QDateTime::currentDateTime();

    auto parent_path { tree_model->Path(parent_id) };
    if (!parent_path.isEmpty())
        parent_path += app_config_.separator;

    const auto children_name { ChildrenName(parent_node) };

    auto* dialog { new InsertNodeBranch(node, unit_model, parent_path, children_name, nullptr) };

    utils::ManageDialog(sc_->widget_hash, dialog);
    dialog->setModal(true);

    connect(dialog, &QDialog::accepted, this, [node, section]() {
        const auto message { JsonGen::NodeInsert(section, node, node->parent->id) };
        WebSocket::Instance()->SendMessage(WsKey::kNodeInsert, message);
    });

    connect(dialog, &QDialog::destroyed, this, [node, section]() { NodePool::Instance().Recycle(node, section); });

    dialog->show();
}

void MainWindow::RReleaseOrder(Section section, QUuid node_id, int version)
{
    auto* sc { GetSectionContext(section) };

    auto widget { sc->widget_hash.value(node_id).widget };
    if (widget) {
        auto* ptr { widget.data() };

        Q_ASSERT(qobject_cast<TableWidgetO*>(ptr));
        auto* d_widget { static_cast<TableWidgetO*>(ptr) };

        d_widget->ReleaseSucceeded(version);
    }
}

void MainWindow::RRecallOrder(Section section, QUuid node_id, int version)
{
    auto* sc { GetSectionContext(section) };

    auto widget { sc->widget_hash.value(node_id).widget };
    if (widget) {
        auto* ptr { widget.data() };

        Q_ASSERT(qobject_cast<TableWidgetO*>(ptr));
        auto* d_widget { static_cast<TableWidgetO*>(ptr) };

        d_widget->RecallSucceeded(version);
    }
}

void MainWindow::RSaveOrder(Section section, QUuid node_id, int version)
{
    auto* sc { GetSectionContext(section) };

    auto widget { sc->widget_hash.value(node_id).widget };
    if (widget) {
        auto* ptr { widget.data() };

        Q_ASSERT(qobject_cast<TableWidgetO*>(ptr));
        auto* d_widget { static_cast<TableWidgetO*>(ptr) };

        d_widget->SaveSucceeded(version);
    }
}

void MainWindow::RDenyOperation()
{
    utils::ShowMessage(QMessageBox::Information, tr("Invalid Operation"),
        tr("The operation you attempted is invalid because your local data is outdated. Please refresh and try again."), time_const::kAutoCloseMs);
}

void MainWindow::RPermissionDeny()
{
    utils::ShowMessage(
        QMessageBox::Information, tr("Permission Denied"), tr("You do not have permission to perform this operation."), time_const::kAutoCloseMs);
}

void MainWindow::ROrderPermissionDeny(Section section, QUuid widget_id)
{
    auto* sc { GetSectionContext(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    Q_ASSERT(qobject_cast<TableWidgetO*>(widget.data()));
    auto* d_widget { static_cast<TableWidgetO*>(widget.data()) };

    d_widget->PermissionDenied();

    RPermissionDeny();
}

void MainWindow::InsertNodeO(SectionContext* sc, const QModelIndex& parent_index)
{
    // Extract frequently used shortcuts
    const auto& section_config { sc->section_config };
    const auto& info { sc->info };
    const Section section { info.section };

    auto tab_widget { sc->tab_widget };
    auto* tab_bar { tab_widget->tabBar() };
    auto* parent_node { sc->tree_model->GetNodeByIndex(parent_index) };

    NodeO* node { static_cast<NodeO*>(NodePool::Instance().Allocate(section)) };
    node->id = QUuid::createUuidV7();
    node->direction_rule = parent_node->direction_rule;
    node->unit = parent_index.isValid() ? parent_node->unit : NodeUnit(sc->shared_config.default_unit);
    node->parent = parent_node;
    node->issued_time = QDateTime::currentDateTime();

    const QUuid node_id { node->id };

    // Create model and widget
    TableModelArg table_model_arg { info.entry_header, node_id, node->direction_rule, section };
    auto* table_model { new TableModelO(table_model_arg, node, this) };

    // Prepare dependencies
    auto tree_model_p { sc_p_.tree_model };
    auto* pmodel { tree_model_p->IncludeUnit(section == Section::kSale ? NodeUnit::PCustomer : NodeUnit::PVendor) };
    auto* emodel { tree_model_p->IncludeUnit(NodeUnit::PEmployee) };

    OrderWidgetArg order_arg {
        table_model,
        pmodel,
        emodel,
        section_config,
        section,
    };

    auto* widget { new TableWidgetO(order_arg, node, this) };

    // Setup tab
    const int tab_index { tab_widget->addTab(widget, QString()) };
    tab_bar->setTabData(tab_index, node_id);
    tab_widget->setCurrentIndex(tab_index);

    // Configure view
    auto* view { widget->View() };
    TableConnectO(table_model, widget);
    SetTableView(view, section, std::to_underlying(EntryEnumO::kDescription));
    TableDelegateO(view, info, section_config);

    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view, &QWidget::customContextMenuRequested, this, &MainWindow::RTableViewCustomContextMenuRequested);

    RegisterWidget(sc, widget, node_id, WidgetRole::kNodeTabO);
    TableSStation::Instance()->RegisterModel(node_id, table_model);
}

void MainWindow::CreateLeafO(SectionContext* sc, QUuid node_id)
{
    Q_ASSERT(sc);

    // Extract frequently used shortcuts
    const auto& section_config { sc->section_config };
    const auto& info { sc->info };

    const Section section { info.section };
    Q_ASSERT(IsOrderSection(section));

    auto* tab_widget { sc->tab_widget.data() };
    auto* tab_bar { tab_widget->tabBar() };

    // Validate node
    auto* node { static_cast<NodeO*>(sc->tree_model->GetNode(node_id)) };
    if (!node) {
        return;
    }

    NodeO* tmp_node { static_cast<NodeO*>(NodePool::Instance().Allocate(section)) };
    *tmp_node = *node;

    const auto partner_id { tmp_node->partner_id };
    Q_ASSERT(!partner_id.isNull());

    // Create model and widget
    TableModelArg table_model_arg { sc->info.entry_header, node_id, tmp_node->direction_rule, section };
    auto* table_model { new TableModelO(table_model_arg, tmp_node, this) };

    // Prepare dependencies
    auto tree_model_p { sc_p_.tree_model };
    auto* pmodel { tree_model_p->IncludeUnit(section == Section::kSale ? NodeUnit::PCustomer : NodeUnit::PVendor) };
    auto* emodel { tree_model_p->IncludeUnit(NodeUnit::PEmployee) };

    OrderWidgetArg order_arg {
        table_model,
        pmodel,
        emodel,
        section_config,
        section,
    };
    auto* widget = new TableWidgetO(order_arg, tmp_node, this);

    // Setup tab
    const QString partner_name { tree_model_p->Name(partner_id) };
    const int tab_index { tab_widget->addTab(widget, partner_name) };

    QString path { sc->tree_model->Path(node->parent->id) };

    if (!path.isEmpty())
        path += app_config_.separator;

    path += partner_name;

    tab_bar->setTabToolTip(tab_index, path);
    tab_bar->setTabData(tab_index, node_id);

    // Configure view
    auto* view = widget->View();
    SetTableView(view, sc->info.section, std::to_underlying(EntryEnumO::kDescription));
    TableConnectO(table_model, widget);
    TableDelegateO(view, sc->info, section_config);

    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view, &QWidget::customContextMenuRequested, this, &MainWindow::RTableViewCustomContextMenuRequested);

    RegisterWidget(sc, widget, node_id, WidgetRole::kNodeTabO);
    TableSStation::Instance()->RegisterModel(node_id, table_model);
}

void MainWindow::RSyncPartner(QUuid parent_id, QUuid node_id, QUuid partner_id)
{
    auto widget { sc_->tab_widget };
    auto* tab_bar { widget->tabBar() };
    const int count { widget->count() };

    for (int index = 0; index != count; ++index) {
        if (tab_bar->tabData(index).toUuid() == node_id) {
            const QString partner_name { sc_p_.tree_model->Name(partner_id) };
            const QString parent_path { sc_->tree_model->Path(parent_id) };

            tab_bar->setTabText(index, partner_name);
            tab_bar->setTabToolTip(index, parent_path.isEmpty() ? partner_name : parent_path + app_config_.separator + partner_name);
            break;
        }
    }
}

void MainWindow::EntryLocateO(Section section, QUuid entry_id, QUuid lhs_node_id)
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

    CreateLeafO(sc, lhs_node_id);

    const auto message { JsonGen::TableAck(section, lhs_node_id, entry_id) };
    WebSocket::Instance()->SendMessage(WsKey::kTableAck, message);
}
