#include "component/constantwebsocket.h"
#include "entryhub/entryhubp.h"
#include "global/tablesstation.h"
#include "mainwindow.h"
#include "table/model/tablemodelf.h"
#include "table/model/tablemodeli.h"
#include "table/model/tablemodelp.h"
#include "table/model/tablemodelt.h"
#include "utils/entryutils.h"
#include "utils/mainwindowutils.h"
#include "utils/templateutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

void MainWindow::on_actionAppendEntry_triggered()
{
    qInfo() << Q_FUNC_INFO;

    auto* widget { qobject_cast<TableWidget*>(sc_->tab_widget->currentWidget()) };
    if (!widget)
        return;

    auto* model { widget->Model() };
    auto* view { widget->View() };
    const auto current_idx { view->currentIndex() };

    const int new_row { current_idx.isValid() ? current_idx.row() + 1 : model->rowCount() };
    if (!model->insertRows(new_row, 1))
        return;

    const int issued_time_col { entry::IssuedTimeColumn(sc_->info.section) };
    const QModelIndex target_index { model->index(new_row, issued_time_col) };

    if (!target_index.isValid())
        return;

    view->setCurrentIndex(target_index);
    view->scrollTo(target_index, QAbstractItemView::PositionAtCenter);

    view->edit(target_index);
}

void MainWindow::DeleteEntry(TableWidget* widget)
{
    auto* view { widget->View() };

    if (!utils::HasSelection(view))
        return;

    const QModelIndex current_index { view->currentIndex() };
    if (!current_index.isValid())
        return;

    if (app_config_.delete_confirm) {
        auto* msg_box { utils::CreateMessage(QMessageBox::Warning, tr("Confirm Delete"),
            tr("Are you sure you want to delete this entry?"
               "<br><br>"
               "<span style='color:#d32f2f; font-weight:bold;'>"
               "⚠️ This action is permanent and cannot be undone."
               "</span>"
               "<br><br>"
               "<i>💡 Tip: You can disable this confirmation in Preferences.</i>"),
            true, QMessageBox::Yes | QMessageBox::No, this) };

        msg_box->setDefaultButton(QMessageBox::No);
        if (msg_box->exec() != QMessageBox::Yes)
            return;
    }

    auto* model { widget->Model() };

    const int current_row { current_index.row() };
    model->removeRows(current_row);
}

// Dispatch an entry locate request to the handler matching the section.
void MainWindow::REntryLocate(Section section, QUuid entry_id, QUuid lhs_node_id, QUuid rhs_node_id)
{
    switch (section) {
    case Section::kFinance:
    case Section::kInventory:
    case Section::kTask:
        EntryLocateFIT(section, entry_id, lhs_node_id, rhs_node_id);
        break;
    case Section::kPartner:
        EntryLocateP(section, entry_id, lhs_node_id);
        break;
    case Section::kSale:
    case Section::kPurchase:
        EntryLocateO(section, entry_id, lhs_node_id);
        break;
    }
}

void MainWindow::REntrySelect(Section section, QUuid node_id, QUuid entry_id)
{
    if (entry_id.isNull() || node_id.isNull())
        return;

    auto* sc { GetSectionContext(section) };

    auto* widget { qobject_cast<TableWidget*>(sc->widget_hash.value(node_id).widget) };
    if (!widget)
        return;

    auto* view { widget->View() };
    const auto index { widget->Model()->GetIndex(entry_id) };

    if (!index.isValid())
        return;

    const auto target { index.siblingAtColumn(std::to_underlying(EntryEnum::kIssuedTime)) };

    view->setCurrentIndex(target);
    view->scrollTo(target, QAbstractItemView::PositionAtCenter);
}

void MainWindow::REntryLocked()
{
    utils::ShowMessage(
        QMessageBox::Information, tr("Invalid Operation"), tr("This entry is locked. The operation cannot be performed."), time_const::kAutoCloseMs);
}

