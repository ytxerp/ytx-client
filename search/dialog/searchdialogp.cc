#include "searchdialogp.h"

#include <QHeaderView>

#include "component/enumclass.h"
#include "delegate/search/searchpathtabler.h"
#include "ui_searchdialog.h"

SearchDialogP::SearchDialogP(CTreeModel* tree, SearchNodeModel* search_tree, SearchEntryModel* search_table, CTreeModel* item_node, CSectionConfig& config,
    CSectionInfo& info, QWidget* parent)
    : SearchDialog(tree, search_tree, search_table, config, info, parent)
    , item_node_ { item_node }
{
    TreeViewDelegate(ui->searchViewNode);
    TableViewDelegate(ui->searchViewEntry);
}

void SearchDialogP::TreeViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kInitialTotal), value_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kFinalTotal), value_);
}

void SearchDialogP::TableViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kUnitPrice), rate_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kLhsNode), node_name_);

    auto* rhs_node_name { new SearchPathTableR(item_node_, view) };
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kRhsNode), rhs_node_name);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kExternalSku), rhs_node_name);

    auto* is_checked { new CheckBoxR(view) };
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kIsChecked), is_checked);
}

void SearchDialogP::REntryDoubleClicked(const QModelIndex& index)
{
    auto lhs_id { index.siblingAtColumn(std::to_underlying(EntryEnumP::kLhsNode)).data().toUuid() };
    auto entry_id { index.siblingAtColumn(std::to_underlying(EntryEnumP::kId)).data().toUuid() };

    if (lhs_id.isNull() || entry_id.isNull())
        return;

    emit SEntryLocation(entry_id, lhs_id, QUuid());
}
