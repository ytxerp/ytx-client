/*
 * Copyright (C) 2023 YTX
 *
 * This file is part of YTX.
 *
 * YTX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * YTX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YTX. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <QActionGroup>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTableView>
#include <QTranslator>

#include "audit/auditinfo.h"
#include "component/config.h"
#include "component/headerinfo.h"
#include "component/sectioncontex.h"
#include "component/sectioninfo.h"
#include "entryhub/entryhub.h"
#include "order/settlement/settlement.h"
#include "order/settlement/settlementprimarymodel.h"
#include "order/statement/statementtertiarywidget.h"
#include "table/model/tablemodel.h"
#include "table/model/tablemodelo.h"
#include "table/widget/tablewidgetfipt.h"
#include "table/widget/tablewidgeto.h"
#include "tree/model/treemodel.h"
#include "tree/widget/treewidgeto.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void on_actionInsertNode_triggered();
    void on_actionAppendEntry_triggered();
    void on_actionDelete_triggered();
    void on_actionAppendNode_triggered();
    void on_actionRename_triggered();
    void on_actionClearColor_triggered();
    void on_actionNewBranch_triggered();
    void on_actionJumpEntry_triggered();
    void on_actionAbout_triggered();
    void on_actionPreferences_triggered();

    void on_actionSearch_triggered();
    void on_actionTags_triggered();

    void on_actionSignIn_triggered();
    void on_actionSignOut_triggered();
    void on_actionReconnect_triggered();

    void on_actionProfile_triggered();
    void on_actionWorkspaceManager_triggered();

    void on_actionCheckUpdates_triggered();

    void on_actionStatement_triggered();
    void on_actionSettlement_triggered();
    void on_actionAuditLog_triggered();
    void on_actionPeriodClose_triggered();

    void on_actionHeatInventory_triggered();
    void on_actionHeatPartner_triggered();
    void on_actionBalanceSheet_triggered();
    void on_actionIncomeStatement_triggered();
    void on_actionCashFlowStatement_triggered();
    void on_actionSettlementView_triggered();
    void on_actionOrderOverview_triggered();

    void on_actionMarkAll_triggered();
    void on_actionMarkNone_triggered();
    void on_actionMarkToggle_triggered();
    void on_actionLockAll_triggered();
    void on_actionUnlockAll_triggered();

    void tabWidget_currentChanged();
    void tabWidget_tabBarDoubleClicked(int index);
    void tabWidget_tabCloseRequestedFIPT(int index);
    void tabWidget_tabCloseRequestedO(int index);

    void RLocateNode(Section section, QUuid node_id);
    void REntryLocation(QUuid entry_id, QUuid lhs_node_id, QUuid rhs_node_id);
    void RSelectNode(Section section, QUuid node_id);

    void RUpdateConfig(const AppConfig& app, const SharedConfig& shared, const SectionConfig& section);
    void RSyncPartner(QUuid parent_id, QUuid node_id, QUuid partner_id);
    void RUpdatePartner(QUuid widget_id, QUuid partner_id);
    void RUpdateName(QUuid node_id, const QString& name, bool branch);
    void RMessage(QMessageBox::Icon icon, const QString& message);

    void RAccountName(const QString& name);
    void RAccountUsername(const QJsonObject& obj);

    void RStatementMarkEntries(MarkOperation operation);

    void RDenyConnection();
    void RAllowConnection();
    void RRemoteHostClosed();

    void RAllowLogin(const QString& name, const QString& expire_date);
    void RDenyLogin();
    void RFinishTreeSync();

    void RFreeWidget(Section section, QUuid node_id);
    void RFlushCaches();

    void ROrderOverview(Section section, QUuid widget_id, const QJsonArray& array);

    void RStatementPrimary(Section section, QUuid widget_id, const QJsonArray& array);
    void RStatemetSecondary(Section section, QUuid widget_id, const QJsonArray& array);
    void RStatementTertiary(Section section, QUuid widget_id, const QJsonArray& array);

    void RTreeViewCustomContextMenuRequested(const QPoint& pos);
    void RInsertNodeTag(const tag::Row& tag, TreeModel* model, const Node* node);
    void RRemoveNodeTag(const tag::Row& tag, TreeModel* model, const Node* node);

    void RTreeViewDoubleClicked(const QModelIndex& index);
    void ROpenSettlementSecondaryWidget(const QModelIndex& index);
    void RCreateSettlementSecondaryWidget(settlement::PrimaryModel* model);

    void RSectionGroup(int id);

    void RShowSecondaryStatement(QUuid partner_id, const utils::DateRange& range, int unit);
    void RShowTertiaryStatement(QUuid partner_id, const utils::DateRange& range, int unit);

    void RSettlementPrimary(Section section, QUuid widget_id, const QJsonArray& array);
    void RSettlementSecondary(Section section, QUuid widget_id, const QJsonArray& array);
    void RInsertSettlement(const QJsonObject& obj);
    void RRecallSettlement(const QJsonObject& obj);
    void RUpdateSettlement(const QJsonObject& obj);
    void RSettlementView(Section section, QUuid widget_id, const QJsonArray& array);

    void RReleaseOrder(Section section, QUuid node_id, int version);
    void RRecallOrder(Section section, QUuid node_id, int version);
    void RSaveOrder(Section section, QUuid node_id, int version);
    void RDenyOperation();
    void RPermissionDeny();
    void ROrderPermissionDeny(Section section, QUuid widget_id);
    void RSettlementPermissionDeny(Section section, QUuid widget_id);

    void RDenyLeafDelete(const QJsonObject& obj);
    void RLeafDeleteLockedDeny(Section section, QUuid node_id);

    void RWorkspaceMemberAck(QUuid widget_id, const QJsonArray& array);
    void RAuditLogAck(QUuid widget_id, const QJsonArray& array);
    void RInventoryHeatAck(QUuid widget_id, const QJsonArray& array);
    void RPartnerHeatAck(QUuid widget_id, const QJsonArray& array);
    void RBalanceSheetAck(QUuid widget_id, const QJsonArray& node_array, const QJsonArray& path_array);
    void RIncomeStatementAck(QUuid widget_id, const QJsonObject& obj);
    void RCashFlowStatementAck(QUuid widget_id, const QJsonObject& obj);

    void RAccountRoleUpdate();

    void RApplyTag(const QJsonObject& obj);
    void RInsertTag(const QJsonObject& obj);
    void RUpdateTag(const QJsonObject& obj);
    void RDeleteTag(const QJsonObject& obj);

    void RTableViewCustomContextMenuRequested(const QPoint& pos);
    void RInsertEntryTag(const tag::Row& tag, TableModel* model, const Entry* entry);
    void RRemoveEntryTag(const tag::Row& tag, TableModel* model, const Entry* entry);

    void RApplySharedConfig(const QJsonArray& arr);
    void RUpdateDocumentDir(Section section, const QString& document_dir);
    void RUpdateDefaultUnit(Section section, int unit);
    void RDenyDefaultUnit();
    void RSelectEntry(QUuid node_id, QUuid entry_id);
    void REntryLocked();

private:
    void SetTabWidget(QTabWidget* tab_widget);
    void ResetMainwindow();

    void SetUniqueConnection() const;
    void SetIcon() const;

    void InitilizeContext();
    void InitContextFinance();
    void InitContextInventory();
    void InitContextPartner();
    void InitContextTask();
    void InitContextSale();
    void InitContextPurchase();

    void ShowLeafWidget(QUuid node_id, QUuid entry_id = {});

    void CreateSettlementSecondary(const settlement::PrimaryRow& primary_row, settlement::PrimaryModel* primary_model);

    void TableDelegateF(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, QUuid node_id) const;
    void TableDelegateI(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, QUuid node_id) const;
    void TableDelegateT(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, QUuid node_id) const;
    void TableDelegateP(QTableView* table_view, CSectionConfig& config) const;
    void TableDelegateO(QTableView* table_view, CSectionInfo& info, CSectionConfig& config) const;

    void SetTableView(QTableView* view, Section section, int stretch_column) const;

    void TableConnectF(TableModel* table_model) const;
    void TableConnectI(TableModel* table_model) const;
    void TableConnectT(TableModel* table_model) const;
    void TableConnectP(TableModel* table_model) const;
    void TableConnectO(TableModelO* table_model_o, TableWidgetO* widget) const;

    void DelegateStatementPrimary(QTableView* table_view, CSectionConfig& config) const;
    void DelegateStatementSecondary(QTableView* table_view, CSectionConfig& config) const;
    void DelegateStatementSecondarySummary(QTableView* table_view, CSectionConfig& config) const;
    void DelegateStatementTertiary(QTableView* table_view, CSectionConfig& config) const;
    void DelegateStatementTertiarySummary(QTableView* table_view, CSectionConfig& config) const;
    void DelegateSettlementPrimary(QTableView* table_view, CSectionConfig& config) const;
    void DelegateSettlementSecondary(QTableView* table_view, CSectionConfig& config) const;
    void DelegateTag(QTableView* table_view) const;
    void DelegateWorkspaceMember(QTableView* table_view) const;
    void DelegateAuditLog(QTableView* table_view) const;
    void DelegatePeriodClose(QTableView* table_view) const;
    void DelegateInventoryHeat(QTableView* table_view) const;
    void DelegatePartnerHeat(QTableView* table_view) const;
    void DelegateBalanceSheet(QTreeView* view) const;
    void DelegateIncomeStatement(QTreeView* view) const;
    void DelegateCashFlowStatement(QTreeView* view) const;
    void DelegateCashFlowStatementWrong(QTableView* view) const;
    void DelegateCashFlowStatementCarrier(QTreeView* view) const;
    void DelegateOrderFilterView(QTableView* view, const SectionInfo& info) const;
    void DelegateOrderOverview(QTableView* view) const;
    void DelegateAuditFilterView(QTableView* view) const;
    void DelegateStatementFilterView(QTableView* view) const;

    void InitTableView(QTableView* view, int stretch_column) const;
    void InitTreeView(QTreeView* view, int id_column, int stretch_column) const;
    void InitFilterView(QTableView* filter_view, QTableView* data_view) const;
    void InitSummaryView(QTableView* summary_view, QTableView* data_view) const;

    void EditNameFIPT();
    void EditNameO();

    void SetTreeView(QTreeView* view, Section section) const;
    void SetTreeHeader(QTreeView* view, Section section);

    void TreeDelegateF(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const;
    void TreeDelegateT(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const;
    void TreeDelegateI(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const;
    void TreeDelegateP(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const;
    void TreeDelegateO(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const;

    void TreeConnectF(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const;
    void TreeConnectI(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const;
    void TreeConnectT(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const;
    void TreeConnectP(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const;
    void TreeConnectO(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub, Section section) const;

    // Section
    SectionContext* GetSectionContext(Section section);
    void CreateSection(const SectionContext* sc, CString& name) const;

    // Node
    void InsertNodeFunction(SectionContext* sc, const QModelIndex& parent_index);
    void InsertNodeFIPT(SectionContext* sc, const QModelIndex& parent_index);
    void InsertNodeO(SectionContext* sc, const QModelIndex& parent_index);

    // Table
    void CreateLeafFIPT(SectionContext* sc, QUuid node_id);
    void CreateLeafO(SectionContext* sc, QUuid node_id);

    // Widget
    void RegisterWidget(SectionContext* sc, QWidget* widget, QUuid widget_id, WidgetRole role);
    void FlushCaches(const SectionContext* sc) const;

    // Tag
    QIcon GetTagIcon(SectionContext* sc, const tag::Row& tag, bool checked);
    void UpdateTagIcon(SectionContext* sc, const tag::Row& tag);

    // Partner
    void UpdatePartnerReference(const SectionContext* sc, const QSet<QUuid>& partner_nodes, bool branch) const;

    void DeleteNode();
    void DeleteEntry(TableWidget* widget);
    void DeleteBranch(Node* node);

    void LoadAndInstallTranslator(CString& language);
    void ResizeColumn(QHeaderView* header, int stretch_column) const;

    void ReadLocalConfig();
    void ReadSectionConfig(SectionConfig& config, CString& section_name);

    void UpdateAppConfig(CAppConfig& app);
    void UpdateSectionConfig(CSectionConfig& section) const;
    void UpdateSharedConfig(CSharedConfig& shared);

    void UpdateAccountInfo(const QString& workspace, const QString& name, const QString& expire_date);

    void DisableActions() const;
    void SetLoginActions(bool enable) const;
    void UpdateSessionActions(ConnectionState connection, LoginState login) const;

    void IniSectionGroup();

    void FocusTabWidget(QUuid node_id) const;

    void WriteConfig();

    void MarkEntries(MarkOperation operation);
    void LockEntries(bool locked);

    void InitSystemTray();
    void InitStatusLabel();
    void InitHeader();
    void InitHeader(SectionInfo& finance, SectionInfo& inventory, SectionInfo& task, SectionInfo& partner, SectionInfo& sale, SectionInfo& purchase);
    void InitAuditInfo();

    QSet<QString> ChildrenName(const Node* node) const;
    QSet<QUuid> LeafChildrenId(const Node* node) const;

    void UpdateLeafNodeName(QUuid node_id, const QString& name);
    void UpdateOneTab(QUuid node_id, const QString& name);

    void UpdateBranchNodeName(QUuid node_id);
    void UpdateMultiTabs(const QSet<QUuid>& nodes);

    inline bool IsTreeWidget(const QWidget* widget) { return widget && widget->inherits(kTreeWidget); }
    inline bool IsTreeWidgetO(const QWidget* widget) { return widget && widget->inherits(kTreeWidgetO); }
    inline bool IsTableWidgetFIPT(const QWidget* widget) { return widget && widget->inherits(kTableWidgetFIPT); }
    inline bool IsTableWidgetO(const QWidget* widget) { return widget && widget->inherits(kTableWidgetO); }
    inline bool IsStatementTertiaryWidget(const QWidget* widget) { return widget && widget->inherits(kStatementTertiaryWidget); }

private:
    Ui::MainWindow* ui {};
    QHash<QUuid, WidgetContext> widget_hash_ {};

    HeaderInfo header_info_ {};
    audit::Info audit_info_ {};

    Section start_ {};
    QLabel* connection_label_ {};
    QLabel* login_label_ {};

    QSystemTrayIcon* tray_icon_ {};
    QMenu* tray_menu_ {};

    QTranslator qt_translator_ {};
    QTranslator ytx_translator_ {};

    AppConfig app_config_ {};

    QSharedPointer<QSettings> app_settings_ {};
    QSharedPointer<QSettings> section_settings_ {};
    QNetworkAccessManager* network_manager_ {};

    QButtonGroup* section_group_ {};

    SectionContext* sc_ {};

    SectionContext sc_f_ {};
    SectionContext sc_i_ {};
    SectionContext sc_t_ {};
    SectionContext sc_p_ {};
    SectionContext sc_sale_ {};
    SectionContext sc_purchase_ {};
};
