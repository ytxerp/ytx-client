#include "searchdialogf.h"

#include "enum/entryenum.h"
#include "ui_searchdialog.h"

SearchDialogF::SearchDialogF(
    CTreeModel* tree, SearchNodeModel* search_tree, SearchEntryModel* search_table, CSectionConfig& config, CSectionInfo& info, QWidget* parent)
    : SearchDialog(tree, search_tree, search_table, config, info, parent)
{
    TreeViewDelegate(ui->searchViewNode);
    TableViewDelegate(ui->searchViewEntry);
}

void SearchDialogF::TreeViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kInitialTotal), value_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kFinalTotal), value_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kUnit), unit_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kDirectionRule), direction_rule_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kKind), kind_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kName), tree_path_);
}

void SearchDialogF::TableViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kLhsDebit), value_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kRhsDebit), value_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kLhsCredit), value_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kRhsCredit), value_);

    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kLhsRate), rate_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kRhsRate), rate_);

    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kLhsNode), table_path_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kRhsNode), table_path_);

    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kStatus), check_);
}

void SearchDialogF::REntryDoubleClicked(const QModelIndex& index)
{
    auto lhs_id { index.siblingAtColumn(std::to_underlying(FullEntryEnumF::kLhsNode)).data().toUuid() };
    auto rhs_id { index.siblingAtColumn(std::to_underlying(FullEntryEnumF::kRhsNode)).data().toUuid() };
    auto entry_id { index.siblingAtColumn(std::to_underlying(FullEntryEnumF::kId)).data().toUuid() };

    if (lhs_id.isNull() || rhs_id.isNull() || entry_id.isNull())
        return;

    emit SEntryLocation(entry_id, lhs_id, rhs_id);
}
