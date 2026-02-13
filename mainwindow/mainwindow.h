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
#include <QLabel>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTableView>
#include <QTranslator>

#include "billing/settlement/settlement.h"
#include "billing/settlement/treewidgetsettlement.h"
#include "component/config.h"
#include "component/info.h"
#include "component/sectioncontex.h"
#include "entryhub/entryhub.h"
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

    void on_actionCheckUpdates_triggered();
    void on_actionExportExcel_triggered();

    void on_actionStatement_triggered();
    void on_actionSettlement_triggered();
    void on_actionEntryJournal_triggered();

    void on_tabWidget_currentChanged(int);
    void on_tabWidget_tabBarDoubleClicked(int index);
    void on_tabWidget_tabCloseRequested(int index);

    void RNodeLocation(Section section, const QUuid& node_id);
    void REntryLocation(const QUuid& entry_id, const QUuid& lhs_node_id, const QUuid& rhs_node_id);
    void RNodeSelected(Section section, const QUuid& node_id);

    void RUpdateConfig(const AppConfig& app, const SharedConfig& shared, const SectionConfig& section);
    void RSyncPartner(const QUuid& node_id, const QUuid& value);
    void RUpdatePartner(const QUuid& widget_id, const QUuid& partner_id);
    void RUpdateName(const QUuid& node_id, const QString& name, bool branch);
    void RActionEntry(EntryAction action);

    void RConnectionRefused();
    void RConnectionSucceeded();
    void RRemoteHostClosed();

    void RLoginSucceeded(const QString& expire_date);
    void RLoginFailed();
    void RTreeSyncFinished();

    void RFreeWidget(Section section, const QUuid& node_id);
    void RFlushCaches();

    void RSaleReference(Section section, const QUuid& widget_id, const QJsonArray& array);
    void RSaleReferencePrimary(const QUuid& node_id, int unit);
    void RSaleReferenceSecondary(const QModelIndex& index);

    void RStatement(Section section, const QUuid& widget_id, const QJsonArray& array);
    void RStatementNodeAcked(Section section, const QUuid& widget_id, const QJsonArray& array);
    void RStatementEntryAcked(Section section, const QUuid& widget_id, const QJsonArray& array, const QJsonObject& total);

    void RTreeViewCustomContextMenuRequested(const QPoint& pos);
    void RInsertNodeTag(const Tag* tag, TreeModel* model, const Node* node);
    void RRemoveNodeTag(const Tag* tag, TreeModel* model, const Node* node);

    void RTreeViewDoubleClicked(const QModelIndex& index);
    void RSettlementTableViewDoubleClicked(const QModelIndex& index);

    void RSectionGroup(int id);

    void RStatementNode(const QUuid& partner_id, const QDateTime& start, const QDateTime& end, int unit);
    void RStatementEntry(const QUuid& partner_id, const QDateTime& start, const QDateTime& end, int unit);

    void RSettlement(Section section, const QUuid& widget_id, const QJsonArray& array);
    void RSettlementItemAcked(Section section, const QUuid& widget_id, const QJsonArray& array);
    void RSettlementInserted(const QJsonObject& obj);
    void RSettlementRecalled(const QJsonObject& obj);
    void RSettlementUpdated(const QJsonObject& obj);

    void ROrderReleased(Section section, const QUuid& node_id, int version);
    void ROrderRecalled(Section section, const QUuid& node_id, int version);
    void ROrderSaved(Section section, const QUuid& node_id, int version);
    void RInvalidOperation();

    void RLeafDeleteDenied(const QJsonObject& obj);

    void RApplyTag(const QJsonObject& obj);
    void RInsertTag(const QJsonObject& obj, bool is_same_session);
    void RUpdateTag(const QJsonObject& obj);
    void RDeleteTag(const QJsonObject& obj);
    inline void RInsertingTag(Tag* tag) { inserting_tag_.insert(tag->id, tag); }

    void RTableViewCustomContextMenuRequested(const QPoint& pos);
    void RInsertEntryTag(const Tag* tag, TableModel* model, const Entry* entry);
    void RRemoveEntryTag(const Tag* tag, TableModel* model, const Entry* entry);

    void RSharedConfig(const QJsonArray& arr);
    void RDocumentDir(Section section, const QString& document_dir);
    void RDefaultUnit(Section section, int unit);
    void RUpdateDefaultUnitFailed(const QString& section);
    void RSelectLeafEntry(const QUuid& node_id, const QUuid& entry_id);

