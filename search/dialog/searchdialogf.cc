#include "searchdialogf.h"

#include "enum/entryenum.h"
#include "ui_searchdialog.h"

SearchDialogF::SearchDialogF(SectionContext* sc, search::NodeModel* search_node, search::EntryModel* search_entry, QWidget* parent)
    : SearchDialog(sc, search_node, search_entry, parent)
{
    cash_kind_ = new IntStringNoneZeroR(info_.cash_kind_map, this);
    roles_ = new FinanceRolesDelegateR(this);

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
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kColor), color_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kTag), tag_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kDocument), document_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kRoles), roles_);
}

void SearchDialogF::TableViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kLhsDebit), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kRhsDebit), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kLhsCredit), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kRhsCredit), quantity_);

    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kLhsRate), rate_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kRhsRate), rate_);

    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kLhsNode), table_path_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kRhsNode), table_path_);

    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kStatus), check_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kIssuedTime), issued_time_);

    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kDocument), document_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kTag), tag_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnumF::kCashKind), cash_kind_);
}
