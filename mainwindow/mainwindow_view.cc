#include <QHeaderView>

#include "enum/reference.h"
#include "mainwindow.h"
#include "utils/mainwindowutils.h"

void MainWindow::SetTreeView(QTreeView* tree_view, CSectionInfo& info) const
{
    const auto section { info.section };

    if (section == Section::kSale || section == Section::kPurchase) {
        tree_view->setColumnHidden(std::to_underlying(NodeEnumO::kSettlementId), kIsHidden);
    }

    {
        tree_view->setColumnHidden(std::to_underlying(NodeEnum::kId), kIsHidden);
        tree_view->setColumnHidden(std::to_underlying(NodeEnum::kUserId), kIsHidden);
        tree_view->setColumnHidden(std::to_underlying(NodeEnum::kCreateBy), kIsHidden);
        tree_view->setColumnHidden(std::to_underlying(NodeEnum::kCreateTime), kIsHidden);
        tree_view->setColumnHidden(std::to_underlying(NodeEnum::kUpdateTime), kIsHidden);
        tree_view->setColumnHidden(std::to_underlying(NodeEnum::kUpdateBy), kIsHidden);

        tree_view->setSelectionMode(QAbstractItemView::SingleSelection);
        tree_view->setDragDropMode(QAbstractItemView::DragDrop);
        tree_view->setEditTriggers(QAbstractItemView::DoubleClicked);
        tree_view->setDropIndicatorShown(true);
        tree_view->setSortingEnabled(true);
        tree_view->setContextMenuPolicy(Qt::CustomContextMenu);
        tree_view->setExpandsOnDoubleClick(true);
    }

    {
        auto* header { tree_view->header() };
        MainWindowUtils::SetupHeaderStatus(header, section_settings_, section, kTreeHeaderState);

        ResizeColumn(header, NodeUtils::DescriptionColumn(section));
        header->setStretchLastSection(false);
        header->setDefaultAlignment(Qt::AlignCenter);
    }
}

void MainWindow::SetTableViewFIPT(QTableView* view, int stretch_column, int lhs_node_column) const
{
    {
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setAlternatingRowColors(true);
        view->setSortingEnabled(true);
        view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::CurrentChanged);

        view->setColumnHidden(std::to_underlying(EntryEnum::kId), kIsHidden);
        view->setColumnHidden(lhs_node_column, kIsHidden);
        view->setColumnHidden(std::to_underlying(EntryEnum::kUserId), kIsHidden);
        view->setColumnHidden(std::to_underlying(EntryEnum::kCreateBy), kIsHidden);
        view->setColumnHidden(std::to_underlying(EntryEnum::kCreateTime), kIsHidden);
        view->setColumnHidden(std::to_underlying(EntryEnum::kUpdateTime), kIsHidden);
        view->setColumnHidden(std::to_underlying(EntryEnum::kUpdateBy), kIsHidden);
    }

    {
        auto* h_header { view->horizontalHeader() };
        ResizeColumn(h_header, stretch_column);
    }

    {
        auto* v_header { view->verticalHeader() };
        v_header->setDefaultSectionSize(kRowHeight);
        v_header->setSectionResizeMode(QHeaderView::Fixed);
        v_header->setHidden(true);
    }
}

void MainWindow::SetTableViewSaleReference(QTableView* view) const
{
    {
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setAlternatingRowColors(true);
        view->setSortingEnabled(true);
    }

    {
        view->setColumnHidden(std::to_underlying(SaleReferenceEnum::kOrderId), kIsHidden);
    }

    {
        auto* h_header { view->horizontalHeader() };
        ResizeColumn(h_header, std::to_underlying(SaleReferenceEnum::kDescription));
    }

    {
        auto* v_header { view->verticalHeader() };
        v_header->setDefaultSectionSize(kRowHeight);
        v_header->setSectionResizeMode(QHeaderView::Fixed);
        v_header->setHidden(true);
    }
}

void MainWindow::SetStatementView(QTableView* view, int stretch_column) const
{
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAlternatingRowColors(true);

    auto* h_header { view->horizontalHeader() };
    ResizeColumn(h_header, stretch_column);

    auto* v_header { view->verticalHeader() };
    v_header->setDefaultSectionSize(kRowHeight);
    v_header->setSectionResizeMode(QHeaderView::Fixed);
    v_header->setHidden(true);
}

void MainWindow::SetTableViewO(QTableView* view, Section section, int stretch_column, int lhs_node_column) const
{
    {
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setAlternatingRowColors(true);
        view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::CurrentChanged);

        view->setColumnHidden(std::to_underlying(EntryEnum::kId), kIsHidden);
        view->setColumnHidden(lhs_node_column, kIsHidden);
    }

    {
        auto* h_header { view->horizontalHeader() };
        MainWindowUtils::SetupHeaderStatus(h_header, section_settings_, section, kTableHeaderState);

        ResizeColumn(h_header, stretch_column);

        h_header->setSectionsMovable(true);
        h_header->setHighlightSections(true);
    }

    {
        auto* v_header { view->verticalHeader() };
        v_header->setDefaultSectionSize(kRowHeight);
        v_header->setSectionResizeMode(QHeaderView::Fixed);
        v_header->setHidden(true);

        view->setSortingEnabled(true);
    }
}
