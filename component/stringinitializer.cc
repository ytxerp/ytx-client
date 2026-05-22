#include "stringinitializer.h"

void StringInitializer::SetHeader(
    SectionInfo& finance, SectionInfo& inventory, SectionInfo& task, SectionInfo& partner, SectionInfo& sale, SectionInfo& purchase)
{
    // Node
    finance.node_header = {
        QObject::tr("Name"),
        QObject::tr("ID"),
        QObject::tr("Version"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("Tag"),
        QObject::tr("Color"),
        QObject::tr("Document"),
        QObject::tr("Direction Rule"),
        QObject::tr("Roles"),
        QObject::tr("Cash Kind"),
        QObject::tr("Kind"),
        QObject::tr("Unit"),
        QObject::tr("Foreign Total"),
        QObject::tr("Local Total"),
    };

    inventory.node_header = {
        QObject::tr("Name"),
        QObject::tr("ID"),
        QObject::tr("Version"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("Tag"),
        QObject::tr("Color"),
        QObject::tr("Document"),
        QObject::tr("Unit Price"),
        QObject::tr("Commission"),
        QObject::tr("Direction Rule"),
        QObject::tr("Kind"),
        QObject::tr("Unit"),
        QObject::tr("Quantity Total"),
        QObject::tr("Amount Total"),
    };

    task.node_header = {
        QObject::tr("Name"),
        QObject::tr("ID"),
        QObject::tr("Version"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("Tag"),
        QObject::tr("Color"),
        QObject::tr("Document"),
        QObject::tr("Direction Rule"),
        QObject::tr("Kind"),
        QObject::tr("Unit"),
        QObject::tr("Quantity Total"),
        QObject::tr("Amount Total"),
    };

    partner.node_header = {
        QObject::tr("Name"),
        QObject::tr("ID"),
        QObject::tr("Version"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("Tag"),
        QObject::tr("Color"),
        QObject::tr("Document"),
        QObject::tr("Payment Term"),
        QObject::tr("Kind"),
        QObject::tr("Unit"),
        QObject::tr("Unpaid Total"),
    };

    sale.node_header = {
        QObject::tr("Group Name"),
        QObject::tr("ID"),
        QObject::tr("Version"),
        QObject::tr("Partner"),
        QObject::tr("Issued Time"),
        QObject::tr("Employee"),
        QObject::tr("Status"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("Tag"),
        QObject::tr("Direction Rule"),
        QObject::tr("Kind"),
        QObject::tr("Unit"),
        QObject::tr("Count Total"),
        QObject::tr("Measure Total"),
        QObject::tr("Gross Total"),
        QObject::tr("Discount Total"),
        QObject::tr("Net Total"),
        QObject::tr("Is Settled"),
        QObject::tr("Settlement ID"),
    };

    // Entry
    finance.entry_header = {
        QObject::tr("ID"),
        QObject::tr("Version"),
        QObject::tr("LHS Node"),
        QObject::tr("Issued Time"),
        QObject::tr("FX Rate"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("Tag"),
        QObject::tr("Document"),
        QObject::tr("Status"),
        QObject::tr("Linked Node"),
        QObject::tr("Debit"),
        QObject::tr("Credit"),
        QObject::tr("Balance"),
    };

    inventory.entry_header = {
        QObject::tr("ID"),
        QObject::tr("Version"),
        QObject::tr("LHS Node"),
        QObject::tr("Issued Time"),
        QObject::tr("Unit Cost"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("Tag"),
        QObject::tr("Document"),
        QObject::tr("Status"),
        QObject::tr("Linked Node"),
        QObject::tr("Debit"),
        QObject::tr("Credit"),
        QObject::tr("Balance"),
    };

    task.entry_header = {
        QObject::tr("ID"),
        QObject::tr("Version"),
        QObject::tr("LHS Node"),
        QObject::tr("Issued Time"),
        QObject::tr("Unit Cost"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("Tag"),
        QObject::tr("Document"),
        QObject::tr("Status"),
        QObject::tr("Linked Node"),
        QObject::tr("Debit"),
        QObject::tr("Credit"),
        QObject::tr("Balance"),
    };

    partner.entry_header = {
        QObject::tr("ID"),
        QObject::tr("Version"),
        QObject::tr("LHS Node"),
        QObject::tr("Issued Time"),
        QObject::tr("Code"),
        QObject::tr("Description"),
        QObject::tr("Tag"),
        QObject::tr("Document"),
        QObject::tr("Status"),
        QObject::tr("Internal SKU"),
        QObject::tr("Unit Price"),
        QObject::tr("External SKU"),
    };

    sale.entry_header = {
        QObject::tr("ID"),
        QObject::tr("Version"),
        QObject::tr("LHS Node"),
        QObject::tr("Internal SKU"),
        QObject::tr("Description"),
        QObject::tr("Tag"),
        QObject::tr("Count"),
        QObject::tr("Measure"),
        QObject::tr("Unit Price"),
        QObject::tr("External SKU"),
        QObject::tr("Unit Discount"),
        QObject::tr("Gross"),
        QObject::tr("Discount"),
        QObject::tr("Net"),
    };

    // Full Entry
    finance.full_entry_header = {
        QObject::tr("ID"),
        QObject::tr("Version"),
        QObject::tr("Issued Time"),
        QObject::tr("Code"),
        QObject::tr("LHS Node"),
        QObject::tr("LHS FX Rate"),
        QObject::tr("LHS Debit"),
        QObject::tr("LHS Credit"),
        QObject::tr("Description"),
        QObject::tr("Tag"),
        QObject::tr("Document"),
        QObject::tr("Status"),
        QObject::tr("RHS Credit"),
        QObject::tr("RHS Debit"),
        QObject::tr("RHS FX Rate"),
        QObject::tr("RHS Node"),
    };

    inventory.full_entry_header = {
        QObject::tr("ID"),
        QObject::tr("Version"),
        QObject::tr("Issued Time"),
        QObject::tr("Code"),
        QObject::tr("LHS Node"),
        QObject::tr("LHS Unit Cost"),
        QObject::tr("LHS Debit"),
        QObject::tr("LHS Credit"),
        QObject::tr("Description"),
        QObject::tr("Tag"),
        QObject::tr("Document"),
        QObject::tr("Status"),
        QObject::tr("RHS Credit"),
        QObject::tr("RHS Debit"),
        QObject::tr("RHS Unit Cost"),
        QObject::tr("RHS Node"),
    };

    task.full_entry_header = {
        QObject::tr("ID"),
        QObject::tr("Version"),
        QObject::tr("Issued Time"),
        QObject::tr("Code"),
        QObject::tr("LHS Node"),
        QObject::tr("LHS Unit Cost"),
        QObject::tr("LHS Debit"),
        QObject::tr("LHS Credit"),
        QObject::tr("Description"),
        QObject::tr("Tag"),
        QObject::tr("Document"),
        QObject::tr("Status"),
        QObject::tr("RHS Credit"),
        QObject::tr("RHS Debit"),
        QObject::tr("RHS Unit Cost"),
        QObject::tr("RHS Node"),
    };

    partner.full_entry_header = partner.entry_header;
    sale.full_entry_header = sale.entry_header;

    // SaleReference
    inventory.node_referenced_header = {
        QObject::tr("Issued Time"),
        QObject::tr("LHS Node"),
        QObject::tr("Customer"),
        QObject::tr("Count"),
        QObject::tr("Measure"),
        QObject::tr("Unit Price"),
        QObject::tr("Description"),
        QObject::tr("Gross"),
    };

    partner.node_referenced_header = {
        QObject::tr("Issued Time"),
        QObject::tr("LHS Node"),
        QObject::tr("Internal SKU"),
        QObject::tr("Color"),
        QObject::tr("External SKU"),
        QObject::tr("Count"),
        QObject::tr("Measure"),
        QObject::tr("Unit Price"),
        QObject::tr("Description"),
        QObject::tr("Gross"),
    };

    purchase.node_header = sale.node_header;
    purchase.entry_header = sale.entry_header;
    purchase.full_entry_header = sale.full_entry_header;
}