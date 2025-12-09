#include "stringinitializer.h"

void StringInitializer::SetHeader(
    SectionInfo& finance, SectionInfo& inventory, SectionInfo& task, SectionInfo& partner, SectionInfo& sale, SectionInfo& purchase)
{
    // Node
    finance.node_header = {
        QObject::tr("Name"),
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("Note"),
        QObject::tr("DirectionRule"),
        QObject::tr("Kind"),
        QObject::tr("Unit"),
        QObject::tr("ForeignTotal"),
        QObject::tr("LocalTotal"),
    };

    inventory.node_header = {
        QObject::tr("Name"),
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("Note"),
        QObject::tr("UnitPrice"),
        QObject::tr("Color"),
        QObject::tr("Commission"),
        QObject::tr("DirectionRule"),
        QObject::tr("Kind"),
        QObject::tr("Unit"),
        QObject::tr("QuantityTotal"),
        QObject::tr("AmountTotal"),
    };

    task.node_header = {
        QObject::tr("Name"),
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("Note"),
        QObject::tr("IssuedTime"),
        QObject::tr("Color"),
        QObject::tr("Document"),
        QObject::tr("Status"),
        QObject::tr("DirectionRule"),
        QObject::tr("Kind"),
        QObject::tr("Unit"),
        QObject::tr("QuantityTotal"),
        QObject::tr("AmountTotal"),
    };

    partner.node_header = {
        QObject::tr("Name"),
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("Note"),
        QObject::tr("PaymentTerm"),
        QObject::tr("Kind"),
        QObject::tr("Unit"),
        QObject::tr("UnpaidTotal"),
    };

    sale.node_header = {
        QObject::tr("GroupName"),
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("Partner"),
        QObject::tr("IssuedTime"),
        QObject::tr("DirectionRule"),
        QObject::tr("Description"),
        QObject::tr("Employee"),
        QObject::tr("Status"),
        QObject::tr("Kind"),
        QObject::tr("Unit"),
        QObject::tr("CountTotal"),
        QObject::tr("MeasureTotal"),
        QObject::tr("GrossTotal"),
        QObject::tr("DiscountTotal"),
        QObject::tr("NetTotal"),
        QObject::tr("IsSettled"),
        QObject::tr("SettlementId"),
    };

    // Entry
    finance.entry_header = {
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("IssuedTime"),
        QObject::tr("LhsNode"),
        QObject::tr("FXRate"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("D"),
        QObject::tr("S"),
        QObject::tr("LinkedNode"),
        QObject::tr("Debit"),
        QObject::tr("Credit"),
        QObject::tr("Balance"),
    };

    inventory.entry_header = {
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("IssuedTime"),
        QObject::tr("LhsNode"),
        QObject::tr("UnitCost"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("D"),
        QObject::tr("S"),
        QObject::tr("LinkedNode"),
        QObject::tr("Debit"),
        QObject::tr("Credit"),
        QObject::tr("Balance"),
    };

    task.entry_header = {
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("IssuedTime"),
        QObject::tr("LhsNode"),
        QObject::tr("UnitCost"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("D"),
        QObject::tr("S"),
        QObject::tr("LinkedNode"),
        QObject::tr("Debit"),
        QObject::tr("Credit"),
        QObject::tr("Balance"),
    };

    partner.entry_header = {
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("IssuedTime"),
        QObject::tr("LhsNode"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("D"),
        QObject::tr("S"),
        QObject::tr("ExternalSku"),
        QObject::tr("UnitPrice"),
        QObject::tr("InternalSku"),
    };

    sale.entry_header = {
        QObject::tr("Id"),
        QObject::tr("InternalSku"),
        QObject::tr("Count"),
        QObject::tr("Measure"),
        QObject::tr("UnitPrice"),
        QObject::tr("Description"),
        QObject::tr("ExternalSku"),
        QObject::tr("LhsNode"),
        QObject::tr("UnitDiscount"),
        QObject::tr("Gross"),
        QObject::tr("Discount"),
        QObject::tr("Net"),
    };

    // Full Entry
    finance.full_entry_header = {
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("IssuedTime"),
        QObject::tr("Code"),
        QObject::tr("LhsNode"),
        QObject::tr("LhsFXRate"),
        QObject::tr("LhsDebit"),
        QObject::tr("LhsCredit"),
        QObject::tr("Description"),
        QObject::tr("D"),
        QObject::tr("S"),
        QObject::tr("RhsCredit"),
        QObject::tr("RhsDebit"),
        QObject::tr("RhsFXRate"),
        QObject::tr("RhsNode"),
    };

    inventory.full_entry_header = {
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("IssuedTime"),
        QObject::tr("Code"),
        QObject::tr("LhsNode"),
        QObject::tr("LhsUnitCost"),
        QObject::tr("LhsDebit"),
        QObject::tr("LhsCredit"),
        QObject::tr("Description"),
        QObject::tr("D"),
        QObject::tr("S"),
        QObject::tr("RhsCredit"),
        QObject::tr("RhsDebit"),
        QObject::tr("RhsUnitCost"),
        QObject::tr("RhsNode"),
    };

    task.full_entry_header = {
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("IssuedTime"),
        QObject::tr("Code"),
        QObject::tr("LhsNode"),
        QObject::tr("LhsUnitCost"),
        QObject::tr("LhsDebit"),
        QObject::tr("LhsCredit"),
        QObject::tr("Description"),
        QObject::tr("D"),
        QObject::tr("S"),
        QObject::tr("RhsCredit"),
        QObject::tr("RhsDebit"),
        QObject::tr("RhsUnitCost"),
        QObject::tr("RhsNode"),
    };

    partner.full_entry_header = partner.entry_header;
    sale.full_entry_header = sale.entry_header;

    // SaleReference
    inventory.node_referenced_header = {
        QObject::tr("IssuedTime"),
        QObject::tr("LhsNode"),
        QObject::tr("Customer"),
        QObject::tr("ExternalSku"),
        QObject::tr("Count"),
        QObject::tr("Measure"),
        QObject::tr("UnitPrice"),
        QObject::tr("UnitDiscount"),
        QObject::tr("Description"),
        QObject::tr("Gross"),
    };

    partner.node_referenced_header = {
        QObject::tr("IssuedTime"),
        QObject::tr("LhsNode"),
        QObject::tr("InternalSku"),
        QObject::tr("ExternalSku"),
        QObject::tr("Count"),
        QObject::tr("Measure"),
        QObject::tr("UnitPrice"),
        QObject::tr("UnitDiscount"),
        QObject::tr("Description"),
        QObject::tr("Gross"),
    };

    // Statement
    sale.statement_header = {
        QObject::tr("Partner"),
        QObject::tr("PBalance"),
        QObject::tr("CCount"),
        QObject::tr("CMeasure"),
        QObject::tr("CAmount"),
        QObject::tr("Description"),
        QObject::tr("CSettlement"),
        QObject::tr("CBalance"),

    };

    sale.statement_node_header = {
        QObject::tr("IssuedTime"),
        QObject::tr("Count"),
        QObject::tr("Measure"),
        QObject::tr("Amount"),
        QObject::tr("Description"),
        QObject::tr("S"),
        QObject::tr("Employee"),
        QObject::tr("Settlement"),
    };

    sale.statement_entry_header = {
        QObject::tr("IssuedTime"),
        QObject::tr("InternalSku"),
        QObject::tr("Count"),
        QObject::tr("Measure"),
        QObject::tr("UnitPrice"),
        QObject::tr("Amount"),
        QObject::tr("Description"),
        QObject::tr("S"),
        QObject::tr("ExternalSku"),
    };

    // Settlement
    sale.settlement_header = {
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("IssuedTime"),
        QObject::tr("Partner"),
        QObject::tr("Description"),
        QObject::tr("Status"),
        QObject::tr("Amount"),
    };

    sale.settlement_node_header = {
        QObject::tr("Id"),
        QObject::tr("SettlementId"),
        QObject::tr("IssuedTime"),
        QObject::tr("Partner"),
        QObject::tr("Description"),
        QObject::tr("Employee"),
        QObject::tr("Amount"),
    };

    purchase.node_header = sale.node_header;
    purchase.entry_header = sale.entry_header;
    purchase.full_entry_header = sale.full_entry_header;
    purchase.statement_header = sale.statement_header;
    purchase.statement_node_header = sale.statement_node_header;
    purchase.statement_entry_header = sale.statement_entry_header;
    purchase.settlement_header = sale.settlement_header;
    purchase.settlement_node_header = sale.settlement_node_header;
}
