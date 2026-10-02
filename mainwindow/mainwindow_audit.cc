#include "audit/auditdialog.h"
#include "audit/auditenum.h"
#include "mainwindow.h"
#include "utils/mainwindowutils.h"

void MainWindow::on_actionAuditLog_triggered()
{
    qInfo() << Q_FUNC_INFO;

    const QUuid widget_id { QUuid::createUuidV7() };

    auto* dialog { new AuditDialog(audit_info_, header_info_.audit, widget_id, start_) };

    auto* data_view { dialog->DataView() };
    auto* filter_view { dialog->FilterView() };

    InitFilterView(filter_view, data_view);

    {
        InitTableView(data_view, std::to_underlying(audit::RowField::kPlaceholder));
        DelegateAuditLog(data_view);
        data_view->horizontalHeader()->hide();
    }

    {
        DelegateAuditFilterView(filter_view);
    }

    utils::ManageDialog(widget_hash_, dialog, widget_id);
    dialog->show();
}

void MainWindow::RAuditLogAck(const QUuid& widget_id, const QJsonArray& log_array)
{
    auto widget { widget_hash_.value(widget_id).widget };
    if (!widget)
        return;

    auto* ptr { widget.data() };
    Q_ASSERT(qobject_cast<AuditDialog*>(ptr));

    auto* d_widget { static_cast<AuditDialog*>(ptr) };

    auto* model { d_widget->DataModel() };
    model->Rebuild(log_array);
}

void MainWindow::InitAuditInfo()
{
    using namespace audit;

    audit_info_.section_map = {
        { std::to_underlying(Section::kFinance), tr("Finance") },
        { std::to_underlying(Section::kTask), tr("Task") },
        { std::to_underlying(Section::kInventory), tr("Inventory") },
        { std::to_underlying(Section::kPartner), tr("Partner") },
        { std::to_underlying(Section::kSale), tr("Sale") },
        { std::to_underlying(Section::kPurchase), tr("Purchase") },
    };

    audit_info_.target_type_map = {
        { std::to_underlying(TargetType::kNode), tr("Node") },
        { std::to_underlying(TargetType::kEntry), tr("Entry") },
        { std::to_underlying(TargetType::kSettlement), tr("Settlement") },
    };

    audit_info_.target_operation_map = {
        { std::to_underlying(TargetOperation::kInsert), tr("Insert") },
        { std::to_underlying(TargetOperation::kUpdate), tr("Update") },
        { std::to_underlying(TargetOperation::kDelete), tr("Delete") },
        { std::to_underlying(TargetOperation::kMove), tr("Move") },
        { std::to_underlying(TargetOperation::kReplace), tr("Replace") },
        { std::to_underlying(TargetOperation::kRelease), tr("Release") },
        { std::to_underlying(TargetOperation::kRecall), tr("Recall") },
        { std::to_underlying(TargetOperation::kPeriodClose), tr("Period Close") },
    };

    audit_info_.target_field_map = {
        { std::to_underlying(TargetField::kNone), tr("None") },
        { std::to_underlying(TargetField::kName), tr("Name") },
        { std::to_underlying(TargetField::kDirectionRule), tr("Direction Rule") },
        { std::to_underlying(TargetField::kNumeric), tr("Numeric") },
        { std::to_underlying(TargetField::kRate), tr("Rate") },
        { std::to_underlying(TargetField::kLinkedNode), tr("Linked Node") },
        { std::to_underlying(TargetField::kContent), tr("Content") },
        { std::to_underlying(TargetField::kStatus), tr("Status") },
    };
}
