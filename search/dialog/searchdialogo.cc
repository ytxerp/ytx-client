#include "searchdialogo.h"

#include <QHeaderView>

#include "component/enumclass.h"
#include "ui_searchdialog.h"

SearchDialogO::SearchDialogO(CTreeModel* tree, SearchNodeModel* search_tree, SearchEntryModel* search_table, CTreeModel* item_node, CTreeModel* partner_node,
    CSectionConfig& config, CSectionInfo& info, QWidget* parent)
    : SearchDialog(tree, search_tree, search_table, config, info, parent)
    , item_node_ { item_node }
    , partner_node_ { partner_node }
{
    TreeViewDelegate(ui->searchViewNode);
    TableViewDelegate(ui->searchViewEntry);
}

void SearchDialogO::TreeViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kInitialTotal), value_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kFinalTotal), value_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kDiscountTotal), value_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kCountTotal), value_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kMeasureTotal), value_);

    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kIsFinished), check_);

    auto* partner_name { new SearchPathTableR(partner_node_, view) };
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kParty), partner_name);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kEmployee), partner_name);
}

void SearchDialogO::TableViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kCount), value_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kMeasure), value_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kDiscount), value_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kInitial), value_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kFinal), value_);

    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kUnitPrice), rate_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kDiscountPrice), rate_);

    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kLhsNode), node_name_);

    auto* item_name { new SearchPathTableR(item_node_, view) };
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kExternalItem), item_name);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kRhsNode), item_name);
}

void SearchDialogO::REntryDoubleClicked(const QModelIndex& index)
{
    auto lhs_id { index.siblingAtColumn(std::to_underlying(EntryEnumO::kLhsNode)).data().toUuid() };
    auto entry_id { index.siblingAtColumn(std::to_underlying(EntryEnumO::kId)).data().toUuid() };

    if (lhs_id.isNull() || entry_id.isNull())
        return;

    emit SEntryLocation(entry_id, lhs_id, QUuid());
}