private:
    void SetTabWidget();
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

    void ShowLeafWidget(const QUuid& node_id, const QUuid& entry_id = {});

    void CreateLeafFIPT(SectionContext* sc, const QUuid& node_id);
    void CreateLeafO(SectionContext* sc, const QUuid& node_id);
    void SettlementItemTab(const QUuid& parent_widget_id, const Settlement& settlement, SyncState sync_state);

    void TableDelegateF(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const;
    void TableDelegateI(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const;
    void TableDelegateT(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const;
    void TableDelegateP(QTableView* table_view, CSectionConfig& config) const;
    void TableDelegateO(QTableView* table_view, CSectionConfig& config) const;

    void SetTableView(QTableView* view, Section section, int stretch_column, int lhs_node_column) const;

    void TableConnectF(TableModel* table_model) const;
    void TableConnectI(TableModel* table_model) const;
    void TableConnectT(TableModel* table_model) const;
    void TableConnectP(TableModel* table_model) const;
    void TableConnectO(TableModelO* table_model_o, TableWidgetO* widget) const;

    void CreateSaleReference(const QUuid& node_id, int unit);
    void DelegateSaleReference(QTableView* table_view, CSectionConfig& config) const;
    void SetTableViewSaleReference(QTableView* view) const;

    void SetStatementView(QTableView* view, int stretch_column) const;
    void DelegateStatement(QTableView* table_view, CSectionConfig& config) const;

    void SetSettlementView(QTableView* view, int stretch_column) const;
    void DelegateSettlement(QTableView* table_view, CSectionConfig& config) const;
    void SetSettlementItemView(QTableView* view, int stretch_column) const;
    void DelegateSettlementNode(QTableView* table_view, CSectionConfig& config) const;

    void SetTagView(QTableView* view) const;
    void DelegateTagView(QTableView* table_view) const;

    void DelegateStatementNode(QTableView* table_view, CSectionConfig& config) const;
    void DelegateStatementEntry(QTableView* table_view, CSectionConfig& config) const;

    void CreateSection(SectionContext& sc, CString& name);
    void SwitchSection(Section section, const QUuid& last_tab) const;
    void SaveLastTab() const;

    void EditNameFIPT();
    void EditNameO();

    void SetTreeView(QTreeView* view, CSectionInfo& info) const;
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
    void TreeConnectO(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const;

    void InsertNodeFunction(const QModelIndex& parent_index);
    void InsertNodeFIPT(const QModelIndex& parent_index); // Finance Inventory Partner Task
    void InsertNodeO(const QModelIndex& parent_index); // Purchase Sales

    void DeleteNode();
    void DeleteEntry(TableWidget* widget);
    void DeleteBranch(TreeModel* tree_model, const QModelIndex& index, const QUuid& node_id);
    void DeleteSettlement(TreeWidgetSettlement* widget);

    void UpdatePartnerReference(const QSet<QUuid>& partner_nodes, bool branch) const;

    void LoadAndInstallTranslator(CString& language);
    void ResizeColumn(QHeaderView* header, int stretch_column) const;

    void ReadLocalConfig();
    void ReadSectionConfig(SectionConfig& config, CString& section_name);

    void UpdateAppConfig(CAppConfig& app);
    void UpdateSectionConfig(CSectionConfig& section) const;
    void UpdateSharedConfig(CSharedConfig& shared);

    void UpdateAccountInfo(const QString& user, const QString& database, const QString& expire_date);

    void SetAction(bool enable) const;

    void IniSectionGroup();
    void IniMarkGroup();

    void FocusTabWidget(const QUuid& node_id) const;

    void RegisterWidget(QWidget* widget, const QUuid& widget_id, WidgetRole role);
    void WriteConfig();

    SectionContext* GetSectionContex(Section section);
    void InitSystemTray();
    void InitStatusLabel();

    void FlushCaches(SectionContext& sc);

    QSet<QString> ChildrenName(const Node* node) const;
    QSet<QUuid> LeafChildrenId(const Node* node) const;

    QIcon GetTagIcon(SectionContext* sc, const Tag* tag, bool checked);
    QPixmap GetTagPixmap(SectionContext* sc, const Tag* tag);

    inline void InvalidateTagIconCache(SectionContext* sc, const QUuid& tag_id)
    {
        sc->tag_icon.remove(tag_id);
        sc->tag_icon_checked.remove(tag_id);
        sc->tag_pixmap.remove(tag_id);
    }
    void UpdateTagIcon(SectionContext* sc, const Tag* tag);

    inline bool IsTreeWidget(const QWidget* widget) { return widget && widget->inherits(kTreeWidget); }
    inline bool IsTableWidgetFIPT(const QWidget* widget) { return widget && widget->inherits(kTableWidgetFIPT); }
    inline bool IsTableWidgetO(const QWidget* widget) { return widget && widget->inherits(kTableWidgetO); }
    inline bool IsTreeWidgetSettlement(const QWidget* widget) { return widget && widget->inherits(kTreeWidgetSettlement); }

private:
    Ui::MainWindow* ui {};

    Section start_ {};
    QLabel* connection_label_ {};
    QLabel* login_label_ {};

    QSystemTrayIcon* tray_icon_ {};
    QMenu* tray_menu_ {};

    QSet<QUuid> deleting_node_ {};
    QHash<QUuid, Tag*> inserting_tag_ {};

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
