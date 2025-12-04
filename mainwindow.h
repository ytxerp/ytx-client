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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QActionGroup>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTableView>
#include <QTranslator>

#include "component/config.h"
#include "component/info.h"
#include "component/sectioncontex.h"
#include "entryhub/entryhub.h"
#include "report/widget/settlementwidget.h"
#include "table/model/tablemodel.h"
#include "table/model/tablemodelo.h"
#include "table/widget/tablewidgetfipt.h"
#include "table/widget/tablewidgeto.h"
#include "tree/model/treemodel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void on_actionInsertNode_triggered();
    void on_actionAppendEntry_triggered();
    void on_actionRemove_triggered();
    void on_actionAppendNode_triggered();
    void on_actionEditName_triggered();
    void on_actionResetColor_triggered();
    void on_actionNewGroup_triggered();
    void on_actionJump_triggered();
    void on_actionAbout_triggered();
    void on_actionPreferences_triggered();
    void on_actionSearch_triggered();

    void on_actionLogin_triggered();
    void on_actionLogout_triggered();
    void on_actionReconnect_triggered();

    void on_actionCheckforUpdates_triggered();
    void on_actionExportExcel_triggered();

    void on_actionStatement_triggered();
    void on_actionSettlement_triggered();

    void on_tabWidget_currentChanged(int);
    void on_tabWidget_tabBarDoubleClicked(int index);
    void on_tabWidget_tabCloseRequested(int index);

    bool RInitializeContext(const QString& expire_date);
    void RNodeLocation(const QUuid& node_id);
    void REntryLocation(const QUuid& entry_id, const QUuid& lhs_node_id, const QUuid& rhs_node_id);

    void RUpdateConfig(const AppConfig& app, const SharedConfig& shared, const SectionConfig& section);
    void RSyncPartner(const QUuid& node_id, const QVariant& value);
    void RUpdateName(const QUuid& node_id, const QString& name, bool branch);
    void RActionEntry(EntryAction action);
    void RLoginResult(bool result);
    void RConnectionAccepted();
    void RConnectionRefused();
    void RRemoteHostClosed();

    void RFreeWidget(const QUuid& node_id);

    void RSaleReferencePrimary(const QUuid& node_id, int unit);
    void RSaleReferenceSecondary(const QModelIndex& index);

    void RTreeViewCustomContextMenuRequested(const QPoint& pos);
    void RTreeViewDoubleClicked(const QModelIndex& index);

    void RSectionGroup(int id);

    void RStatementPrimary(const QUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end);
    void RStatementSecondary(const QUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end);

    void RLeafRemoveDenied(const QJsonObject& obj);
    inline void RNodeRemoveConfirmed(const QUuid& node_id) { node_pending_removal_.remove(node_id); }

    void RSharedConfig(const QJsonArray& arr);
    void RDocumentDir(Section section, const QString& document_dir);
    void RDefaultUnit(Section section, int unit);
    void RUpdateDefaultUnitFailed(const QString& section);
    void RSelectLeafEntry(const QUuid& node_id, const QUuid& entry_id);

