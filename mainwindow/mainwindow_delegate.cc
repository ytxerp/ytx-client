#include "delegate/bool.h"
#include "delegate/boolstring.h"
#include "delegate/color.h"
#include "delegate/document.h"
#include "delegate/double.h"
#include "delegate/filterunit.h"
#include "delegate/int.h"
#include "delegate/issuedtime.h"
#include "delegate/line.h"
#include "delegate/plaintext.h"
#include "delegate/readonly/amountr.h"
#include "delegate/readonly/amountsalereferencer.h"
#include "delegate/readonly/boolstringr.h"
#include "delegate/readonly/doublespinnonezeror.h"
#include "delegate/readonly/financeforeignr.h"
#include "delegate/readonly/intstringr.h"
#include "delegate/readonly/issuedtimer.h"
#include "delegate/readonly/nodenamer.h"
#include "delegate/readonly/nodepathr.h"
#include "delegate/readonly/quantityr.h"
#include "delegate/readonly/statusr.h"
#include "delegate/rhsnode.h"
#include "delegate/status.h"
#include "delegate/tagdelegate.h"
#include "enum/reference.h"
#include "enum/settlementenum.h"
#include "enum/statementenum.h"
#include "enum/tagenum.h"
#include "mainwindow.h"

void MainWindow::TreeDelegateF(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const
{
    auto* line { new Line(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kCode), line);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kDescription), line);

    auto* plain_text { new PlainText(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kNote), plain_text);

    auto* direction_rule { new BoolString(info.rule_map, QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kDirectionRule), direction_rule);

    auto* unit { new IntStringR(info.unit_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kUnit), unit);

    auto* kind { new IntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kKind), kind);

    auto* color { new Color(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kColor), color);

    auto* final_total { new AmountR(section.amount_decimal, sc_f_.shared_config.default_unit, info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kFinalTotal), final_total);

    auto* initial_total { new FinanceForeignR(section.amount_decimal, sc_f_.shared_config.default_unit, info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kInitialTotal), initial_total);

    auto* tag { new TagDelegate(sc_f_.tag_hash, sc_f_.tag_pixmap, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kTag), tag);
}

void MainWindow::TreeDelegateT(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const
{
    auto* line { new Line(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kCode), line);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kDescription), line);

    auto* plain_text { new PlainText(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kNote), plain_text);

    auto* direction_rule { new BoolString(info.rule_map, QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kDirectionRule), direction_rule);

    auto* unit { new IntStringR(info.unit_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kUnit), unit);

    auto* kind { new IntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kKind), kind);

    auto* quantity { new QuantityR(section.quantity_decimal, kCoefficient16, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kInitialTotal), quantity);

    auto* amount { new AmountR(section.amount_decimal, sc_f_.shared_config.default_unit, sc_f_.info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kFinalTotal), amount);

    auto* color { new Color(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kColor), color);

    auto* status { new Status(QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kStatus), status);

    auto* document { new Document(sc_t_.shared_config.document_dir, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kDocument), document);

    auto* issued_time { new IssuedTime(section.date_format, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kIssuedTime), issued_time);

    auto* tag { new TagDelegate(sc_t_.tag_hash, sc_t_.tag_pixmap, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kTag), tag);
}

void MainWindow::TreeDelegateI(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const
{
    auto* line { new Line(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kCode), line);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kDescription), line);

    auto* plain_text { new PlainText(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kNote), plain_text);

    auto* direction_rule { new BoolString(info.rule_map, QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kDirectionRule), direction_rule);

    auto* unit { new IntStringR(info.unit_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kUnit), unit);

    auto* kind { new IntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kKind), kind);

    auto* quantity { new QuantityR(section.quantity_decimal, kCoefficient16, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kInitialTotal), quantity);

    auto* amount { new AmountSaleReferenceR(info.section, section.amount_decimal, sc_f_.shared_config.default_unit, sc_f_.info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kFinalTotal), amount);
    connect(amount, &AmountSaleReferenceR::SSaleReferencePrimary, this, &MainWindow::RSaleReferencePrimary);

    auto* unit_price { new Double(section.rate_decimal, 0.0, kDoubleMax, kCoefficient8, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kUnitPrice), unit_price);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kCommission), unit_price);

    auto* color { new Color(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kColor), color);

    auto* tag { new TagDelegate(sc_i_.tag_hash, sc_i_.tag_pixmap, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kTag), tag);
}