void MainWindow::CreateLeafFIPT(SectionContext* sc, QUuid node_id)
{
    Q_ASSERT(sc);

    auto tree_model { sc->tree_model };
    const auto& info = sc->info;
    const auto& section_config = sc->section_config;

    const Section section { info.section };
    Q_ASSERT(!IsOrderSection(section));

    // The model is parented to the LeafWidgetFIPT inside its constructor.
    // When the LeafWidget is destroyed, it will automatically delete the model.
    TableModel* table_model {};

    {
        const bool rule { tree_model->Rule(node_id) };
        const TableModelArg arg { info.entry_header, node_id, rule, section };

        switch (section) {
        case Section::kFinance:
            table_model = new TableModelF(arg, nullptr);
            break;
        case Section::kInventory:
            table_model = new TableModelI(arg, nullptr);
            break;
        case Section::kTask:
            table_model = new TableModelT(arg, nullptr);
            break;
        case Section::kPartner:
            table_model = new TableModelP(arg, nullptr);
            break;
        case Section::kSale:
        case Section::kPurchase:
            Q_UNREACHABLE();
        }
    }

    TableWidgetFIPT* widget { new TableWidgetFIPT(table_model, this) };
    const WidgetRole role { section == Section::kPartner ? WidgetRole::kNodeTabP : WidgetRole::kNodeTabFIT };

    {
        CString name { tree_model->Name(node_id) };
        const int tab_index { sc->tab_widget->addTab(widget, name) };
        auto* tab_bar { sc->tab_widget->tabBar() };

        tab_bar->setTabData(tab_index, node_id);
        tab_bar->setTabToolTip(tab_index, tree_model->Path(node_id));

        auto* view { widget->View() };

        view->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(view, &QWidget::customContextMenuRequested, this, &MainWindow::RTableViewCustomContextMenuRequested);

        const int description_column { entry::DescriptionColumn(section) };
        SetTableView(view, section, description_column);

        switch (section) {
        case Section::kFinance:
            TableDelegateF(view, tree_model, section_config, node_id);
            TableConnectF(table_model);
            break;
        case Section::kInventory:
            TableDelegateI(view, tree_model, section_config, node_id);
            TableConnectI(table_model);
            break;
        case Section::kTask:
            TableDelegateT(view, tree_model, section_config, node_id);
            TableConnectT(table_model);
            break;
        case Section::kPartner:
            TableDelegateP(view, section_config);
            TableConnectP(table_model);
            break;
        case Section::kSale:
        case Section::kPurchase:
            Q_UNREACHABLE();
        }
    }

    RegisterWidget(sc, widget, node_id, role);
    TableSStation::Instance()->RegisterModel(node_id, table_model);
}

void MainWindow::MarkEntries(MarkOperation operation)
{
    auto* current_widget { sc_->tab_widget->currentWidget() };

    Q_ASSERT(qobject_cast<TableWidget*>(current_widget));
    auto* leaf_widget { static_cast<TableWidget*>(current_widget) };

    auto table_model { leaf_widget->Model() };
    table_model->MarkEntries(operation);
}

void MainWindow::on_actionMarkAll_triggered() { MarkEntries(MarkOperation::kSelect); }

void MainWindow::on_actionMarkNone_triggered() { MarkEntries(MarkOperation::kClear); }

void MainWindow::on_actionMarkToggle_triggered() { MarkEntries(MarkOperation::kToggle); }

void MainWindow::LockEntries(bool locked)
{
    auto* current_widget { sc_->tab_widget->currentWidget() };

    Q_ASSERT(qobject_cast<TableWidget*>(current_widget));
    auto* leaf_widget { static_cast<TableWidget*>(current_widget) };

    auto* table_model { leaf_widget->Model() };
    table_model->LockEntries(locked);
}

void MainWindow::EntryLocateFIT(Section section, QUuid entry_id, QUuid lhs_node_id, QUuid rhs_node_id)
{
    if (entry_id.isNull() || lhs_node_id.isNull() || rhs_node_id.isNull())
        return;

    auto* sc { GetSectionContext(section) };

    QUuid node_id {};
    auto& widget_hash { sc->widget_hash };

    if (widget_hash.contains(lhs_node_id)) {
        node_id = lhs_node_id;
    } else if (widget_hash.contains(rhs_node_id)) {
        node_id = rhs_node_id;
    }

    if (!node_id.isNull()) {
        LocateLeafWidget(sc, node_id, entry_id);
        return;
    }

    node_id = lhs_node_id;

    CreateLeafFIPT(sc, node_id);

    const auto message { JsonGen::TableAck(section, node_id, entry_id) };
    WebSocket::Instance()->SendMessage(WsKey::kTableAck, message);
}

void MainWindow::EntryLocateP(Section section, QUuid entry_id, QUuid lhs_node_id)
{
    if (entry_id.isNull() || lhs_node_id.isNull())
        return;

    auto* sc { GetSectionContext(section) };

    if (!sc->widget_hash.contains(lhs_node_id)) {
        CreateLeafFIPT(sc, lhs_node_id);
        static_cast<EntryHubP*>(sc->entry_hub.data())->PushEntry(lhs_node_id);
    }

    LocateLeafWidget(sc, lhs_node_id, entry_id);
}

void MainWindow::on_actionLockAll_triggered() { LockEntries(true); }

void MainWindow::on_actionUnlockAll_triggered() { LockEntries(false); }
