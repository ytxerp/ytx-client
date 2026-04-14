#include "audithub/auditdialog.h"
#include "audithub/auditenum.h"
#include "component/constantwebsocket.h"
#include "global/logininfo.h"
#include "mainwindow.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

void MainWindow::on_actionAuditLog_triggered()
{
    qInfo() << "[UI]" << "on_actionAuditLog_triggered";

    static QPointer<AuditDialog> dialog {};

    if (!dialog) {
        dialog = new AuditDialog(audit_info_, this);

        const auto widget_id { utils::ManageDialog(widget_hash_, dialog) };
        const auto message { JsonGen::AuditLogAck(widget_id, LoginInfo::Instance().Workspace()) };

        WebSocket::Instance()->SendMessage(WsKey::kAuditLogAck, message);

        auto* view { dialog->View() };
        InitTableView(view, std::to_underlying(audit_hub::AuditField::kId), -1, std::to_underlying(audit_hub::AuditField::kAfter));

        view->horizontalHeader()->setSectionResizeMode(std::to_underlying<>(audit_hub::AuditField::kBefore), QHeaderView::Interactive);

        DelegateAuditLog(view);

        connect(dialog, &AuditDialog::SRefresh, this, [widget_id]() {
            const auto message { JsonGen::AuditLogAck(widget_id, LoginInfo::Instance().Workspace()) };
            WebSocket::Instance()->SendMessage(WsKey::kAuditLogAck, message);
        });
    }

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void MainWindow::RAuditLogAck(const QUuid& widget_id, const QJsonArray& log_array, const QJsonArray& user_array)
{
    auto widget { widget_hash_.value(widget_id).widget };
    if (!widget)
        return;

    {
        // Populate user_hash first so model can resolve user_id -> username
        auto& user_hash { audit_info_.user_hash };
        user_hash.clear();

        for (const auto& value : user_array) {
            if (!value.isObject())
                continue;

            const auto obj { value.toObject() };
            const auto id { QUuid(obj.value(kId).toString()) };
            const auto username { obj.value(kUsername).toString() };

            if (!id.isNull())
                user_hash.emplace(id, username);
        }
    }

    {
        auto* ptr { widget.data() };
        Q_ASSERT(qobject_cast<AuditDialog*>(ptr));

        auto* d_widget { static_cast<AuditDialog*>(ptr) };

        auto* model { d_widget->Model() };
        model->ResetModel(log_array);
    }
}

void MainWindow::InitAuditInfo()
{
    using namespace audit_hub;

    audit_info_.header = {
        tr("ID"),
        tr("Target ID"),
        tr("User"),
        tr("LHS Node"),
        tr("RHS Node"),
        tr("Created Time"),
        tr("Section"),
        tr("WS Key"),
        tr("Target Type"),
        tr("Level"),
        tr("Target Code"),
        tr("Before"),
        tr("After"),
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
        { std::to_underlying(AuditLevel::kInfo), tr("Info") },
        { std::to_underlying(AuditLevel::kWarn), tr("Warn") },
        { std::to_underlying(AuditLevel::kCritical), tr("Critical") },
    };

    audit_info_.ws_key_hash = {
        // --- Settlement ---
        { std::to_underlying(WsKey::kSettlementInsert), tr("Settlement Insert") },
        { std::to_underlying(WsKey::kSettlementUpdate), tr("Settlement Update") },
        { std::to_underlying(WsKey::kSettlementRecall), tr("Settlement Recall") },
        { std::to_underlying(WsKey::kSettlementDelete), tr("Settlement Delete") },
        // --- Tree ---
        { std::to_underlying(WsKey::kNodeInsert), tr("Node Insert") },
        { std::to_underlying(WsKey::kNodeNameUpdate), tr("Node Name Update") },
        { std::to_underlying(WsKey::kNodeDrag), tr("Node Drag") },
        { std::to_underlying(WsKey::kLeafDelete), tr("Leaf Delete") },
        { std::to_underlying(WsKey::kLeafDeleteP), tr("Leaf Delete P") },
        { std::to_underlying(WsKey::kLeafDeleteO), tr("Leaf Delete O") },
        { std::to_underlying(WsKey::kLeafReplace), tr("Leaf Replace") },
        { std::to_underlying(WsKey::kBranchDelete), tr("Branch Delete") },
        { std::to_underlying(WsKey::kNodeDirectionRuleUpdate), tr("Node Direction Rule Update") },
        // --- Entry ---
        { std::to_underlying(WsKey::kEntryInsert), tr("Entry Insert") },
        { std::to_underlying(WsKey::kEntryDelete), tr("Entry Delete") },
        { std::to_underlying(WsKey::kEntryLinkedNodeUpdate), tr("Entry Linked Node Update") },
        { std::to_underlying(WsKey::kEntryRateUpdate), tr("Entry Rate Update") },
        { std::to_underlying(WsKey::kEntryNumericUpdate), tr("Entry Numeric Update") },
        { std::to_underlying(WsKey::kEntryIssuedTimeUpdate), tr("Entry Issued Time Update") },
        // --- Order ---
        { std::to_underlying(WsKey::kOrderInsertSave), tr("Order Insert Save") },
        { std::to_underlying(WsKey::kOrderUpdateSave), tr("Order Update Save") },
        { std::to_underlying(WsKey::kOrderInsertRelease), tr("Order Insert Release") },
        { std::to_underlying(WsKey::kOrderUpdateRelease), tr("Order Update Release") },
        { std::to_underlying(WsKey::kOrderRecall), tr("Order Recall") },

    };
}
