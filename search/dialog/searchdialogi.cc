#include "searchdialogi.h"

#include "ui_searchdialog.h"

SearchDialogI::SearchDialogI(SectionContext* sc, search::NodeModel* search_node, search::EntryModel* search_entry, QWidget* parent)
    : SearchDialog(sc, search_node, search_entry, parent)
{
    TreeViewDelegate(ui->searchViewNode);
    TableViewDelegate(ui->searchViewEntry);
}

void SearchDialogI::TreeViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kInitialTotal), amount_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kFinalTotal), amount_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kColor), color_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kUnit), unit_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kDirectionRule), direction_rule_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kKind), kind_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kName), tree_path_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kUnitPrice), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kCommission), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kTag), tag_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kDocument), document_);
}
