#include "billing/settlement/treewidgetsettlement.h"
#include "global/tablesstation.h"
#include "mainwindow.h"
#include "table/model/tablemodelf.h"
#include "table/model/tablemodeli.h"
#include "table/model/tablemodelp.h"
#include "table/model/tablemodelt.h"
#include "ui_mainwindow.h"
#include "utils/entryutils.h"
#include "utils/templateutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

void MainWindow::on_actionAppendEntry_triggered()
{
    qInfo() << "[UI]" << "on_actionAppendEntry_triggered";

    {
        auto* widget { qobject_cast<TreeWidgetSettlement*>(ui->tabWidget->currentWidget()) };
        if (widget) {
            const QUuid settlement_widget_id { widget->WidgetId() };

            Settlement settlement {};

            settlement.issued_time = QDateTime::currentDateTimeUtc();
            settlement.id = QUuid::createUuidV7();

            SettlementItemTab(settlement_widget_id, settlement, SyncState::kLocalOnly);
            return;
        }
    }

    {
        auto* widget { qobject_cast<TableWidget*>(ui->tabWidget->currentWidget()) };
        if (widget) {
            auto* model { widget->Model() };
            auto* view { widget->View() };
            const auto current_idx { view->currentIndex() };

            const int new_row { current_idx.isValid() ? current_idx.row() + 1 : model->rowCount() };
            if (!model->insertRows(new_row, 1))
                return;

            const int linked_node_col { Utils::LinkedNodeColumn(start_) };
            const QModelIndex target_index { model->index(new_row, linked_node_col) };

            if (target_index.isValid()) {
                view->setCurrentIndex(target_index);
                view->scrollTo(target_index, QAbstractItemView::PositionAtCenter);
            }
        }
    }
}

void MainWindow::RemoveEntry(TableWidget* widget)
{
    auto* view { widget->View() };

    if (!Utils::HasSelection(view))
        return;

    const QModelIndex current_index { view->currentIndex() };
    if (!current_index.isValid())
        return;

    auto* model { widget->Model() };

    const int current_row { current_index.row() };
    if (!model->removeRows(current_row, 1)) {
        qDebug() << "Failed to remove row:" << current_row;
        return;
    }

    const int new_row_count { model->rowCount() };
    if (new_row_count == 0)
        return;

    QModelIndex new_index {};
    if (current_row < new_row_count) {
        new_index = model->index(current_row, 0);
    } else {
        new_index = model->index(new_row_count - 1, 0);
    }

    if (new_index.isValid()) {
        view->setCurrentIndex(new_index);
        view->selectionModel()->select(new_index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        view->closePersistentEditor(new_index);
    }
}

void MainWindow::REntryLocation(const QUuid& entry_id, const QUuid& lhs_node_id, const QUuid& rhs_node_id)
{
    // Note: For Partner, Sale, and Purchase sections, rhs_node_id check is not required
    if (entry_id.isNull() || lhs_node_id.isNull())
        return;

    QUuid node_id {};
    auto& leaf_wgt_hash { sc_->table_wgt_hash };

    if (leaf_wgt_hash.contains(lhs_node_id)) {
        node_id = lhs_node_id;
    } else if (leaf_wgt_hash.contains(rhs_node_id)) {
        node_id = rhs_node_id;
    }

    if (!node_id.isNull()) {
        FocusTableWidget(node_id);
        RSelectLeafEntry(node_id, entry_id);
        return;
    }

    node_id = lhs_node_id;

    if (IsOrderSection(start_)) {
        auto& tree_model { sc_->tree_model };

        if (!tree_model->Contains(lhs_node_id)) {
            tree_model->AckNode(node_id);
        }
    }

    const auto message { JsonGen::LeafEntry(sc_->info.section, node_id, entry_id) };
    WebSocket::Instance()->SendMessage(kTableAcked, message);

    switch (start_) {
    case Section::kSale:
    case Section::kPurchase:
        CreateLeafO(sc_, node_id);
        break;
    case Section::kTask:
    case Section::kFinance:
    case Section::kInventory:
    case Section::kPartner:
        CreateLeafFIPT(sc_, node_id);
        break;
    default:
        break;
    }

    FocusTableWidget(node_id);
}

// RSelectLeafEntry - Scroll to and select the specified entry (slot)
// -------------------------------
// Behavior:
//  1. Skip if entry_id is null (no target to scroll to)
//  2. Find and select the entry in the view
//  3. Scroll to center the IssuedTime column
//  4. Close any persistent editor on that row
void MainWindow::RSelectLeafEntry(const QUuid& node_id, const QUuid& entry_id)
{
    if (entry_id.isNull() || node_id.isNull())
        return;

    auto widget { sc_->table_wgt_hash.value(node_id) };
    Q_ASSERT(widget);

    auto* view { widget->View() };
    auto index { widget->Model()->GetIndex(entry_id) };

    if (!index.isValid())
        return;

    view->setCurrentIndex(index);
    view->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    view->scrollTo(index.siblingAtColumn(std::to_underlying(EntryEnum::kIssuedTime)), QAbstractItemView::PositionAtCenter);
    view->closePersistentEditor(index);
}

void MainWindow::CreateLeafFIPT(SectionContext* sc, CUuid& node_id)
{
    auto tree_model { sc->tree_model };
    const auto& info = sc->info;
    const auto& section_config = sc->section_config;

    const Section section { info.section };

    // The model is parented to the LeafWidgetFIPT inside its constructor.
    // When the LeafWidget is destroyed, it will automatically delete the model.
    TableModel* table_model {};

    {
        const bool rule { tree_model->Rule(node_id) };
        TableModelArg arg { info, node_id, rule };

        switch (section) {
        case Section::kFinance:
            table_model = new TableModelF(arg, nullptr);
            break;
        case Section::kInventory:
            table_model = new TableModelI(arg, nullptr);
            break;
        case Section::kTask:
            table_model = new TableModelT(arg, static_cast<TreeModelT*>(tree_model.data()), nullptr);
            break;
        case Section::kPartner:
            table_model = new TableModelP(arg, nullptr);
            break;
        case Section::kSale:
        case Section::kPurchase:
            return;
        }
    }

    TableSStation::Instance()->RegisterModel(node_id, table_model);

    TableWidgetFIPT* widget { new TableWidgetFIPT(table_model, this) };

    {
        CString name { tree_model->Name(node_id) };
        const int tab_index { ui->tabWidget->addTab(widget, name) };
        auto* tab_bar { ui->tabWidget->tabBar() };

        tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { section, node_id }));
        tab_bar->setTabToolTip(tab_index, tree_model->Path(node_id));

        auto* view { widget->View() };

        const int description_column { Utils::EntryDescriptionColumn(section) };
        SetTableView(view, section, description_column, std::to_underlying(EntryEnum::kLhsNode));

        switch (section) {
        case Section::kFinance:
            TableDelegateF(view, tree_model, section_config, node_id);
            TableConnectF(view, table_model);
            break;
        case Section::kInventory:
            TableDelegateI(view, tree_model, section_config, node_id);
            TableConnectI(view, table_model);
            break;
        case Section::kTask:
            TableDelegateT(view, tree_model, section_config, node_id);
            TableConnectT(view, table_model);
            break;
        case Section::kPartner:
            TableDelegateP(view, section_config);
            TableConnectP(view, table_model);
            static_cast<EntryHubP*>(sc_->entry_hub.data())->PushEntry(node_id);
            break;
        case Section::kSale:
        case Section::kPurchase:
            return;
        }
    }

    sc->table_wgt_hash.insert(node_id, widget);
}
