#include "searchdialogo.h"

#include <QHeaderView>

#include "enum/entryenum.h"
#include "ui_searchdialog.h"

SearchDialogO::SearchDialogO(SectionContext* sc, search::NodeModel* search_node, search::EntryModel* search_entry, CTreeModel* inventory, CTreeModel* partner,
    QUuid widget_id, QWidget* parent)
    : SearchDialog(sc, search_node, search_entry, widget_id, parent)
    , inventory_ { inventory }
    , partner_ { partner }
{
    TreeViewDelegate(ui->searchViewNode);
    TableViewDelegate(ui->searchViewEntry);
}

void SearchDialogO::TreeViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kInitialTotal), amount_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kCountTotal), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kMeasureTotal), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kUnit), unit_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kDirectionRule), direction_rule_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kKind), kind_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kTag), tag_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kIssuedTime), issued_time_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kColor), color_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kDocument), document_);
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kStatus), check_);

    auto* partner_name { new SearchPathTableR(partner_, view) };
    view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kEmployeeId), partner_name);
}

void SearchDialogO::TableViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kCount), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kMeasure), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kInitial), amount_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kTag), tag_);

    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kUnitPrice), rate_);

    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kLhsNode), table_path_);
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kDocument), document_);

    auto* item_name { new SearchPathTableR(inventory_, view) };
    view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kRhsNode), item_name);
}