private:
    void SetTabWidget();
    void ClearMainwindow();

    void SetUniqueConnection() const;
    void SetIcon() const;

    void InitContextFinance();
    void InitContextInventory();
    void InitContextPartner();
    void InitContextTask();
    void InitContextSale();
    void InitContextPurchase();

    void ShowLeafWidget(const QUuid& node_id, const QUuid& entry_id = {});

    void CreateLeafFIPT(SectionContext* sc, const QUuid& node_id);
    void CreateLeafO(SectionContext* sc, const QUuid& node_id);

    void TableDelegateF(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const;
    void TableDelegateI(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const;
    void TableDelegateT(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const;
    void TableDelegateP(QTableView* table_view, CSectionConfig& config) const;
    void TableDelegateO(QTableView* table_view, CSectionConfig& config) const;

    void SetTableViewFIPT(QTableView* table_view, int stretch_column, int lhs_node_column) const;
    void SetTableViewO(QTableView* table_view, Section section, int stretch_column, int lhs_node_column) const;

    void TableConnectF(QTableView* table_view, TableModel* table_model) const;
    void TableConnectI(QTableView* table_view, TableModel* table_model) const;
    void TableConnectT(QTableView* table_view, TableModel* table_model) const;
    void TableConnectP(QTableView* table_view, TableModel* table_model) const;
    void TableConnectO(QTableView* table_view, TableModelO* table_model, TableWidgetO* widget) const;

    void CreateSaleReference(TreeModel* tree_model, CSectionInfo& info, const QUuid& node_id, int unit);
    void DelegateSaleReference(QTableView* table_view, CSectionConfig& config) const;
    void SetTableViewSaleReference(QTableView* table_view) const;

    void SetStatementView(QTableView* table_view, int stretch_column) const;
    void DelegateStatement(QTableView* table_view, CSectionConfig& config) const;
    void DelegateSettlement(QTableView* table_view, CSectionConfig& config) const;
    void DelegateSettlementPrimary(QTableView* table_view, CSectionConfig& config) const;

    void DelegateStatementPrimary(QTableView* table_view, CSectionConfig& config) const;
    void DelegateStatementSecondary(QTableView* table_view, CSectionConfig& config) const;

    void CreateSection(SectionContext& sc, CString& name);
    void SwitchSection(Section section, const QUuid& last_tab) const;
    void UpdateLastTab() const;

    void EditNameFIPT();
    void EditNameO();

    void SetTreeView(QTreeView* tree_view, CSectionInfo& info) const;

    void TreeDelegateF(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const;
    void TreeDelegateT(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const;
    void TreeDelegateI(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const;
    void TreeDelegateP(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const;
    void TreeDelegateO(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const;

    void TreeConnectF(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const;
    void TreeConnectI(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const;
    void TreeConnectT(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const;
    void TreeConnectP(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const;
    void TreeConnectO(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const;

    void InsertNodeFunction(const QModelIndex& parent, const QUuid& parent_id, int row);
    void InsertNodeFIPT(Node* node, const QModelIndex& parent, const QUuid& parent_id, int row); // Finance Inventory Partner Task
    void InsertNodeO(Node* base_node, const QModelIndex& parent, int row); // Purchase Sales

    void RemoveNode();
    void BranchRemove(TreeModel* tree_model, const QModelIndex& index, const QUuid& node_id);

    void UpdatePartnerReference(const QSet<QUuid>& partner_nodes, bool branch) const;

    void LoadAndInstallTranslator(CString& language);
    void ResizeColumn(QHeaderView* header, int stretch_column) const;

    void ReadLocalConfig();
    void ReadSectionConfig(SectionConfig& config, CString& section_name);

    void UpdateAppConfig(CAppConfig& app);
    void UpdateSectionConfig(CSectionConfig& section);
    void UpdateSharedConfig(CSharedConfig& shared);

    void UpdateAccountInfo(const QString& user, const QString& database, const QString& expire_date);
    void ClearAccountInfo();

    void SetAction(bool enable) const;

    void IniSectionGroup();
    void IniMarkGroup();

    void FocusTableWidget(const QUuid& node_id) const;

    void RegisterRptWgt(const QUuid& report_id, QWidget* widget);
    void WriteConfig();

    SectionContext* GetSectionContex(Section section);
    void InitSystemTray();

    void SetRemoveShortcut();

    inline bool IsTreeWidget(const QWidget* widget) { return widget && widget->inherits(kTreeWidget); }
    inline bool IsTableWidgetFIPT(const QWidget* widget) { return widget && widget->inherits(kTableWidgetFIPT); }
    inline bool IsTableWidgetO(const QWidget* widget) { return widget && widget->inherits(kTableWidgetO); }

private:
    Ui::MainWindow* ui {};

    Section start_ {};

    QSystemTrayIcon* tray_icon_ {};
    QMenu* tray_menu_ {};

    QSet<QUuid> node_pending_removal_ {};

    QPointer<SettlementWidget> settlement_widget_ {};

    QTranslator qt_translator_ {};
    QTranslator ytx_translator_ {};

    AppConfig app_config_ {};

    QSharedPointer<QSettings> app_settings_ {};
    QSharedPointer<QSettings> section_settings_ {};
    QNetworkAccessManager* network_manager_ {};

    QButtonGroup* section_group_ {};
    QActionGroup* mark_group_ {};

    SectionContext* sc_ {};

    SectionContext sc_f_ {};
    SectionContext sc_i_ {};
    SectionContext sc_t_ {};
    SectionContext sc_p_ {};
    SectionContext sc_sale_ {};
    SectionContext sc_purchase_ {};
};
#endif // MAINWINDOW_H
