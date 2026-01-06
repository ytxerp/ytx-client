#include "global/tablesstation.h"
#include "mainwindow.h"

void MainWindow::TreeConnectF(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SResizeColumnToContents, tree_view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRemoveEntryHash, TableSStation::Instance(), &TableSStation::RRemoveEntryHash, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveMultiEntry, TableSStation::Instance(), &TableSStation::RRemoveMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendMultiEntry, TableSStation::Instance(), &TableSStation::RAppendMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRefreshStatus, TableSStation::Instance(), &TableSStation::RRefreshStatus, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAppendOneEntry, TableSStation::Instance(), &TableSStation::RAppendOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveOneEntry, TableSStation::Instance(), &TableSStation::RRemoveOneEntry, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshField, TableSStation::Instance(), &TableSStation::RRefreshField, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SUpdateBalance, TableSStation::Instance(), &TableSStation::RUpdateBalance, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SDirectionRule, TableSStation::Instance(), &TableSStation::RDirectionRule, Qt::UniqueConnection);
}

void MainWindow::TreeConnectI(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SResizeColumnToContents, tree_view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRemoveEntryHash, TableSStation::Instance(), &TableSStation::RRemoveEntryHash, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveMultiEntry, TableSStation::Instance(), &TableSStation::RRemoveMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendMultiEntry, TableSStation::Instance(), &TableSStation::RAppendMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRefreshStatus, TableSStation::Instance(), &TableSStation::RRefreshStatus, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAppendOneEntry, TableSStation::Instance(), &TableSStation::RAppendOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveOneEntry, TableSStation::Instance(), &TableSStation::RRemoveOneEntry, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshField, TableSStation::Instance(), &TableSStation::RRefreshField, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SUpdateBalance, TableSStation::Instance(), &TableSStation::RUpdateBalance, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SDirectionRule, TableSStation::Instance(), &TableSStation::RDirectionRule, Qt::UniqueConnection);
}

void MainWindow::TreeConnectT(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SResizeColumnToContents, tree_view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRemoveEntryHash, TableSStation::Instance(), &TableSStation::RRemoveEntryHash, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveMultiEntry, TableSStation::Instance(), &TableSStation::RRemoveMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendMultiEntry, TableSStation::Instance(), &TableSStation::RAppendMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRefreshStatus, TableSStation::Instance(), &TableSStation::RRefreshStatus, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAppendOneEntry, TableSStation::Instance(), &TableSStation::RAppendOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveOneEntry, TableSStation::Instance(), &TableSStation::RRemoveOneEntry, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshField, TableSStation::Instance(), &TableSStation::RRefreshField, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SUpdateBalance, TableSStation::Instance(), &TableSStation::RUpdateBalance, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SDirectionRule, TableSStation::Instance(), &TableSStation::RDirectionRule, Qt::UniqueConnection);
}

void MainWindow::TreeConnectP(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAppendMultiEntry, TableSStation::Instance(), &TableSStation::RAppendMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendOneEntry, TableSStation::Instance(), &TableSStation::RAppendOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveOneEntry, TableSStation::Instance(), &TableSStation::RRemoveOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRefreshField, TableSStation::Instance(), &TableSStation::RRefreshField, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SResizeColumnToContents, tree_view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshStatus, TableSStation::Instance(), &TableSStation::RRefreshStatus, Qt::UniqueConnection);
}

void MainWindow::TreeConnectO(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SResizeColumnToContents, tree_view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendMultiEntry, TableSStation::Instance(), &TableSStation::RAppendMultiEntry, Qt::UniqueConnection);
}

void MainWindow::TableConnectF(QTableView* table_view, TableModel* table_model) const
{
    auto tree_model { sc_f_.tree_model };
    auto entry_hub { sc_f_.entry_hub };

    connect(table_model, &TableModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);

    connect(table_model, &TableModel::SAppendOneEntry, entry_hub, &EntryHub::RAppendOneEntry);
    connect(table_model, &TableModel::SRemoveOneEntry, entry_hub, &EntryHub::RRemoveOneEntry);

    connect(table_model, &TableModel::SAttachOneEntry, TableSStation::Instance(), &TableSStation::RAppendOneEntry);
    connect(table_model, &TableModel::SDetachOneEntry, TableSStation::Instance(), &TableSStation::RRemoveOneEntry);
    connect(table_model, &TableModel::SUpdateBalance, TableSStation::Instance(), &TableSStation::RUpdateBalance);
}

void MainWindow::TableConnectI(QTableView* table_view, TableModel* table_model) const
{
    auto tree_model { sc_i_.tree_model };
    auto entry_hub { sc_i_.entry_hub };

    connect(table_model, &TableModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);

    connect(table_model, &TableModel::SAppendOneEntry, entry_hub, &EntryHub::RAppendOneEntry);
    connect(table_model, &TableModel::SRemoveOneEntry, entry_hub, &EntryHub::RRemoveOneEntry);

    connect(table_model, &TableModel::SAttachOneEntry, TableSStation::Instance(), &TableSStation::RAppendOneEntry);
    connect(table_model, &TableModel::SDetachOneEntry, TableSStation::Instance(), &TableSStation::RRemoveOneEntry);
    connect(table_model, &TableModel::SUpdateBalance, TableSStation::Instance(), &TableSStation::RUpdateBalance);
}

void MainWindow::TableConnectT(QTableView* table_view, TableModel* table_model) const
{
    auto tree_model { sc_t_.tree_model };
    auto entry_hub { sc_t_.entry_hub };

    connect(table_model, &TableModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);

    connect(table_model, &TableModel::SAppendOneEntry, entry_hub, &EntryHub::RAppendOneEntry);
    connect(table_model, &TableModel::SRemoveOneEntry, entry_hub, &EntryHub::RRemoveOneEntry);

    connect(table_model, &TableModel::SAttachOneEntry, TableSStation::Instance(), &TableSStation::RAppendOneEntry);
    connect(table_model, &TableModel::SDetachOneEntry, TableSStation::Instance(), &TableSStation::RRemoveOneEntry);
    connect(table_model, &TableModel::SUpdateBalance, TableSStation::Instance(), &TableSStation::RUpdateBalance);
}

void MainWindow::TableConnectP(QTableView* table_view, TableModel* table_model) const
{
    auto entry_hub { sc_p_.entry_hub };

    connect(table_model, &TableModel::SAppendOneEntry, entry_hub, &EntryHub::RAppendOneEntry);
    connect(table_model, &TableModel::SRemoveOneEntry, entry_hub, &EntryHub::RRemoveOneEntry);

    connect(table_model, &TableModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);
}

void MainWindow::TableConnectO(QTableView* table_view, TableModelO* table_model_o, TableWidgetO* widget) const
{
    connect(table_model_o, &TableModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);
    connect(table_model_o, &TableModelO::SSyncDeltaO, widget, &TableWidgetO::RSyncDeltaO);

    connect(widget, &TableWidgetO::SSyncPartner, this, &MainWindow::RSyncPartner);
}
