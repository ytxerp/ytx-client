#include "audit/auditdialog.h"
#include "audit/auditenum.h"
#include "component/constantwebsocket.h"
#include "mainwindow.h"
#include "websocket/jsongen.h"

void MainWindow::on_actionAuditLog_triggered()
{
    qInfo() << Q_FUNC_INFO;

    static QPointer<AuditDialog> dialog {};

    if (!dialog) {
        const QUuid widget_id { QUuid::createUuidV7() };

        dialog = new AuditDialog(audit_info_, widget_id, this);

        {
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            WidgetContext wc { dialog, widget_id, WidgetRole::kDialog };
            widget_hash_.insert(widget_id, wc);
        }

        auto* view { dialog->View() };
        InitTableView(view, std::to_underlying(audit::RowField::kId), -1, std::to_underlying(audit::RowField::kAfter));

        view->horizontalHeader()->setSectionResizeMode(std::to_underlying<>(audit::RowField::kBefore), QHeaderView::Interactive);

        DelegateAuditLog(view);
    }

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void MainWindow::RAuditLogAck(const QUuid& widget_id, const QJsonArray& log_array)
{
    auto widget { widget_hash_.value(widget_id).widget };
    if (!widget)
        return;

    auto* ptr { widget.data() };
    Q_ASSERT(qobject_cast<AuditDialog*>(ptr));

    auto* d_widget { static_cast<AuditDialog*>(ptr) };

    auto* model { d_widget->Model() };
    model->Rebuild(log_array);
}

void MainWindow::InitAuditInfo()
{
    using namespace audit;

    audit_info_.header = {
        tr("ID"),
        tr("Target ID"),
        tr("User"),
        tr("LHS Node"),
        tr("RHS Node"),
        tr("Issued Time"),
        tr("Section"),
        tr("Target"),
        tr("Code"),
        tr("Operation"),
        tr("Level"),
        tr("Before Change"),
        tr("After Change"),
    };

    audit_info_.f_leaf_path = sc_f_.tree_model->LeafPath();
    audit_info_.f_branch_path = sc_f_.tree_model->BranchPath();
    audit_info_.i_leaf_path = sc_i_.tree_model->LeafPath();
    audit_info_.i_branch_path = sc_i_.tree_model->BranchPath();
    audit_info_.p_leaf_path = sc_p_.tree_model->LeafPath();
    audit_info_.p_branch_path = sc_p_.tree_model->BranchPath();
    audit_info_.t_leaf_path = sc_t_.tree_model->LeafPath();
    audit_info_.t_branch_path = sc_t_.tree_model->BranchPath();

    audit_info_.section_hash = {
        { std::to_underlying(Section::kFinance), tr("Finance") },
        { std::to_underlying(Section::kTask), tr("Task") },
        { std::to_underlying(Section::kInventory), tr("Inventory") },
        { std::to_underlying(Section::kPartner), tr("Partner") },
        { std::to_underlying(Section::kSale), tr("Sale") },
        { std::to_underlying(Section::kPurchase), tr("Purchase") },
    };

    audit_info_.target_type_hash = {
        { std::to_underlying(TargetType::kNode), tr("Node") },
        { std::to_underlying(TargetType::kEntry), tr("Entry") },
        { std::to_underlying(TargetType::kSettlement), tr("Settlement") },
    };

    audit_info_.level_hash = {
        { std::to_underlying(Level::kInfo), tr("Info") },
        { std::to_underlying(Level::kWarn), tr("Warn") },
        { std::to_underlying(Level::kCritical), tr("Critical") },
    };

    audit_info_.ws_key_hash = {
        // --- Settlement ---
        { std::to_underlying(WsKey::kSettlementInsert), tr("Insert") },
        { std::to_underlying(WsKey::kSettlementUpdate), tr("Update") },
        { std::to_underlying(WsKey::kSettlementRecall), tr("Recall") },
        { std::to_underlying(WsKey::kSettlementDelete), tr("Delete") },
        // --- Tree ---
        { std::to_underlying(WsKey::kNodeInsert), tr("Insert") },
        { std::to_underlying(WsKey::kNodeUpdate), tr("Update") },
        { std::to_underlying(WsKey::kNodeNameUpdate), tr("Name Update") },
        { std::to_underlying(WsKey::kNodeDrag), tr("Drag") },
        { std::to_underlying(WsKey::kLeafDelete), tr("Delete") },
        { std::to_underlying(WsKey::kLeafDeleteO), tr("Delete O") },
        { std::to_underlying(WsKey::kLeafReplace), tr("Replace") },
        { std::to_underlying(WsKey::kNodeDirectionRuleUpdate), tr("Direction Rule Update") },
        // --- Entry ---
        { std::to_underlying(WsKey::kEntryInsert), tr("Insert") },
        { std::to_underlying(WsKey::kEntryUpdate), tr("Update") },
        { std::to_underlying(WsKey::kEntryDelete), tr("Delete") },
        { std::to_underlying(WsKey::kEntryLinkedNodeUpdate), tr("Linked Node Update") },
        { std::to_underlying(WsKey::kEntryRateUpdate), tr("Rate Update") },
        { std::to_underlying(WsKey::kEntryNumericUpdate), tr("Numeric Update") },
        // --- Order ---
        { std::to_underlying(WsKey::kOrderInsertSave), tr("Insert Save") },
        { std::to_underlying(WsKey::kOrderUpdateSave), tr("Update Save") },
        { std::to_underlying(WsKey::kOrderInsertRelease), tr("Insert Release") },
        { std::to_underlying(WsKey::kOrderUpdateRelease), tr("Update Release") },
        { std::to_underlying(WsKey::kOrderRecall), tr("Recall") },
        // --- Period Close ---
        { std::to_underlying(WsKey::kPeriodClose), tr("Period Close") },
    };
}
