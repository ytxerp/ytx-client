#include "searchdialogf.h"

#include "enum/entryenum.h"
#include "ui_searchdialog.h"

SearchDialogF::SearchDialogF(
    CTreeModel* tree, SearchNodeModel* search_node, SearchEntryModel* search_entry, CSectionConfig& config, CSectionInfo& info, QWidget* parent)
    : SearchDialog(tree, search_node, search_entry, config, info, parent)
{
    TreeViewDelegate(ui->searchViewNode);
    TableViewDelegate(ui->searchViewEntry);
}

void SearchDialogF::TreeViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kInitialTotal), amount_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kFinalTotal), amount_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kUnit), unit_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kDirectionRule), direction_rule_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kKind), kind_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kName), tree_path_);
}

void SearchDialogF::REntryDoubleClicked(const QModelIndex& index)
{
    auto lhs_id { index.siblingAtColumn(std::to_underlying(FullEntryEnum::kLhsNode)).data().toUuid() };
    auto rhs_id { index.siblingAtColumn(std::to_underlying(FullEntryEnum::kRhsNode)).data().toUuid() };
    auto entry_id { index.siblingAtColumn(std::to_underlying(FullEntryEnum::kId)).data().toUuid() };

    if (lhs_id.isNull() || rhs_id.isNull() || entry_id.isNull())
        return;

    emit SEntryLocation(entry_id, lhs_id, rhs_id);
}