void MainWindow::TreeDelegateP(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const
{
    auto* color { new Color(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kColor), color);

    auto* line { new Line(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kCode), line);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kDescription), line);

    auto* plain_text { new PlainText(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kNote), plain_text);

    auto* unit { new IntStringR(info.unit_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kUnit), unit);

    auto* kind { new IntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kKind), kind);

    auto* amount { new AmountSaleReferenceR(info.section, section.amount_decimal, sc_f_.shared_config.default_unit, sc_f_.info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kInitialTotal), amount);
    connect(amount, &AmountSaleReferenceR::SSaleReferencePrimary, this, &MainWindow::RSaleReferencePrimary);

    auto* payment_term { new Int(0, 36500, tree_view) }; // one hundred years
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kPaymentTerm), payment_term);

    auto* tag { new TagDelegate(sc_p_.tag_hash, sc_p_.tag_pixmap, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kTag), tag);
}

void MainWindow::TreeDelegateO(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const
{
    auto* direction_rule_r { new BoolStringR(info.rule_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kDirectionRule), direction_rule_r);

    auto* unit_r { new IntStringR(info.unit_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kUnit), unit_r);

    auto* kind_r { new IntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kKind), kind_r);

    auto* amount { new AmountR(section.amount_decimal, sc_f_.shared_config.default_unit, sc_f_.info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kInitialTotal), amount);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kFinalTotal), amount);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kDiscountTotal), amount);

    auto* quantity_r { new DoubleSpinNoneZeroR(section.quantity_decimal, kCoefficient16, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kMeasureTotal), quantity_r);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kCountTotal), quantity_r);

    auto* name_r { new NodeNameR(sc_p_.tree_model, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kEmployeeId), name_r);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kPartnerId), name_r);

    auto* issued_time { new IssuedTimeR(section.date_format, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kIssuedTime), issued_time);

    auto* status_r { new StatusR(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kStatus), status_r);
}

void MainWindow::TableDelegateF(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const
{
    auto* issued_time { new IssuedTime(config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kIssuedTime), issued_time);

    auto* lhs_rate { new Double(config.rate_decimal, 0.0, kDoubleMax, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kLhsRate), lhs_rate);

    auto* line { new Line(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kCode), line);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kDescription), line);

    auto* document { new Document(sc_->shared_config.document_dir, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kDocument), document);

    auto* status { new Status(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kStatus), status);

    QSortFilterProxyModel* filter_model { tree_model->ExcludeId(node_id, table_view) };

    auto* node { new RhsNode(tree_model, filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kRhsNode), node);

    auto* value { new Double(config.quantity_decimal, kDoubleLowest, kDoubleMax, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kDebit), value);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kCredit), value);

    auto* quantity { new QuantityR(config.quantity_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kBalance), quantity);

    auto* tag { new TagDelegate(sc_f_.tag_hash, sc_f_.tag_pixmap, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kTag), tag);
}

void MainWindow::TableDelegateI(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const
{
    auto* issued_time { new IssuedTime(config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kIssuedTime), issued_time);

    auto* unit_cost { new Double(config.rate_decimal, 0.0, kDoubleMax, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kLhsRate), unit_cost);

    auto* line { new Line(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kCode), line);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kDescription), line);

    auto* document { new Document(sc_->shared_config.document_dir, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kDocument), document);

    auto* status { new Status(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kStatus), status);

    QSortFilterProxyModel* filter_model { tree_model->ExcludeIdUnit(node_id, NodeUnit::IExternal, table_view) };

    auto* node { new RhsNode(tree_model, filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kRhsNode), node);

    auto* value { new Double(config.quantity_decimal, kDoubleLowest, kDoubleMax, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kDebit), value);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kCredit), value);

    auto* quantity { new QuantityR(config.quantity_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kBalance), quantity);

    auto* tag { new TagDelegate(sc_i_.tag_hash, sc_i_.tag_pixmap, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kTag), tag);
}

