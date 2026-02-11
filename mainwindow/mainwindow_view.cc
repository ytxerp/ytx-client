#include <QHeaderView>

#include "enum/reference.h"
#include "enum/settlementenum.h"
#include "enum/tagenum.h"
#include "mainwindow.h"
#include "utils/mainwindowutils.h"
#include "utils/templateutils.h"

void MainWindow::SetTreeView(QTreeView* view, CSectionInfo& info) const
{
    const auto section { info.section };

    {
        auto* header { view->header() };

        ResizeColumn(header, Utils::NodeDescriptionColumn(section));
        header->setStretchLastSection(false);
        header->setDefaultAlignment(Qt::AlignCenter);
    }

    if (section == Section::kSale || section == Section::kPurchase) {
        view->setColumnHidden(std::to_underlying(NodeEnumO::kSettlementId), kIsHidden);
        view->setColumnHidden(std::to_underlying(NodeEnumO::kIsSettled), kIsHidden);
    }

    {
        view->setColumnHidden(std::to_underlying(NodeEnum::kId), kIsHidden);
        view->setColumnHidden(std::to_underlying(NodeEnum::kUserId), kIsHidden);
        view->setColumnHidden(std::to_underlying(NodeEnum::kCreateBy), kIsHidden);
        view->setColumnHidden(std::to_underlying(NodeEnum::kCreateTime), kIsHidden);
        view->setColumnHidden(std::to_underlying(NodeEnum::kUpdateTime), kIsHidden);
        view->setColumnHidden(std::to_underlying(NodeEnum::kUpdateBy), kIsHidden);
        view->setColumnHidden(std::to_underlying(NodeEnum::kVersion), kIsHidden);

        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setDragDropMode(QAbstractItemView::DragDrop);
        view->setEditTriggers(QAbstractItemView::DoubleClicked);
        view->setDropIndicatorShown(true);
        view->setSortingEnabled(true);
        view->setContextMenuPolicy(Qt::CustomContextMenu);
        view->setExpandsOnDoubleClick(true);
    }
}

void MainWindow::SetTreeHeader(QTreeView* view, Section section)
{
    auto* header { view->header() };
    Utils::SetupHeaderStatus(header, section_settings_, section, kTreeHeaderState);
}

void MainWindow::SetTableView(QTableView* view, Section section, int stretch_column, int lhs_node_column) const
{
    {
        auto* h_header { view->horizontalHeader() };
        Utils::SetupHeaderStatus(h_header, section_settings_, section, kTableHeaderState);

        ResizeColumn(h_header, stretch_column);

        h_header->setSectionsMovable(true);
        h_header->setHighlightSections(true);
    }

    {
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setAlternatingRowColors(true);
        view->setSortingEnabled(true);
        view->setEditTriggers(QAbstractItemView::DoubleClicked);

        view->setColumnHidden(std::to_underlying(EntryEnum::kId), kIsHidden);
        view->setColumnHidden(lhs_node_column, kIsHidden);
        view->setColumnHidden(std::to_underlying(EntryEnum::kUserId), kIsHidden);
        view->setColumnHidden(std::to_underlying(EntryEnum::kCreateBy), kIsHidden);
        view->setColumnHidden(std::to_underlying(EntryEnum::kCreateTime), kIsHidden);
        view->setColumnHidden(std::to_underlying(EntryEnum::kUpdateTime), kIsHidden);
        view->setColumnHidden(std::to_underlying(EntryEnum::kUpdateBy), kIsHidden);
        view->setColumnHidden(std::to_underlying(EntryEnum::kVersion), kIsHidden);
    }

    {
        Utils::SetupVerticalHeader(view, kRowHeight);
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
    {
        view->setSortingEnabled(true);
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setAlternatingRowColors(true);
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

void MainWindow::SetSettlementView(QTableView* view, int stretch_column) const
{
    {
        view->setSortingEnabled(true);
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setAlternatingRowColors(true);
    }

    {
        view->setColumnHidden(std::to_underlying(SettlementEnum::kId), kIsHidden);
        view->setColumnHidden(std::to_underlying(SettlementEnum::kUserId), kIsHidden);
        view->setColumnHidden(std::to_underlying(SettlementEnum::kCreateBy), kIsHidden);
        view->setColumnHidden(std::to_underlying(SettlementEnum::kCreateTime), kIsHidden);
        view->setColumnHidden(std::to_underlying(SettlementEnum::kUpdateTime), kIsHidden);
        view->setColumnHidden(std::to_underlying(SettlementEnum::kUpdateBy), kIsHidden);
        view->setColumnHidden(std::to_underlying(SettlementEnum::kVersion), kIsHidden);
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

void MainWindow::SetSettlementItemView(QTableView* view, int stretch_column) const
{
    {
        view->setSortingEnabled(true);
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setAlternatingRowColors(true);
    }

    {
        view->setColumnHidden(std::to_underlying(SettlementItemEnum::kId), kIsHidden);
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

void MainWindow::SetTagView(QTableView* view) const
{
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAlternatingRowColors(true);

    view->setColumnHidden(std::to_underlying(TagEnum::kId), kIsHidden);
    view->setColumnHidden(std::to_underlying(TagEnum::kVersion), kIsHidden);

    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    view->horizontalHeader()->setSectionResizeMode(std::to_underlying(TagEnum::kColor), QHeaderView::Stretch);

    auto* v_header { view->verticalHeader() };
    v_header->setDefaultSectionSize(kRowHeight);
    v_header->setSectionResizeMode(QHeaderView::Fixed);
    v_header->setHidden(true);
}
