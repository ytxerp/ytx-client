#include "global/tablesstation.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::TreeConnectF(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SDeleteEntryHash, TableSStation::Instance(), &TableSStation::RDeleteEntryHash, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SDeleteMultiEntries, TableSStation::Instance(), &TableSStation::RDeleteMultiEntries, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendMultiEntries, TableSStation::Instance(), &TableSStation::RAppendMultiEntries, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRefreshStatus, TableSStation::Instance(), &TableSStation::RRefreshStatus, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAttachOneEntry, TableSStation::Instance(), &TableSStation::RAttachOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SDetachOneEntry, TableSStation::Instance(), &TableSStation::RDetachOneEntry, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshField, TableSStation::Instance(), &TableSStation::RRefreshField, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SUpdateBalance, TableSStation::Instance(), &TableSStation::RUpdateBalance, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SDirectionRule, TableSStation::Instance(), &TableSStation::RDirectionRule, Qt::UniqueConnection);

    connect(ui->tabWidgetF, &QTabWidget::currentChanged, this, &MainWindow::tabWidget_currentChanged, Qt::UniqueConnection);
    connect(ui->tabWidgetF, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::tabWidget_tabBarDoubleClicked, Qt::UniqueConnection);
    connect(ui->tabWidgetF, &QTabWidget::tabCloseRequested, this, &MainWindow::tabWidget_tabCloseRequestedFIT, Qt::UniqueConnection);
}

void MainWindow::TreeConnectI(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SDeleteEntryHash, TableSStation::Instance(), &TableSStation::RDeleteEntryHash, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SDeleteMultiEntries, TableSStation::Instance(), &TableSStation::RDeleteMultiEntries, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendMultiEntries, TableSStation::Instance(), &TableSStation::RAppendMultiEntries, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRefreshStatus, TableSStation::Instance(), &TableSStation::RRefreshStatus, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAttachOneEntry, TableSStation::Instance(), &TableSStation::RAttachOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SDetachOneEntry, TableSStation::Instance(), &TableSStation::RDetachOneEntry, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshField, TableSStation::Instance(), &TableSStation::RRefreshField, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SUpdateBalance, TableSStation::Instance(), &TableSStation::RUpdateBalance, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SDirectionRule, TableSStation::Instance(), &TableSStation::RDirectionRule, Qt::UniqueConnection);

    connect(ui->tabWidgetI, &QTabWidget::currentChanged, this, &MainWindow::tabWidget_currentChanged, Qt::UniqueConnection);
    connect(ui->tabWidgetI, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::tabWidget_tabBarDoubleClicked, Qt::UniqueConnection);
    connect(ui->tabWidgetI, &QTabWidget::tabCloseRequested, this, &MainWindow::tabWidget_tabCloseRequestedFIT, Qt::UniqueConnection);
}

void MainWindow::TreeConnectT(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SDeleteEntryHash, TableSStation::Instance(), &TableSStation::RDeleteEntryHash, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SDeleteMultiEntries, TableSStation::Instance(), &TableSStation::RDeleteMultiEntries, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendMultiEntries, TableSStation::Instance(), &TableSStation::RAppendMultiEntries, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRefreshStatus, TableSStation::Instance(), &TableSStation::RRefreshStatus, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAttachOneEntry, TableSStation::Instance(), &TableSStation::RAttachOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SDetachOneEntry, TableSStation::Instance(), &TableSStation::RDetachOneEntry, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshField, TableSStation::Instance(), &TableSStation::RRefreshField, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SUpdateBalance, TableSStation::Instance(), &TableSStation::RUpdateBalance, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SDirectionRule, TableSStation::Instance(), &TableSStation::RDirectionRule, Qt::UniqueConnection);

    connect(ui->tabWidgetT, &QTabWidget::currentChanged, this, &MainWindow::tabWidget_currentChanged, Qt::UniqueConnection);
    connect(ui->tabWidgetT, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::tabWidget_tabBarDoubleClicked, Qt::UniqueConnection);
    connect(ui->tabWidgetT, &QTabWidget::tabCloseRequested, this, &MainWindow::tabWidget_tabCloseRequestedFIT, Qt::UniqueConnection);
}

