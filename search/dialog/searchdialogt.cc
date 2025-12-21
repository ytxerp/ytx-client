#include "searchdialogt.h"

#include <QHeaderView>

#include "enum/entryenum.h"
#include "ui_searchdialog.h"

SearchDialogT::SearchDialogT(
    CTreeModel* tree, SearchNodeModel* search_tree, SearchEntryModel* search_table, CSectionConfig& config, CSectionInfo& info, QWidget* parent)
    : SearchDialog(tree, search_tree, search_table, config, info, parent)
{
    TreeViewDelegate(ui->searchViewNode);
    TableViewDelegate(ui->searchViewEntry);
}

void SearchDialogT::TreeViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kInitialTotal), value_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kFinalTotal), value_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kStatus), check_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kColor), color_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kUnit), unit_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kDirectionRule), direction_rule_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kKind), kind_);
}

void SearchDialogT::REntryDoubleClicked(const QModelIndex& index)
{
    auto lhs_id { index.siblingAtColumn(std::to_underlying(FullEntryEnum::kLhsNode)).data().toUuid() };
    auto rhs_id { index.siblingAtColumn(std::to_underlying(FullEntryEnum::kRhsNode)).data().toUuid() };
    auto entry_id { index.siblingAtColumn(std::to_underlying(FullEntryEnum::kId)).data().toUuid() };

    if (lhs_id.isNull() || rhs_id.isNull() || entry_id.isNull())
        return;

    emit SEntryLocation(entry_id, lhs_id, rhs_id);
}
