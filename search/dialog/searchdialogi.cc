#include "searchdialogi.h"

#include "enum/entryenum.h"
#include "ui_searchdialog.h"

SearchDialogI::SearchDialogI(
    CTreeModel* tree, SearchNodeModel* search_tree, SearchEntryModel* search_table, CSectionConfig& config, CSectionInfo& info, QWidget* parent)
    : SearchDialog(tree, search_tree, search_table, config, info, parent)
{
    TreeViewDelegate(ui->searchViewNode);
    TableViewDelegate(ui->searchViewEntry);
}

void SearchDialogI::TreeViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kInitialTotal), value_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kFinalTotal), value_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kColor), color_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kUnit), unit_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kDirectionRule), direction_rule_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kKind), kind_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kName), tree_path_);
}

void SearchDialogI::TableViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumI::kLhsDebit), value_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumI::kRhsDebit), value_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumI::kLhsCredit), value_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumI::kRhsCredit), value_);

    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumI::kLhsRate), rate_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumI::kRhsRate), rate_);

    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumI::kLhsNode), table_path_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumI::kRhsNode), table_path_);

    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kStatus), check_);
}

void SearchDialogI::REntryDoubleClicked(const QModelIndex& index)
{
    auto lhs_id { index.siblingAtColumn(std::to_underlying(FullEntryEnumI::kLhsNode)).data().toUuid() };
    auto rhs_id { index.siblingAtColumn(std::to_underlying(FullEntryEnumI::kRhsNode)).data().toUuid() };
    auto entry_id { index.siblingAtColumn(std::to_underlying(FullEntryEnumI::kId)).data().toUuid() };

    if (lhs_id.isNull() || rhs_id.isNull() || entry_id.isNull())
        return;

    emit SEntryLocation(entry_id, lhs_id, rhs_id);
}
