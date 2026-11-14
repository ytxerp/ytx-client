#include "searchdialogr.h"

#include <QHeaderView>

#include "enum/entryenum.h"
#include "ui_searchdialog.h"

SearchDialogR::SearchDialogR(
    SectionContext* sc, search::NodeModel* search_node, search::EntryModel* search_entry, CTreeModel* finance, QUuid widget_id, QWidget* parent)
    : SearchDialog(sc, search_node, search_entry, widget_id, parent)
    , finance_ { finance }
{
    TreeViewDelegate(ui->searchViewNode);
    TableViewDelegate(ui->searchViewEntry);
}

void SearchDialogR::TreeViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumR::kInitialTotal), amount_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumR::kKind), kind_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumR::kTag), tag_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumR::kIssuedTime), issued_time_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumR::kColor), color_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumR::kDocument), document_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumR::kStatus), status_);
}

void SearchDialogR::TableViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumR::kInitial), amount_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumR::kTag), tag_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumR::kDocument), document_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumR::kIssuedTime), issued_time_);

    auto* finance_name { new SearchPathTableR(finance_->LeafPath(), view) };
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumR::kDebitNode), finance_name);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumR::kCreditNode), finance_name);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumR::kIssuedTime), issued_time_);
}
