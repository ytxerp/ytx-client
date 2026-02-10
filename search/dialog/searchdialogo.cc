#include "searchdialogo.h"

#include <QHeaderView>

#include "enum/entryenum.h"
#include "ui_searchdialog.h"

SearchDialogO::SearchDialogO(CTreeModel* tree, SearchNodeModel* search_node, SearchEntryModel* search_entry, CTreeModel* inventory, CTreeModel* partner,
    CSectionConfig& config, CSectionInfo& info, const QHash<QUuid, Tag*>& tag_hash, QWidget* parent)
    : SearchDialog(tree, search_node, search_entry, config, info, tag_hash, parent)
    , inventory_ { inventory }
    , partner_ { partner }
{
    TreeViewDelegate(ui->searchViewNode);
    TableViewDelegate(ui->searchViewEntry);
}

void SearchDialogO::TreeViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kInitialTotal), amount_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kFinalTotal), amount_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kDiscountTotal), amount_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kCountTotal), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kMeasureTotal), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kUnit), unit_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kDirectionRule), direction_rule_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kKind), kind_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kName), tree_path_);

    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kStatus), check_);

    auto* partner_name { new SearchPathTableR(partner_, view) };
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kPartnerId), partner_name);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kEmployeeId), partner_name);
}

void SearchDialogO::TableViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kCount), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kMeasure), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kDiscount), amount_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kInitial), amount_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kFinal), amount_);

    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kUnitPrice), rate_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kUnitDiscount), rate_);

    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kLhsNode), table_path_);

    auto* item_name { new SearchPathTableR(inventory_, view) };
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kExternalSku), item_name);
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

void SearchDialogO::RNodeDoubleClicked(const QModelIndex& index)
{
    auto node_id { index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() };

    if (tree_model_->Contains(node_id))
        emit SNodeLocation(info_.section, node_id);
    else
        tree_model_->AckNode(node_id);
}
