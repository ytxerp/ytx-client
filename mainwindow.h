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
#include "table/model/leafmodel.h"
#include "table/model/leafmodelo.h"
#include "table/widget/leafwidgetfipt.h"
#include "table/widget/leafwidgeto.h"
#include "tree/model/treemodel.h"
#include "tree/model/treemodelo.h"

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

public slots:
    void on_actionInsertNode_triggered();
    void on_actionAppendEntry_triggered();
    void on_actionRemove_triggered();

private slots:
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
    void on_actionRegister_triggered();
    void on_actionReconnect_triggered();

    void on_actionCheckforUpdates_triggered();
    void on_actionExportExcel_triggered();

    void on_actionStatement_triggered();
    void on_actionSettlement_triggered();

    void on_tabWidget_currentChanged(int index);
    void on_tabWidget_tabBarDoubleClicked(int index);
    void on_tabWidget_tabCloseRequested(int index);

    bool RInitializeContext(const QString& expire_date);
    void RNodeLocation(const QUuid& node_id);
    void REntryLocation(const QUuid& entry_id, const QUuid& lhs_node_id, const QUuid& rhs_node_id);

    void RUpdateConfig(const LocalConfig& local, const GlobalConfig& global, const SectionConfig& section);
    void RSyncParty(const QUuid& node_id, int column, const QVariant& value);
    void RUpdateName(const QUuid& node_id, const QString& name, bool branch);
    void RUpdateState();
    void RConnectResult(bool result);

    void RFreeWidget(const QUuid& node_id);
    void RLeafExternalReference(const QUuid& node_id, int unit);

    void RTreeViewCustomContextMenuRequested(const QPoint& pos);
    void RTreeViewDoubleClicked(const QModelIndex& index);

    void RSectionGroup(int id);
    void REntryRefDoubleClicked(const QModelIndex& index);

    void RStatementPrimary(const QUuid& party_id, int unit, const QDateTime& start, const QDateTime& end);
    void RStatementSecondary(const QUuid& party_id, int unit, const QDateTime& start, const QDateTime& end);

    void REnableAction(bool finished);

    void RRemoveLeafNode(const QJsonObject& obj);
    // void RRemoveSupportNode(const QJsonObject& obj);
    void RGlobalConfig(const QJsonArray& arr);
    void RDocumentDir(const QString& section, const QString& document_dir);
    void RDefaultUnit(const QString& section, int unit);
    void RUpdateDefaultUnitFailed(const QString& section);

private:
    void SetTabWidget();
    void ClearMainwindow();

    void SetUniqueConnection() const;
    void SetAction() const;

    void InitContextFinance();
    void InitContextInventory();
    void InitContextPartner();
    void InitContextTask();
    void InitContextSale();
    void InitContextPurchase();

    void CreateLeafWidget(const QUuid& node_id);

    void CreateLeafFIST(TreeModel* tree_model, EntryHub* dbhub, LeafWgtHash& entry_wgt_hash, CSectionInfo& info, CSectionConfig& config, const QUuid& node_id);
    void CreateLeafO(TreeModel* tree_model, LeafWgtHash& entry_wgt_hash, CSectionInfo& info, CSectionConfig& config, const QUuid& node_id);

    void TableDelegateF(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const;
    void TableDelegateI(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const;
    void TableDelegateT(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const;
    void TableDelegateS(QTableView* table_view, CSectionConfig& config) const;
    void TableDelegateO(QTableView* table_view, CSectionConfig& config) const;

    void SetTableView(QTableView* table_view, int stretch_column, int lhs_node_column) const;
    void TableConnectF(QTableView* table_view, LeafModel* table_model, TreeModel* tree_model) const;
    void TableConnectI(QTableView* table_view, LeafModel* table_model, TreeModel* tree_model) const;
    void TableConnectT(QTableView* table_view, LeafModel* table_model, TreeModel* tree_model) const;
    void TableConnectS(QTableView* table_view, LeafModel* table_model) const;
    void TableConnectO(QTableView* table_view, LeafModelO* table_model, TreeModelO* tree_model, LeafWidgetO* widget) const;

    void CreateLeafExternalReference(TreeModel* tree_model, CSectionInfo& info, const QUuid& node_id, int unit);
    void DelegateLeafExternalReference(QTableView* table_view, CSectionConfig& config) const;

    void SetStatementView(QTableView* table_view, int stretch_column) const;
    void DelegateStatement(QTableView* table_view, CSectionConfig& config) const;
    void DelegateSettlement(QTableView* table_view, CSectionConfig& config) const;
    void DelegateSettlementPrimary(QTableView* table_view, CSectionConfig& config) const;

    void DelegateStatementPrimary(QTableView* table_view, CSectionConfig& config) const;
    void DelegateStatementSecondary(QTableView* table_view, CSectionConfig& config) const;

    void CreateSection(SectionContext& sc, CString& name);
    void SwitchSection(Section section, const QUuid& last_tab) const;
    void UpdateLastTab() const;

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
    void TreeConnectO(QTreeView* tree_view, TreeModel* tree_model) const;

    void InsertNodeFunction(const QModelIndex& parent, const QUuid& parent_id, int row);
    void InsertNodeFIPT(Node* node, const QModelIndex& parent, const QUuid& parent_id, int row); // Finance Inventory Partner Task
    void InsertNodeO(Node* node, const QModelIndex& parent, int row); // Purchase Sales

    void RemoveNode();
    void RemoveBranchNode(TreeModel* tree_model, const QModelIndex& index, const QUuid& node_id);

    void UpdatePartnerReference(const QSet<QUuid>& partner_nodes, bool branch) const;

    void LoadAndInstallTranslator(CString& language);
    void ResizeColumn(QHeaderView* header, int stretch_column) const;

    void ReadLocalConfig();
    void ReadSectionConfig(SectionConfig& config, CString& section_name);

    void UpdateLocalConfig(CLocalConfig& local);
    void UpdateSectionConfig(CSectionConfig& section);
    void UpdateGlobalConfig(CGlobalConfig& global);

    void UpdateAccountInfo(const QString& user, const QString& database, const QString& expire_date);
    void ClearAccountInfo();

    void EnableAction(bool enable) const;

    void IniSectionGroup();
    void LeafExternalReferenceI(const QUuid& node_id, int unit);
    void LeafExternalReferenceS(const QUuid& node_id, int unit);
    void OrderNodeLocation(Section section, const QUuid& node_id);

    void SwitchToLeaf(const QUuid& node_id, const QUuid& entry_id = {}) const;

    void RegisterRptWgt(const QUuid& report_id, ReportWidget* widget);
    void WriteConfig();

    SectionContext* GetSectionContex(const QString& section);
    void InitSystemTray();

    void SetAppFontByDpi();

    inline bool IsTreeWidget(const QWidget* widget) { return widget && widget->inherits(kTreeWidget); }
    inline bool IsLeafWidgetFIPT(const QWidget* widget) { return widget && widget->inherits(kLeafWidgetFIPT); }
    inline bool IsLeafWidgetO(const QWidget* widget) { return widget && widget->inherits(kLeafWidgetO); }

private:
    Ui::MainWindow* ui {};

    Section start_ {};

    QSystemTrayIcon* tray_icon_ {};
    QMenu* tray_menu_ {};

    QPointer<SettlementWidget> settlement_widget_ {};
    QMap<QString, QString> print_template_ {};

    QTranslator qt_translator_ {};
    QTranslator ytx_translator_ {};

    LocalConfig local_config_ {};

    QSharedPointer<QSettings> local_settings_ {};
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
#endif // MAINWINDOW_H
