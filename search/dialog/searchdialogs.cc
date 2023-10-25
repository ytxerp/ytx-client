#include "searchdialogs.h"

#include <QHeaderView>

#include "component/enumclass.h"
#include "delegate/search/searchpathtabler.h"
#include "ui_searchdialog.h"

SearchDialogS::SearchDialogS(CTreeModel* tree, SearchNodeModel* search_tree, SearchEntryModel* search_table, CTreeModel* item_node, CSectionConfig& config,
    CSectionInfo& info, QWidget* parent)
    : SearchDialog(tree, search_tree, search_table, config, info, parent)
    , item_node_ { item_node }
{
    TreeViewDelegate(ui->searchViewNode);
    TableViewDelegate(ui->searchViewEntry);
}

void SearchDialogS::TreeViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumS::kInitialTotal), value_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumS::kFinalTotal), value_);
}

void SearchDialogS::TableViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumS::kUnitPrice), rate_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumS::kLhsNode), node_name_);

    auto* rhs_node_name { new SearchPathTableR(item_node_, view) };
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumS::kRhsNode), rhs_node_name);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumS::kExternalItem), rhs_node_name);

    auto* is_checked { new CheckBoxR(view) };
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumS::kIsChecked), is_checked);
}

void SearchDialogS::REntryDoubleClicked(const QModelIndex& index)
{
    auto lhs_id { index.siblingAtColumn(std::to_underlying(EntryEnumS::kLhsNode)).data().toUuid() };
    auto entry_id { index.siblingAtColumn(std::to_underlying(EntryEnumS::kId)).data().toUuid() };

    if (lhs_id.isNull() || entry_id.isNull())
        return;

    emit SEntryLocation(entry_id, lhs_id, QUuid());
}
