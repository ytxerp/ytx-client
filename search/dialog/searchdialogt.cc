#include "searchdialogt.h"

#include <QHeaderView>

#include "ui_searchdialog.h"

SearchDialogT::SearchDialogT(SectionContext* sc, search::NodeModel* search_node, search::EntryModel* search_entry, QWidget* parent)
    : SearchDialog(sc, search_node, search_entry, parent)
{
    TreeViewDelegate(ui->searchViewNode);
    TableViewDelegate(ui->searchViewEntry);
}

void SearchDialogT::TreeViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kInitialTotal), amount_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kFinalTotal), amount_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kColor), color_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kUnit), unit_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kDirectionRule), direction_rule_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kKind), kind_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kDocument), document_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kTag), tag_);
}
