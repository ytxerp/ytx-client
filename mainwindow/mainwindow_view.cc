#include <QHeaderView>

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
        view->setColumnHidden(std::to_underlying(NodeEnum::kVersion), kIsHidden);
    }
}

void MainWindow::SetTreeHeader(QTreeView* view, Section section)
{
    auto* header { view->header() };
    Utils::SetupHeaderStatus(header, section_settings_, section, kTreeHeaderState);
}

void MainWindow::SetTableView(QTableView* view, Section section, int stretch_column) const
{
    {
        view->setSortingEnabled(true);
        view->setAlternatingRowColors(true);
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setEditTriggers(QAbstractItemView::DoubleClicked);

        view->setColumnHidden(std::to_underlying(EntryEnum::kId), kIsHidden);
        view->setColumnHidden(std::to_underlying(EntryEnum::kVersion), kIsHidden);
        view->setColumnHidden(std::to_underlying(EntryEnum::kLhsNode), kIsHidden);
    }

    {
        auto* h_header { view->horizontalHeader() };
        Utils::SetupHeaderStatus(h_header, section_settings_, section, kTableHeaderState);

        h_header->setSectionsMovable(true);
        h_header->setHighlightSections(true);

        ResizeColumn(h_header, stretch_column);
    }

    {
        Utils::SetupVerticalHeader(view, UiConst::kRowHeight);
    }
}

void MainWindow::InitTableView(QTableView* view, int id_column, int version_column, int stretch_column) const
{
    {
        view->setSortingEnabled(true);
        view->setAlternatingRowColors(true);
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);

        if (id_column >= 0)
            view->setColumnHidden(id_column, kIsHidden);

        if (version_column >= 0)
            view->setColumnHidden(version_column, kIsHidden);
    }

    {
        auto* h_header { view->horizontalHeader() };
        h_header->setSectionsMovable(true);
        h_header->setHighlightSections(true);

        ResizeColumn(h_header, stretch_column);
    }

    {
        Utils::SetupVerticalHeader(view, UiConst::kRowHeight);
    }
}
