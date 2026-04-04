#include <QHeaderView>

#include "enum/settlementenum.h"
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
        Utils::SetupVerticalHeader(view, UiConst::kRowHeight);
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
        v_header->setDefaultSectionSize(UiConst::kRowHeight);
        v_header->setSectionResizeMode(QHeaderView::Fixed);
        v_header->setHidden(true);
    }
}

void MainWindow::InitTableView(QTableView* view, int id_column, int stretch_column) const
{
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAlternatingRowColors(true);

    if (id_column >= 0)
        view->setColumnHidden(id_column, kIsHidden);

    auto* h_header { view->horizontalHeader() };
    h_header->setSectionResizeMode(QHeaderView::ResizeToContents);

    if (stretch_column >= 0)
        h_header->setSectionResizeMode(stretch_column, QHeaderView::Stretch);

    auto* v_header { view->verticalHeader() };
    v_header->setDefaultSectionSize(UiConst::kRowHeight);
    v_header->setSectionResizeMode(QHeaderView::Fixed);
    v_header->setHidden(true);
    h_header->setSectionsMovable(true);
}