void MainWindow::TableDelegateT(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const
{
    auto* issued_time { new IssuedTime(config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kIssuedTime), issued_time);

    auto* unit_cost { new Double(config.rate_decimal, 0.0, kDoubleMax, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kLhsRate), unit_cost);

    auto* line { new Line(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kCode), line);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kDescription), line);

    auto* document { new Document(sc_->shared_config.document_dir, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kDocument), document);

    auto* status { new Status(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kStatus), status);

    QSortFilterProxyModel* filter_model { tree_model->ExcludeId(node_id, table_view) };

    auto* node { new RhsNode(tree_model, filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kRhsNode), node);

    auto* value { new Double(config.quantity_decimal, kDoubleLowest, kDoubleMax, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kDebit), value);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kCredit), value);

    auto* quantity { new QuantityR(config.quantity_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kBalance), quantity);

    auto* tag { new TagDelegate(sc_t_.tag_hash, sc_t_.tag_pixmap, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnum::kTag), tag);
}

void MainWindow::TableDelegateP(QTableView* table_view, CSectionConfig& config) const
{
    auto* issued_time { new IssuedTime(config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kIssuedTime), issued_time);

    auto* unit_price { new Double(config.rate_decimal, 0.0, kDoubleMax, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kUnitPrice), unit_price);

    auto* line { new Line(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kCode), line);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kDescription), line);

    auto tree_model_i { sc_i_.tree_model };

    auto* ext_filter_model { tree_model_i->IncludeUnit(NodeUnit::IExternal, table_view) };
    auto* external_sku { new FilterUnit(tree_model_i, ext_filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kExternalSku), external_sku);

    auto* document { new Document(sc_->shared_config.document_dir, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kDocument), document);

    auto* status { new Status(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kStatus), status);

    auto* int_filter_model { tree_model_i->IncludeUnit(NodeUnit::IInternal, table_view) };
    auto* internal_sku { new FilterUnit(tree_model_i, int_filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kRhsNode), internal_sku);

    auto* tag { new TagDelegate(sc_p_.tag_hash, sc_p_.tag_pixmap, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kTag), tag);
}

void MainWindow::TableDelegateO(QTableView* table_view, CSectionConfig& config) const
{
    auto tree_model_i { sc_i_.tree_model };
    auto* int_filter_model { tree_model_i->IncludeUnit(NodeUnit::IInternal, table_view) };

    auto* internal_sku { new FilterUnit(tree_model_i, int_filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kRhsNode), internal_sku);

    auto* name_r { new NodeNameR(sc_i_.tree_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kExternalSku), name_r);

    auto* line { new Line(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kDescription), line);

    auto* price { new Double(config.rate_decimal, 0.0, kDoubleMax, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kUnitPrice), price);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kUnitDiscount), price);

    auto* quantity { new Double(config.quantity_decimal, kDoubleLowest, kDoubleMax, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kCount), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kMeasure), quantity);

    auto* amount { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kInitial), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kDiscount), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kFinal), amount);
}

void MainWindow::DelegateSaleReference(QTableView* table_view, CSectionConfig& config) const
{
    auto* price { new DoubleSpinNoneZeroR(config.rate_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SaleReferenceEnum::kUnitPrice), price);
    table_view->setItemDelegateForColumn(std::to_underlying(SaleReferenceEnum::kUnitDiscount), price);

    auto* quantity { new DoubleSpinNoneZeroR(config.quantity_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SaleReferenceEnum::kCount), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(SaleReferenceEnum::kMeasure), quantity);

    auto* amount { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SaleReferenceEnum::kInitial), amount);

    auto* issued_time { new IssuedTimeR(sc_sale_.section_config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SaleReferenceEnum::kIssuedTime), issued_time);

    if (start_ == Section::kInventory) {
        auto* name { new NodeNameR(sc_p_.tree_model, table_view) };
        table_view->setItemDelegateForColumn(std::to_underlying(SaleReferenceEnum::kNodeId), name);
    }

    if (start_ == Section::kPartner) {
        auto* internal_sku { new NodePathR(sc_i_.tree_model, table_view) };
        table_view->setItemDelegateForColumn(std::to_underlying(SaleReferenceEnum::kNodeId), internal_sku);
    }
}