void MainWindow::TreeConnectP(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAppendMultiEntries, TableSStation::Instance(), &TableSStation::RAppendMultiEntries, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAttachOneEntry, TableSStation::Instance(), &TableSStation::RAttachOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SDetachOneEntry, TableSStation::Instance(), &TableSStation::RDetachOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRefreshField, TableSStation::Instance(), &TableSStation::RRefreshField, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshStatus, TableSStation::Instance(), &TableSStation::RRefreshStatus, Qt::UniqueConnection);

    connect(ui->tabWidgetP, &QTabWidget::currentChanged, this, &MainWindow::tabWidget_currentChanged, Qt::UniqueConnection);
    connect(ui->tabWidgetP, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::tabWidget_tabBarDoubleClicked, Qt::UniqueConnection);
    connect(ui->tabWidgetP, &QTabWidget::tabCloseRequested, this, &MainWindow::tabWidget_tabCloseRequestedP, Qt::UniqueConnection);
}

void MainWindow::TreeConnectO(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub, Section section) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendMultiEntries, TableSStation::Instance(), &TableSStation::RAppendMultiEntries, Qt::UniqueConnection);

    auto* tab_widget { section == Section::kSale ? ui->tabWidgetSale : ui->tabWidgetPurchase };

    connect(tab_widget, &QTabWidget::currentChanged, this, &MainWindow::tabWidget_currentChanged, Qt::UniqueConnection);
    connect(tab_widget, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::tabWidget_tabBarDoubleClicked, Qt::UniqueConnection);
    connect(tab_widget, &QTabWidget::tabCloseRequested, this, &MainWindow::tabWidget_tabCloseRequestedO, Qt::UniqueConnection);
}

void MainWindow::TableConnectF(TableModel* table_model) const
{
    auto tree_model { sc_f_.tree_model };
    auto entry_hub { sc_f_.entry_hub };

    connect(table_model, &TableModel::SAppendOneEntry, entry_hub, &EntryHub::RAppendOneEntry);
    connect(table_model, &TableModel::SDeleteOneEntry, entry_hub, &EntryHub::RDeleteOneEntry);

    connect(table_model, &TableModel::SAttachOneEntry, TableSStation::Instance(), &TableSStation::RAttachOneEntry);
    connect(table_model, &TableModel::SDetachOneEntry, TableSStation::Instance(), &TableSStation::RDetachOneEntry);
    connect(table_model, &TableModel::SUpdateBalance, TableSStation::Instance(), &TableSStation::RUpdateBalance);
}

void MainWindow::TableConnectI(TableModel* table_model) const
{
    auto tree_model { sc_i_.tree_model };
    auto entry_hub { sc_i_.entry_hub };

    connect(table_model, &TableModel::SAppendOneEntry, entry_hub, &EntryHub::RAppendOneEntry);
    connect(table_model, &TableModel::SDeleteOneEntry, entry_hub, &EntryHub::RDeleteOneEntry);

    connect(table_model, &TableModel::SAttachOneEntry, TableSStation::Instance(), &TableSStation::RAttachOneEntry);
    connect(table_model, &TableModel::SDetachOneEntry, TableSStation::Instance(), &TableSStation::RDetachOneEntry);
    connect(table_model, &TableModel::SUpdateBalance, TableSStation::Instance(), &TableSStation::RUpdateBalance);
}

void MainWindow::TableConnectT(TableModel* table_model) const
{
    auto tree_model { sc_t_.tree_model };
    auto entry_hub { sc_t_.entry_hub };

    connect(table_model, &TableModel::SAppendOneEntry, entry_hub, &EntryHub::RAppendOneEntry);
    connect(table_model, &TableModel::SDeleteOneEntry, entry_hub, &EntryHub::RDeleteOneEntry);

    connect(table_model, &TableModel::SAttachOneEntry, TableSStation::Instance(), &TableSStation::RAttachOneEntry);
    connect(table_model, &TableModel::SDetachOneEntry, TableSStation::Instance(), &TableSStation::RDetachOneEntry);
    connect(table_model, &TableModel::SUpdateBalance, TableSStation::Instance(), &TableSStation::RUpdateBalance);
}

void MainWindow::TableConnectP(TableModel* table_model) const
{
    auto entry_hub { sc_p_.entry_hub };

    connect(table_model, &TableModel::SAppendOneEntry, entry_hub, &EntryHub::RAppendOneEntry);
    connect(table_model, &TableModel::SDeleteOneEntry, entry_hub, &EntryHub::RDeleteOneEntry);
}

void MainWindow::TableConnectO(TableModelO* table_model_o, TableWidgetO* widget) const
{
    connect(table_model_o, &TableModelO::SSyncDeltaO, widget, &TableWidgetO::RSyncDeltaO);

    connect(widget, &TableWidgetO::SSyncPartner, this, &MainWindow::RSyncPartner);
}
