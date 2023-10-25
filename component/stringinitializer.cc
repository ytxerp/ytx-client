#include "stringinitializer.h"

void StringInitializer::SetHeader(
    SectionInfo& finance, SectionInfo& item, SectionInfo& task, SectionInfo& stakeholder, SectionInfo& sale, SectionInfo& purchase)
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

    item.node_header = {
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
        QObject::tr("Color"),
        QObject::tr("UnitPrice"),
        QObject::tr("Commission"),
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
        QObject::tr("DirectionRule"),
        QObject::tr("Kind"),
        QObject::tr("Unit"),
        QObject::tr("Color"),
        QObject::tr("Document"),
        QObject::tr("IssuedTime"),
        QObject::tr("IsFinished"),
        QObject::tr("QuantityTotal"),
        QObject::tr("AmountTotal"),
    };

    stakeholder.node_header = {
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
        QObject::tr("TradeTotal"),
        QObject::tr("UnpaidTradeTotal"),
    };

    sale.node_header = {
        QObject::tr("Name"),
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("Description"),
        QObject::tr("DirectionRule"),
        QObject::tr("Kind"),
        QObject::tr("Unit"),
        QObject::tr("Party"),
        QObject::tr("Employee"),
        QObject::tr("IssuedTime"),
        QObject::tr("FirstTotal"),
        QObject::tr("SecondTotal"),
        QObject::tr("IsFinished"),
        QObject::tr("InitialTotal"),
        QObject::tr("DiscountTotal"),
        QObject::tr("FinalTotal"),
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
        QObject::tr("C"),
        QObject::tr("RelatedNode"),
        QObject::tr("Debit"),
        QObject::tr("Credit"),
        QObject::tr("Balance"),
    };

    item.entry_header = {
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
        QObject::tr("C"),
        QObject::tr("RelatedNode"),
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
        QObject::tr("C"),
        QObject::tr("RelatedNode"),
        QObject::tr("Debit"),
        QObject::tr("Credit"),
        QObject::tr("Balance"),
    };

    stakeholder.entry_header = {
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("IssuedTime"),
        QObject::tr("LhsNode"),
        QObject::tr("UnitPrice"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("D"),
        QObject::tr("C"),
        QObject::tr("Internal"),
        QObject::tr("External"),
    };

    sale.entry_header = {
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("Internal"),
        QObject::tr("First"),
        QObject::tr("Second"),
        QObject::tr("UnitPrice"),
        QObject::tr("Description"),
        QObject::tr("ExternalItem"),
        QObject::tr("LhsNode"),
        QObject::tr("DiscountPrice"),
        QObject::tr("Color"),
        QObject::tr("GrossAmount"),
        QObject::tr("Discount"),
        QObject::tr("NetAmount"),
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
        QObject::tr("C"),
        QObject::tr("RhsCredit"),
        QObject::tr("RhsDebit"),
        QObject::tr("RhsFXRate"),
        QObject::tr("RhsNode"),
    };

    item.full_entry_header = {
        QObject::tr("Id"),
        QObject::tr("UserId"),
        QObject::tr("CreatedTime"),
        QObject::tr("CreatedBy"),
        QObject::tr("UpdatedTime"),
        QObject::tr("UpdatedBy"),
        QObject::tr("IssuedTime"),
        QObject::tr("Code"),
        QObject::tr("LhsNode"),
        QObject::tr("LhsDebit"),
        QObject::tr("LhsCredit"),
        QObject::tr("Description"),
        QObject::tr("UnitCost"),
        QObject::tr("D"),
        QObject::tr("C"),
        QObject::tr("RhsCredit"),
        QObject::tr("RhsDebit"),
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
        QObject::tr("LhsDebit"),
        QObject::tr("LhsCredit"),
        QObject::tr("Description"),
        QObject::tr("UnitCost"),
        QObject::tr("D"),
        QObject::tr("C"),
        QObject::tr("RhsCredit"),
        QObject::tr("RhsDebit"),
        QObject::tr("RhsNode"),
    };

    stakeholder.full_entry_header = stakeholder.entry_header;
    sale.full_entry_header = sale.entry_header;

    // EntryRef
    item.entry_ref_header = {
        QObject::tr("IssuedTime"),
        QObject::tr("LhsNode"),
        QObject::tr("Section"),
        QObject::tr("Party"),
        QObject::tr("ExternalItem"),
        QObject::tr("First"),
        QObject::tr("Second"),
        QObject::tr("UnitPrice"),
        QObject::tr("DiscountPrice"),
        QObject::tr("Description"),
        QObject::tr("GrossAmount"),
    };

    stakeholder.entry_ref_header = {
        QObject::tr("IssuedTime"),
        QObject::tr("LhsNode"),
        QObject::tr("Section"),
        QObject::tr("Internal"),
        QObject::tr("ExternalItem"),
        QObject::tr("First"),
        QObject::tr("Second"),
        QObject::tr("UnitPrice"),
        QObject::tr("DiscountPrice"),
        QObject::tr("Description"),
        QObject::tr("GrossAmount"),
    };

    // Statement
    sale.statement_header = {
        QObject::tr("Party"),
        QObject::tr("PBalance"),
        QObject::tr("CFirst"),
        QObject::tr("CSecond"),
        QObject::tr("CGrossAmount"),
        QObject::tr("CBalance"),
        QObject::tr("Description"),
        QObject::tr("CSettlement"),
    };

    sale.statement_primary_header = {
        QObject::tr("IssuedTime"),
        QObject::tr("First"),
        QObject::tr("Second"),
        QObject::tr("GrossAmount"),
        QObject::tr("C"),
        QObject::tr("Description"),
        QObject::tr("Employee"),
        QObject::tr("Settlement"),
    };

    sale.statement_secondary_header = {
        QObject::tr("IssuedTime"),
        QObject::tr("Internal"),
        QObject::tr("First"),
        QObject::tr("Second"),
        QObject::tr("UnitPrice"),
        QObject::tr("GrossAmount"),
        QObject::tr("C"),
        QObject::tr("Description"),
        QObject::tr("External"),
        QObject::tr("Settlement"),
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
        QObject::tr("Party"),
        QObject::tr("Description"),
        QObject::tr("IsFinished"),
        QObject::tr("GrossAmount"),
    };

    sale.settlement_primary_header = {
        QObject::tr("IssuedTime"),
        QObject::tr("Id"),
        QObject::tr("Employee"),
        QObject::tr("Description"),
        QObject::tr("C"),
        QObject::tr("GrossAmount"),
    };

    purchase.node_header = sale.node_header;
    purchase.entry_header = sale.entry_header;
    purchase.full_entry_header = sale.full_entry_header;
    purchase.statement_header = sale.statement_header;
    purchase.statement_primary_header = sale.statement_primary_header;
    purchase.statement_secondary_header = sale.statement_secondary_header;
    purchase.settlement_header = sale.settlement_header;
    purchase.settlement_primary_header = sale.settlement_primary_header;
}