void MainWindow::DelegateStatement(QTableView* table_view, CSectionConfig& config) const
{
    auto* quantity { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCCount), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCMeasure), quantity);

    auto* amount { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCAmount), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCSettlement), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kPBalance), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCBalance), amount);

    auto* name { new NodeNameR(sc_p_.tree_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kPartner), name);
}

void MainWindow::DelegateStatementNode(QTableView* table_view, CSectionConfig& config) const
{
    auto* quantity { new DoubleSpinNoneZeroR(config.quantity_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementNodeEnum::kCount), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementNodeEnum::kMeasure), quantity);

    auto* amount { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementNodeEnum::kAmount), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementNodeEnum::kSettlement), amount);

    auto* employee { new NodeNameR(sc_p_.tree_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementNodeEnum::kEmployee), employee);

    auto* status { new Status(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementNodeEnum::kStatus), status);

    auto* issued_time { new IssuedTimeR(sc_sale_.section_config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementNodeEnum::kIssuedTime), issued_time);
}

void MainWindow::DelegateStatementEntry(QTableView* table_view, CSectionConfig& config) const
{
    auto* quantity { new DoubleSpinNoneZeroR(config.quantity_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEntryEnum::kCount), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEntryEnum::kMeasure), quantity);

    auto* amount { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEntryEnum::kAmount), amount);

    auto* unit_price { new DoubleSpinNoneZeroR(config.rate_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEntryEnum::kUnitPrice), unit_price);

    auto* status { new Status(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEntryEnum::kStatus), status);

    auto* issued_time { new IssuedTimeR(sc_sale_.section_config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEntryEnum::kIssuedTime), issued_time);

    auto tree_model_i { sc_i_.tree_model };

    auto* external_sku { new NodeNameR(tree_model_i, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEntryEnum::kExternalSku), external_sku);

    auto* int_filter_model { tree_model_i->IncludeUnit(NodeUnit::IInternal, table_view) };
    auto* internal_sku { new FilterUnit(tree_model_i, int_filter_model, table_view) };

    table_view->setItemDelegateForColumn(std::to_underlying(StatementEntryEnum::kInternalSku), internal_sku);
}

void MainWindow::DelegateSettlement(QTableView* table_view, CSectionConfig& config) const
{
    auto* amount { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kAmount), amount);

    auto* status { new Status(QEvent::MouseButtonDblClick, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kStatus), status);

    auto* line { new Line(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kDescription), line);

    auto* issued_time { new IssuedTime(kDateFST, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kIssuedTime), issued_time);

    auto model { sc_p_.tree_model };
    const auto unit { start_ == Section::kSale ? NodeUnit::PCustomer : NodeUnit::PVendor };

    auto* filter_model { model->IncludeUnit(unit, table_view) };
    auto* node { new RhsNode(model, filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kPartner), node);
}

void MainWindow::DelegateSettlementNode(QTableView* table_view, CSectionConfig& config) const
{
    auto* amount { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementItemEnum::kAmount), amount);

    auto* employee { new NodeNameR(sc_p_.tree_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementItemEnum::kEmployee), employee);

    auto* status { new Bool(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementItemEnum::kIsSelected), status);

    auto* issued_time { new IssuedTimeR(kDateFST, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementItemEnum::kIssuedTime), issued_time);
}

void MainWindow::DelegateTagView(QTableView* table_view) const
{
    auto* color { new Color(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(TagEnum::kColor), color);

    auto* line { new Line(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(TagEnum::kName), line);
}
