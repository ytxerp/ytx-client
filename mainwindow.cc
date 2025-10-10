#include "mainwindow.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QHeaderView>
#include <QMessageBox>
#include <QNetworkReply>
#include <QQueue>
#include <QScrollBar>
#include <QUrl>
#include <QtConcurrent>

#include "component/arg/insertnodeargfipt.h"
#include "component/constant.h"
#include "component/enumclass.h"
#include "component/signalblocker.h"
#include "component/stringinitializer.h"
#include "delegate/boolstring.h"
#include "delegate/document.h"
#include "delegate/double.h"
#include "delegate/filterunit.h"
#include "delegate/int.h"
#include "delegate/line.h"
#include "delegate/plaintext.h"
#include "delegate/readonly/doublespinnonezeror.h"
#include "delegate/readonly/doublespinr.h"
#include "delegate/readonly/doublespinunitr.h"
#include "delegate/readonly/intstringr.h"
#include "delegate/readonly/issuedtimer.h"
#include "delegate/readonly/nodenamer.h"
#include "delegate/readonly/nodepathr.h"
#include "delegate/readonly/sectionr.h"
#include "delegate/status.h"
#include "delegate/table/tablecombofilter.h"
#include "delegate/table/tableissuedtime.h"
#include "delegate/tree/color.h"
#include "delegate/tree/finance/financeforeignr.h"
#include "delegate/tree/order/ordernamer.h"
#include "delegate/tree/order/orderrule.h"
#include "delegate/tree/order/orderunit.h"
#include "delegate/tree/treeissuedtime.h"
#include "dialog/about.h"
#include "dialog/editnodename.h"
#include "dialog/insertnode/insertnodebranch.h"
#include "dialog/insertnode/insertnodefinance.h"
#include "dialog/insertnode/insertnodei.h"
#include "dialog/insertnode/insertnodep.h"
#include "dialog/insertnode/insertnodetask.h"
#include "dialog/login.h"
#include "dialog/preferences.h"
#include "dialog/registerdialog.h"
#include "dialog/removenode/leafremovedialog.h"
#include "document.h"
#include "entryhub/entryhubf.h"
#include "entryhub/entryhubi.h"
#include "entryhub/entryhubo.h"
#include "entryhub/entryhubp.h"
#include "entryhub/entryhubt.h"
#include "global/leafsstation.h"
#include "global/logininfo.h"
#include "global/nodepool.h"
#include "report/model/entryrefmodel.h"
#include "report/model/settlementmodel.h"
#include "report/model/statementmodel.h"
#include "report/model/statementprimarymodel.h"
#include "report/model/statementsecondarymodel.h"
#include "report/widget/refwidget.h"
#include "report/widget/statementwidget.h"
#include "search/dialog/searchdialog.h"
#include "search/dialog/searchdialogf.h"
#include "search/dialog/searchdialogi.h"
#include "search/dialog/searchdialogo.h"
#include "search/dialog/searchdialogp.h"
#include "search/dialog/searchdialogt.h"
#include "search/entry/searchentrymodelf.h"
#include "search/entry/searchentrymodeli.h"
#include "search/entry/searchentrymodelo.h"
#include "search/entry/searchentrymodelp.h"
#include "search/entry/searchentrymodelt.h"
#include "search/node/searchnodemodelf.h"
#include "search/node/searchnodemodeli.h"
#include "search/node/searchnodemodelo.h"
#include "search/node/searchnodemodelp.h"
#include "search/node/searchnodemodelt.h"
#include "table/model/leafmodelf.h"
#include "table/model/leafmodeli.h"
#include "table/model/leafmodelp.h"
#include "table/model/leafmodelt.h"
#include "tree/model/treemodelf.h"
#include "tree/model/treemodeli.h"
#include "tree/model/treemodelp.h"
#include "tree/model/treemodelt.h"
#include "tree/widget/treewidgetf.h"
#include "tree/widget/treewidgeti.h"
#include "tree/widget/treewidgetp.h"
#include "tree/widget/treewidgetto.h"
#include "ui_mainwindow.h"
#include "utils/mainwindowutils.h"
#include "utils/widgetutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , network_manager_(new QNetworkAccessManager(this))
{
    ReadLocalConfig();

    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniSectionGroup();
    IniMarkGroup();
    InitSystemTray();

    SetTabWidget();
    SetIcon();
    SetUniqueConnection();
    SetRemoveShortcut();
    SetAction(false);

    StringInitializer::SetHeader(sc_f_.info, sc_i_.info, sc_t_.info, sc_p_.info, sc_sale_.info, sc_purchase_.info);

    WidgetUtils::ReadConfig(ui->splitter, &QSplitter::restoreState, app_settings_, kSplitter, kState);
    WidgetUtils::ReadConfig(this, &QMainWindow::restoreState, app_settings_, kMainwindow, kState, 0);
    WidgetUtils::ReadConfig(this, &QMainWindow::restoreGeometry, app_settings_, kMainwindow, kGeometry);
}

void MainWindow::SetRemoveShortcut()
{
#ifdef Q_OS_WIN
    ui->actionRemove->setShortcut(QKeySequence::Delete);
#elif defined(Q_OS_MACOS)
    ui->actionRemove->setShortcut(Qt::Key_Backspace);
#else
    ui->actionRemove->setShortcut(QKeySequence::Delete);
#endif
}

MainWindow::~MainWindow()
{
    WriteConfig();
    delete ui;
}

bool MainWindow::RInitializeContext(const QString& expire_date)
{
    LoginInfo& login_info { LoginInfo::Instance() };
    UpdateAccountInfo(login_info.Email(), login_info.Workspace(), expire_date);

    if (!section_settings_) {
        const QString section_file_path { QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator()
            + MainWindowUtils::SectionFile(login_info.Email(), login_info.Workspace()) + kDotSuffixINI };

        section_settings_ = QSharedPointer<QSettings>::create(section_file_path, QSettings::IniFormat);
    }

    InitContextFinance();
    InitContextTask();
    InitContextInventory();
    InitContextPartner();
    InitContextSale();
    InitContextPurchase();

    CreateSection(sc_f_, tr("Finance"));
    CreateSection(sc_p_, tr("Partner"));
    CreateSection(sc_i_, tr("Inventory"));
    CreateSection(sc_t_, tr("Task"));
    CreateSection(sc_sale_, tr("Sale"));
    CreateSection(sc_purchase_, tr("Purchase"));

    RSectionGroup(static_cast<int>(start_));

    SetAction(true);
    on_tabWidget_currentChanged(0);

    QTimer::singleShot(0, this, [this]() { MainWindowUtils::ReadPrintTmplate(print_template_); });
    return true;
}

void MainWindow::RTreeViewDoubleClicked(const QModelIndex& index)
{
    if (index.column() != 0)
        return;

    const int kind_column { NodeUtils::KindColumn(start_) };
    const NodeKind kind { index.siblingAtColumn(kind_column).data().toInt() };
    if (kind == NodeKind::kBranch)
        return;

    const int unit_column { NodeUtils::UnitColumn(start_) };
    const int unit { index.siblingAtColumn(unit_column).data().toInt() };
    if (start_ == Section::kInventory && unit == std::to_underlying(UnitI::kExternal))
        return;

    const auto node_id { index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() };
    assert(!node_id.isNull());

    ShowLeafWidget(node_id);
}

// ShowLeafWidget - Show leaf widget (create and scroll as needed)
// -------------------------------
// Triggers:
//  • Node double-click: entry_id empty → no scroll
//  • Entry jump: entry_id valid → scroll to entry
//
// Flow:
//  1. If leaf exists → switch and scroll (if applicable)
//  2. If not exists → send request → create leaf → switch
//
// Note: Scrolling only occurs when entry_id is non-empty
void MainWindow::ShowLeafWidget(const QUuid& node_id, const QUuid& entry_id)
{
    auto& leaf_wgt_hash { sc_->leaf_wgt_hash };

    if (leaf_wgt_hash.contains(node_id)) {
        ActivateLeafTab(node_id);
        RScrollToEntry(node_id, entry_id);
        return;
    }

    const auto message { JsonGen::LeafAcked(sc_->info.section, node_id, entry_id) };
    WebSocket::Instance()->SendMessage(kLeafAcked, message);

    if (start_ == Section::kSale || start_ == Section::kPurchase) {
        CreateLeafO(sc_, node_id);
    } else {
        CreateLeafFIPT(sc_, node_id);
    }

    ActivateLeafTab(node_id);
}

void MainWindow::RSectionGroup(int id)
{
    const Section kSection { id };
    start_ = kSection;

    if (!section_settings_)
        return;

    MainWindowUtils::SwitchDialog(sc_, false);
    UpdateLastTab();

    switch (kSection) {
    case Section::kFinance:
        sc_ = &sc_f_;
        break;
    case Section::kInventory:
        sc_ = &sc_i_;
        break;
    case Section::kTask:
        sc_ = &sc_t_;
        break;
    case Section::kPartner:
        sc_ = &sc_p_;
        break;
    case Section::kSale:
        sc_ = &sc_sale_;
        break;
    case Section::kPurchase:
        sc_ = &sc_purchase_;
        break;
    default:
        break;
    }

    SwitchSection(start_, sc_->info.last_tab_id);
}

void MainWindow::REntryRefDoubleClicked(const QModelIndex& index)
{
    const auto node_id { index.siblingAtColumn(std::to_underlying(EntryRefEnum::kOrderId)).data().toUuid() };
    const int kColumn { std::to_underlying(EntryRefEnum::kInitial) };

    assert(!node_id.isNull());

    if (index.column() != kColumn)
        return;

    const Section section { index.siblingAtColumn(std::to_underlying(EntryRefEnum::kSection)).data().toInt() };
    OrderNodeLocation(section, node_id);
}

void MainWindow::RStatementPrimary(const QUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end)
{
    auto* model { new StatementPrimaryModel(sc_->entry_hub, sc_->info, partner_id, this) };
    auto* widget { new StatementWidget(model, unit, false, start, end, this) };

    const QString name { tr("StatementPrimary-") + sc_p_.tree_model->Name(partner_id) };
    const int tab_index { ui->tabWidget->addTab(widget, name) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    const QUuid report_id { QUuid::createUuidV7() };
    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, report_id }));
    tab_bar->setTabToolTip(tab_index, name);

    auto* view { widget->View() };
    SetStatementView(view, std::to_underlying(StatementPrimaryEnum::kDescription));
    DelegateStatementPrimary(view, sc_->section_config);

    connect(widget, &StatementWidget::SResetModel, model, &StatementPrimaryModel::RResetModel);

    RegisterRptWgt(report_id, widget);
}

void MainWindow::RStatementSecondary(const QUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end)
{
    auto tree_model_p { sc_p_.tree_model };

    auto* model { new StatementSecondaryModel(
        sc_->entry_hub, sc_->info, partner_id, sc_i_.tree_model->LeafPath(), tree_model_p, app_config_.company_name, this) };
    auto* widget { new StatementWidget(model, unit, true, start, end, this) };

    const QString name { tr("StatementSecondary-") + tree_model_p->Name(partner_id) };
    const int tab_index { ui->tabWidget->addTab(widget, name) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    const QUuid report_id { QUuid::createUuidV7() };
    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, report_id }));
    tab_bar->setTabToolTip(tab_index, name);

    auto* view { widget->View() };
    SetStatementView(view, std::to_underlying(StatementSecondaryEnum::kDescription));
    DelegateStatementSecondary(view, sc_->section_config);

    connect(widget, &StatementWidget::SResetModel, model, &StatementSecondaryModel::RResetModel);
    connect(widget, &StatementWidget::SExport, model, &StatementSecondaryModel::RExport);

    RegisterRptWgt(report_id, widget);
}

void MainWindow::REnableAction(bool finished)
{
    ui->actionAppendEntry->setEnabled(!finished);
    ui->actionRemove->setEnabled(!finished);
}

// ActivateLeafTab - Activate and focus the leaf widget tab
// -------------------------------
// Used for:
//  • Node double-click → navigate to node's tab
//  • Entry jump → switch between different node tabs
//
// Note: Clears view selection after switching
void MainWindow::ActivateLeafTab(const QUuid& node_id) const
{
    auto widget { sc_->leaf_wgt_hash.value(node_id, nullptr) };
    assert(widget);

    ui->tabWidget->setCurrentWidget(widget);
    widget->activateWindow();

    widget->View()->setCurrentIndex(QModelIndex());
}

void MainWindow::CreateLeafFIPT(SectionContext* sc, CUuid& node_id)
{
    auto tree_model { sc->tree_model };
    auto& entry_hub = sc->entry_hub;
    const auto& info = sc->info;
    const auto& section_config = sc->section_config;

    assert(tree_model);
    assert(tree_model->Contains(node_id));

    CString name { tree_model->Name(node_id) };
    const Section section { info.section };
    const bool rule { tree_model->Rule(node_id) };

    LeafModel* leaf_model {};
    LeafModelArg arg { entry_hub, info, node_id, rule };

    switch (section) {
    case Section::kFinance:
        leaf_model = new LeafModelF(arg, this);
        break;
    case Section::kInventory:
        leaf_model = new LeafModelI(arg, this);
        break;
    case Section::kTask: {
        const int status { tree_model->Status(node_id) };
        leaf_model = new LeafModelT(arg, status, this);
    } break;
    case Section::kPartner:
        leaf_model = new LeafModelP(arg, this);
        break;
    default:
        break;
    }

    LeafWidgetFIPT* widget { new LeafWidgetFIPT(leaf_model, this) };

    const int tab_index { ui->tabWidget->addTab(widget, name) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { section, node_id }));
    tab_bar->setTabToolTip(tab_index, tree_model->Path(node_id));

    auto* view { widget->View() };

    SetTableView(view, std::to_underlying(EntryEnum::kDescription), std::to_underlying(EntryEnum::kLhsNode));

    switch (section) {
    case Section::kFinance:
        TableDelegateF(view, tree_model, section_config, node_id);
        TableConnectF(view, leaf_model, tree_model);
        break;
    case Section::kInventory:
        TableDelegateI(view, tree_model, section_config, node_id);
        TableConnectI(view, leaf_model, tree_model);
        break;
    case Section::kTask:
        TableDelegateT(view, tree_model, section_config, node_id);
        TableConnectT(view, leaf_model, tree_model);
        break;
    case Section::kPartner:
        TableDelegateS(view, section_config);
        TableConnectS(view, leaf_model);
        break;
    default:
        break;
    }

    sc->leaf_wgt_hash.insert(node_id, widget);
    LeafSStation::Instance()->RegisterModel(node_id, leaf_model);
}

void MainWindow::InsertNodeO(Node* node, const QModelIndex& parent, int row)
{
    auto* tree_model_order { static_cast<TreeModelO*>(sc_->tree_model.data()) };
    const QUuid node_id { node->id };

    LeafModelArg model_arg { sc_->entry_hub, sc_->info, node_id, true };
    auto* leaf_model_order { new LeafModelO(model_arg, node, sc_i_.tree_model, sc_p_.entry_hub, this) };

    auto print_manager { QSharedPointer<PrintManager>::create(app_config_, sc_i_.tree_model, sc_p_.tree_model) };

    auto widget_arg { InsertNodeArgO { node, sc_->entry_hub, leaf_model_order, sc_p_.tree_model, sc_->section_config, start_ } };
    auto* widget { new LeafWidgetO(widget_arg, true, print_template_, print_manager, this) };
    auto* view { widget->View() };

    TableConnectO(view, leaf_model_order, tree_model_order, widget);

    connect(widget, &LeafWidgetO::SSaveOrder, this, [=, this]() {
        if (tree_model_order->InsertNode(row, parent, node)) {
            auto index { tree_model_order->index(row, 0, parent) };
            sc_->tree_view->setCurrentIndex(index);
        }
    });

    connect(widget, &LeafWidgetO::SSaveOrder, leaf_model_order, &LeafModelO::RSaveOrder);

    SetTableView(view, std::to_underlying(EntryEnumO::kDescription), std::to_underlying(EntryEnumO::kLhsNode));
    TableDelegateO(view, sc_->section_config);

    const int tab_index { ui->tabWidget->addTab(widget, QString()) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, node_id }));

    sc_->leaf_wgt_hash.insert(node_id, widget);
    ActivateLeafTab(node_id);
}

void MainWindow::CreateLeafO(SectionContext* sc, const QUuid& node_id)
{
    // Extract frequently used shortcuts
    const auto section { sc->info.section };
    auto& entry_hub = sc->entry_hub;
    auto& section_config = sc->section_config;
    auto* tab_widget = ui->tabWidget;
    auto* tab_bar = tab_widget->tabBar();

    // Validate tree model and node
    auto* tree_model_o = static_cast<TreeModelO*>(sc->tree_model.data());
    if (!tree_model_o)
        return;

    auto* node = static_cast<NodeO*>(tree_model_o->GetNode(node_id));
    if (!node)
        return;

    const auto partner_id { node->partner };
    assert(!partner_id.isNull());

    // Prepare dependencies
    auto tree_model_p { sc_p_.tree_model };
    auto tree_model_i { sc_i_.tree_model };

    // Create model and widget
    LeafModelArg model_arg { entry_hub, sc->info, node_id, node->direction_rule };
    auto* model = new LeafModelO(model_arg, node, tree_model_i, sc_p_.entry_hub, this);

    auto print_manager { QSharedPointer<PrintManager>::create(app_config_, tree_model_i, tree_model_p) };
    InsertNodeArgO widget_arg { node, entry_hub, model, tree_model_p, section_config, section };
    auto* widget = new LeafWidgetO(widget_arg, false, print_template_, print_manager, this);

    // Setup tab
    const int tab_index { tab_widget->addTab(widget, tree_model_p->Name(partner_id)) };
    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { section, node_id }));
    tab_bar->setTabToolTip(tab_index, tree_model_p->Path(partner_id));

    // Configure view
    auto* view = widget->View();
    SetTableView(view, std::to_underlying(EntryEnumO::kDescription), std::to_underlying(EntryEnumO::kLhsNode));
    TableConnectO(view, model, tree_model_o, widget);
    TableDelegateO(view, section_config);

    sc->leaf_wgt_hash.insert(node_id, widget);
}

void MainWindow::TableConnectF(QTableView* table_view, LeafModel* table_model, TreeModel* tree_model) const
{
    connect(table_model, &LeafModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);

    connect(table_model, &LeafModel::SSyncDelta, tree_model, &TreeModel::RSyncDelta);

    connect(table_model, &LeafModel::SRemoveOneEntry, LeafSStation::Instance(), &LeafSStation::RRemoveOneEntry);
    connect(table_model, &LeafModel::SAppendOneEntry, LeafSStation::Instance(), &LeafSStation::RAppendOneEntry);
    connect(table_model, &LeafModel::SUpdateBalance, LeafSStation::Instance(), &LeafSStation::RUpdateBalance);
}

void MainWindow::TableConnectI(QTableView* table_view, LeafModel* table_model, TreeModel* tree_model) const
{
    connect(table_model, &LeafModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);

    connect(table_model, &LeafModel::SSyncDelta, tree_model, &TreeModel::RSyncDelta);

    connect(table_model, &LeafModel::SRemoveOneEntry, LeafSStation::Instance(), &LeafSStation::RRemoveOneEntry);
    connect(table_model, &LeafModel::SAppendOneEntry, LeafSStation::Instance(), &LeafSStation::RAppendOneEntry);
    connect(table_model, &LeafModel::SUpdateBalance, LeafSStation::Instance(), &LeafSStation::RUpdateBalance);
}

void MainWindow::TableConnectT(QTableView* table_view, LeafModel* table_model, TreeModel* tree_model) const
{
    connect(table_model, &LeafModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);

    connect(table_model, &LeafModel::SSyncDelta, tree_model, &TreeModel::RSyncDelta);

    connect(table_model, &LeafModel::SRemoveOneEntry, LeafSStation::Instance(), &LeafSStation::RRemoveOneEntry);
    connect(table_model, &LeafModel::SAppendOneEntry, LeafSStation::Instance(), &LeafSStation::RAppendOneEntry);
    connect(table_model, &LeafModel::SUpdateBalance, LeafSStation::Instance(), &LeafSStation::RUpdateBalance);
}

void MainWindow::TableConnectO(QTableView* table_view, LeafModelO* leaf_model_order, TreeModelO* tree_model, LeafWidgetO* widget) const
{
    connect(leaf_model_order, &LeafModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);

    connect(leaf_model_order, &LeafModel::SSyncDelta, widget, &LeafWidgetO::RSyncDelta);

    connect(widget, &LeafWidgetO::SSyncPartner, leaf_model_order, &LeafModelO::RSyncPartner);
    connect(widget, &LeafWidgetO::SSyncFinished, leaf_model_order, &LeafModelO::RSyncFinished);
    connect(widget, &LeafWidgetO::SSyncFinished, tree_model, &TreeModelO::RSyncFinished);

    connect(widget, &LeafWidgetO::SSyncPartner, this, &MainWindow::RSyncPartner);
    connect(widget, &LeafWidgetO::SEnableAction, this, &MainWindow::REnableAction);
}

void MainWindow::TableConnectS(QTableView* table_view, LeafModel* leaf_model) const
{
    connect(leaf_model, &LeafModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);
}

void MainWindow::TableDelegateF(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const
{
    auto* issued_time { new TableIssuedTime(config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kIssuedTime), issued_time);

    auto* lhs_rate { new Double(config.rate_decimal, 0, kMaxNumeric_16_8, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kLhsRate), lhs_rate);

    auto* line { new Line(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kCode), line);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kDescription), line);

    auto* document { new Document(sc_->shared_config.document_dir, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kDocument), document);

    auto* status { new Status(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kStatus), status);

    QSortFilterProxyModel* filter_model { tree_model->ExcludeOneModel(node_id) };

    auto* node { new TableComboFilter(tree_model, filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kRhsNode), node);

    auto* value { new Double(config.amount_decimal, kMinNumeric_12_4, kMaxNumeric_12_4, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kDebit), value);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kCredit), value);

    auto* initial { new DoubleSpinR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kBalance), initial);
}

void MainWindow::TableDelegateI(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const
{
    auto* issued_time { new TableIssuedTime(config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumI::kIssuedTime), issued_time);

    auto* unit_cost { new Double(config.rate_decimal, 0, kMaxNumeric_16_8, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumI::kUnitCost), unit_cost);

    auto* line { new Line(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumI::kCode), line);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumI::kDescription), line);

    auto* document { new Document(sc_->shared_config.document_dir, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumI::kDocument), document);

    auto* status { new Status(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumI::kStatus), status);

    QSortFilterProxyModel* filter_model { tree_model->ExcludeMultipleModel(node_id, std::to_underlying(UnitI::kExternal)) };

    auto* node { new TableComboFilter(tree_model, filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumI::kRhsNode), node);

    auto* value { new Double(config.amount_decimal, kMinNumeric_12_4, kMaxNumeric_12_4, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumI::kDebit), value);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumI::kCredit), value);

    auto* initial { new DoubleSpinR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumI::kBalance), initial);
}

void MainWindow::TableDelegateT(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const
{
    auto* issued_time { new TableIssuedTime(config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumT::kIssuedTime), issued_time);

    auto* unit_cost { new Double(config.rate_decimal, 0, kMaxNumeric_16_8, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumT::kUnitCost), unit_cost);

    auto* line { new Line(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumT::kCode), line);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumT::kDescription), line);

    auto* document { new Document(sc_->shared_config.document_dir, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumT::kDocument), document);

    auto* status { new Status(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumT::kStatus), status);

    QSortFilterProxyModel* filter_model { tree_model->ExcludeOneModel(node_id) };

    auto* node { new TableComboFilter(tree_model, filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumT::kRhsNode), node);

    auto* value { new Double(config.amount_decimal, kMinNumeric_12_4, kMaxNumeric_12_4, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumT::kDebit), value);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumT::kCredit), value);

    auto* initial { new DoubleSpinR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumT::kBalance), initial);
}

void MainWindow::TableDelegateS(QTableView* table_view, CSectionConfig& config) const
{
    auto* issued_time { new TableIssuedTime(config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kIssuedTime), issued_time);

    auto* unit_price { new Double(config.rate_decimal, 0, kMaxNumeric_12_4, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kUnitPrice), unit_price);

    auto* line { new Line(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kCode), line);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kDescription), line);

    auto tree_model_i { sc_i_.tree_model };

    auto* ext_filter_model { tree_model_i->IncludeUnitModel(std::to_underlying(UnitI::kExternal)) };
    auto* external_sku { new FilterUnit(tree_model_i, ext_filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kExternalSku), external_sku);

    auto* document { new Document(sc_->shared_config.document_dir, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kDocument), document);

    auto* status { new Status(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kStatus), status);

    auto* int_filter_model { tree_model_i->IncludeUnitModel(std::to_underlying(UnitI::kInternal)) };
    auto* internal_sku { new FilterUnit(tree_model_i, int_filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumP::kRhsNode), internal_sku);
}

void MainWindow::TableDelegateO(QTableView* table_view, CSectionConfig& config) const
{
    auto tree_model_i { sc_i_.tree_model };
    auto* int_filter_model { tree_model_i->IncludeUnitModel(std::to_underlying(UnitI::kInternal)) };

    auto* internal_sku { new FilterUnit(tree_model_i, int_filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kRhsNode), internal_sku);

    auto* ext_filter_model { tree_model_i->IncludeUnitModel(std::to_underlying(UnitI::kExternal)) };
    auto* external_sku { new FilterUnit(tree_model_i, ext_filter_model, table_view) };

    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kExternalSku), external_sku);

    auto* line { new Line(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kDescription), line);

    auto* price { new Double(config.rate_decimal, kMinNumeric_12_4, kMaxNumeric_12_4, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kUnitPrice), price);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kDiscountPrice), price);

    auto* quantity { new Double(config.amount_decimal, kMinNumeric_12_4, kMaxNumeric_12_4, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kCount), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kMeasure), quantity);

    auto* amount { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kInitial), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kDiscount), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kFinal), amount);
}

void MainWindow::CreateSection(SectionContext& sc, CString& name)
{
    auto* tab_widget { ui->tabWidget };
    auto tree_widget { sc.tree_widget };

    auto view { sc.tree_view };
    auto tree_model { sc.tree_model };
    auto entry_hub { sc.entry_hub };

    const auto& info { sc.info };
    const auto& config { sc.section_config };
    const auto& section { info.section };

    // ReadSettings must before SetTreeView
    WidgetUtils::ReadConfig(view->header(), &QHeaderView::restoreState, section_settings_, info.node, kHeaderState);

    SetTreeView(view, info);

    auto* tab_bar = tab_widget->tabBar();
    const int index { tab_widget->addTab(tree_widget, name) };

    tab_bar->setTabData(index, QVariant::fromValue(TabInfo { section, QUuid() }));
    tab_bar->setTabButton(index, QTabBar::RightSide, nullptr);

    switch (info.section) {
    case Section::kFinance:
        TreeDelegateF(view, info, config);
        TreeConnectF(view, tree_model, entry_hub);
        break;
    case Section::kTask:
        TreeDelegateT(view, info, config);
        TreeConnectT(view, tree_model, entry_hub);
        break;
    case Section::kPartner:
        TreeDelegateP(view, info, config);
        TreeConnectP(view, tree_model, entry_hub);
        break;
    case Section::kInventory:
        TreeDelegateI(view, info, config);
        TreeConnectI(view, tree_model, entry_hub);
        break;
    case Section::kSale:
    case Section::kPurchase:
        TreeDelegateO(view, info, config);
        TreeConnectO(view, tree_model);
        break;
    default:
        break;
    }
}

void MainWindow::TreeDelegateF(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const
{
    auto* line { new Line(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kCode), line);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kDescription), line);

    auto* plain_text { new PlainText(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kNote), plain_text);

    auto* direction_rule { new BoolString(info.rule_map, QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kDirectionRule), direction_rule);

    auto* unit { new IntStringR(info.unit_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kUnit), unit);

    auto* kind { new IntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kKind), kind);

    auto* final_total { new DoubleSpinUnitR(section.amount_decimal, sc_f_.shared_config.default_unit, info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kFinalTotal), final_total);

    auto* initial_total { new FinanceForeignR(section.amount_decimal, sc_f_.shared_config.default_unit, info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kInitialTotal), initial_total);
}

void MainWindow::TreeDelegateT(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const
{
    auto* line { new Line(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kCode), line);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kDescription), line);

    auto* plain_text { new PlainText(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kNote), plain_text);

    auto* direction_rule { new BoolString(info.rule_map, QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kDirectionRule), direction_rule);

    auto* unit { new IntStringR(info.unit_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kUnit), unit);

    auto* kind { new IntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kKind), kind);

    auto* quantity { new DoubleSpinR(section.amount_decimal, kCoefficient16, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kInitialTotal), quantity);

    auto* amount { new DoubleSpinUnitR(section.amount_decimal, sc_f_.shared_config.default_unit, sc_f_.info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kFinalTotal), amount);

    auto* color { new Color(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kColor), color);

    auto* status { new Status(QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kStatus), status);

    auto* document { new Document(sc_t_.shared_config.document_dir, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kDocument), document);

    auto* issued_time { new TreeIssuedTime(section.date_format, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kIssuedTime), issued_time);
}

void MainWindow::TreeDelegateI(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const
{
    auto* line { new Line(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kCode), line);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kDescription), line);

    auto* plain_text { new PlainText(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kNote), plain_text);

    auto* direction_rule { new BoolString(info.rule_map, QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kDirectionRule), direction_rule);

    auto* unit { new IntStringR(info.unit_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kUnit), unit);

    auto* kind { new IntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kKind), kind);

    auto* quantity { new DoubleSpinR(section.amount_decimal, kCoefficient16, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kInitialTotal), quantity);

    auto* amount { new DoubleSpinUnitR(section.amount_decimal, sc_f_.shared_config.default_unit, sc_f_.info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kFinalTotal), amount);

    auto* unit_price { new Double(section.rate_decimal, 0, kMaxNumeric_12_4, kCoefficient8, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kUnitPrice), unit_price);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kCommission), unit_price);

    auto* color { new Color(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kColor), color);
}

void MainWindow::TreeDelegateP(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const
{
    auto* line { new Line(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kCode), line);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kDescription), line);

    auto* plain_text { new PlainText(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kNote), plain_text);

    auto* unit { new IntStringR(info.unit_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kUnit), unit);

    auto* kind { new IntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kKind), kind);

    auto* amount { new DoubleSpinUnitR(section.amount_decimal, sc_f_.shared_config.default_unit, sc_f_.info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kFinalTotal), amount);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kInitialTotal), amount);

    auto* payment_term { new Int(0, 36500, tree_view) }; // one hundred years
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumP::kPaymentTerm), payment_term);
}

void MainWindow::TreeDelegateO(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const
{
    auto* line { new Line(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kDescription), line);

    auto* direction_rule { new OrderRule(info.rule_map, QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kDirectionRule), direction_rule);

    auto* unit { new OrderUnit(info.unit_map, info.unit_model, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kUnit), unit);

    auto* kind { new IntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kKind), kind);

    auto* amount { new DoubleSpinUnitR(section.amount_decimal, sc_f_.shared_config.default_unit, sc_f_.info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kInitialTotal), amount);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kFinalTotal), amount);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kDiscountTotal), amount);

    auto* quantity { new DoubleSpinNoneZeroR(section.amount_decimal, kCoefficient16, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kMeasureTotal), quantity);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kCountTotal), quantity);

    auto tree_model_p { sc_p_.tree_model };

    auto* filter_model { tree_model_p->IncludeUnitModel(std::to_underlying(UnitP::kEmployee)) };
    auto* employee { new FilterUnit(tree_model_p, filter_model, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kEmployee), employee);

    auto* name { new OrderNameR(tree_model_p, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kName), name);

    auto* issued_time { new TreeIssuedTime(section.date_format, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kIssuedTime), issued_time);

    auto* status { new Status(QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kStatus), status);
}

void MainWindow::TreeConnectF(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SResizeColumnToContents, tree_view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRemoveEntryHash, LeafSStation::Instance(), &LeafSStation::RRemoveEntryHash, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveMultiEntry, LeafSStation::Instance(), &LeafSStation::RRemoveMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendMultiEntry, LeafSStation::Instance(), &LeafSStation::RAppendMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRefreshStatus, LeafSStation::Instance(), &LeafSStation::RRefreshStatus, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAppendOneEntry, LeafSStation::Instance(), &LeafSStation::RAppendOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveOneEntry, LeafSStation::Instance(), &LeafSStation::RRemoveOneEntry, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshField, LeafSStation::Instance(), &LeafSStation::RRefreshField, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SUpdateBalance, LeafSStation::Instance(), &LeafSStation::RUpdateBalance, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SDirectionRule, LeafSStation::Instance(), &LeafSStation::RDirectionRule, Qt::UniqueConnection);
}

void MainWindow::TreeConnectI(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SResizeColumnToContents, tree_view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRemoveEntryHash, LeafSStation::Instance(), &LeafSStation::RRemoveEntryHash, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveMultiEntry, LeafSStation::Instance(), &LeafSStation::RRemoveMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendMultiEntry, LeafSStation::Instance(), &LeafSStation::RAppendMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRefreshStatus, LeafSStation::Instance(), &LeafSStation::RRefreshStatus, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAppendOneEntry, LeafSStation::Instance(), &LeafSStation::RAppendOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveOneEntry, LeafSStation::Instance(), &LeafSStation::RRemoveOneEntry, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshField, LeafSStation::Instance(), &LeafSStation::RRefreshField, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SUpdateBalance, LeafSStation::Instance(), &LeafSStation::RUpdateBalance, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SDirectionRule, LeafSStation::Instance(), &LeafSStation::RDirectionRule, Qt::UniqueConnection);
}

void MainWindow::TreeConnectT(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SResizeColumnToContents, tree_view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRemoveEntryHash, LeafSStation::Instance(), &LeafSStation::RRemoveEntryHash, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveMultiEntry, LeafSStation::Instance(), &LeafSStation::RRemoveMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendMultiEntry, LeafSStation::Instance(), &LeafSStation::RAppendMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRefreshStatus, LeafSStation::Instance(), &LeafSStation::RRefreshStatus, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAppendOneEntry, LeafSStation::Instance(), &LeafSStation::RAppendOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveOneEntry, LeafSStation::Instance(), &LeafSStation::RRemoveOneEntry, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshField, LeafSStation::Instance(), &LeafSStation::RRefreshField, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SUpdateBalance, LeafSStation::Instance(), &LeafSStation::RUpdateBalance, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SDirectionRule, LeafSStation::Instance(), &LeafSStation::RDirectionRule, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SNodeStatus, LeafSStation::Instance(), &LeafSStation::RNodeStatus, Qt::UniqueConnection);
}

void MainWindow::TreeConnectP(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAppendMultiEntry, LeafSStation::Instance(), &LeafSStation::RAppendMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendOneEntry, LeafSStation::Instance(), &LeafSStation::RAppendOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveOneEntry, LeafSStation::Instance(), &LeafSStation::RRemoveOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRefreshField, LeafSStation::Instance(), &LeafSStation::RRefreshField, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SResizeColumnToContents, tree_view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshStatus, LeafSStation::Instance(), &LeafSStation::RRefreshStatus, Qt::UniqueConnection);
}

void MainWindow::TreeConnectO(QTreeView* tree_view, TreeModel* tree_model) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SResizeColumnToContents, tree_view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);
}

void MainWindow::InsertNodeFunction(const QModelIndex& parent, const QUuid& parent_id, int row)
{
    auto model { sc_->tree_model };

    auto* node { NodePool::Instance().Allocate(start_) };

    node->id = QUuid::createUuidV7();
    node->direction_rule = model->Rule(parent_id);
    node->unit = parent_id.isNull() ? sc_->shared_config.default_unit : model->Unit(parent_id);

    model->SetParent(node, parent_id);

    if (start_ == Section::kSale || start_ == Section::kPurchase)
        InsertNodeO(node, parent, row);

    if (start_ != Section::kSale && start_ != Section::kPurchase)
        InsertNodeFIPT(node, parent, parent_id, row);
}

void MainWindow::on_actionNewGroup_triggered()
{
    if (start_ != Section::kSale && start_ != Section::kPurchase)
        return;

    auto current_index { sc_->tree_view->currentIndex() };
    current_index = current_index.isValid() ? current_index : QModelIndex();

    auto parent_index { current_index.parent() };
    parent_index = parent_index.isValid() ? parent_index : QModelIndex();

    const QUuid parent_id { parent_index.isValid() ? parent_index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() : QUuid() };

    auto model { sc_->tree_model };

    auto* node { NodePool::Instance().Allocate(start_) };

    node->id = QUuid::createUuidV7();
    node->unit = parent_id.isNull() ? sc_->shared_config.default_unit : model->Unit(parent_id);
    node->kind = std::to_underlying(NodeKind::kBranch);

    model->SetParent(node, parent_id);

    auto tree_model { sc_->tree_model };
    auto unit_model { sc_->info.unit_model };

    auto parent_path { tree_model->Path(parent_id) };
    if (!parent_path.isEmpty())
        parent_path += app_config_.separator;

    const auto children_name { tree_model->ChildrenName(parent_id) };
    const int row { current_index.row() + 1 };

    QDialog* dialog { new InsertNodeBranch(node, unit_model, parent_path, children_name, this) };

    connect(dialog, &QDialog::accepted, this, [=, this]() {
        if (tree_model->InsertNode(row, parent_index, node)) {
            auto index { tree_model->index(row, 0, parent_index) };
            sc_->tree_view->setCurrentIndex(index);
        }
    });

    connect(dialog, &QDialog::rejected, this, [=, this]() { NodePool::Instance().Recycle(node, start_); });
    dialog->exec();
}

void MainWindow::on_actionRemove_triggered()
{
    auto* widget { ui->tabWidget->currentWidget() };
    assert(widget);

    if (dynamic_cast<TreeWidget*>(widget)) {
        RemoveNode();
    }

    if (auto* leaf_widget { dynamic_cast<LeafWidget*>(widget) }) {
        WidgetUtils::RemoveEntry(leaf_widget);
    }
}

void MainWindow::RemoveNode()
{
    const auto index { sc_->tree_view->currentIndex() };
    if (!index.isValid())
        return;

    auto model { sc_->tree_model };
    assert(model);

    const QUuid node_id { index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() };
    if (node_pending_removal_.contains(node_id))
        return;

    node_pending_removal_.insert(node_id);

    const NodeKind kind { model->Kind(node_id) };

    switch (kind) {
    case NodeKind::kBranch: {
        BranchRemove(model, index, node_id);
        break;
    }
    case NodeKind::kLeaf: {
        const auto message { JsonGen::LeafRemoveCheck(sc_->info.section, node_id) };
        WebSocket::Instance()->SendMessage(kLeafRemoveCheck, message);
        break;
    }
    default:
        break;
    }
}

void MainWindow::RLeafRemoveDenied(const QJsonObject& obj)
{
    Section section { obj.value(kSection).toInt() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };

    auto* section_contex = GetSectionContex(section);

    auto model { section_contex->tree_model };
    const int unit { model->Unit(node_id) };

    auto* dialog { new LeafRemoveDialog(model, section_contex->info, obj, node_id, unit, this) };
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(true);
    dialog->show();

    connect(dialog, &LeafRemoveDialog::SRemoveNode, model, &TreeModel::RRemoveNode, Qt::SingleShotConnection);
    connect(dialog, &QObject::destroyed, this, [this, node_id]() { node_pending_removal_.remove(node_id); });
}

void MainWindow::RSharedConfig(const QJsonArray& arr)
{
    for (const QJsonValue& val : arr) {
        if (!val.isObject()) {
            qWarning() << "Invalid item in GlobalConfig array:" << val;
            continue;
        }

        QJsonObject obj { val.toObject() };
        Section section { obj.value("section").toInt() };
        int default_unit { obj.value("default_unit").toInt() };
        QString document_dir { obj.value("document_dir").toString() };

        auto* section_contex = GetSectionContex(section);
        if (!section_contex) {
            continue;
        }

        section_contex->shared_config.default_unit = default_unit;
        section_contex->shared_config.document_dir = document_dir;
    }
}

void MainWindow::RDocumentDir(Section section, const QString& document_dir)
{
    auto* sc { GetSectionContex(section) };
    sc->shared_config.document_dir = document_dir;
}

void MainWindow::RDefaultUnit(Section section, int unit)
{
    auto* sc { GetSectionContex(section) };
    sc->shared_config.default_unit = unit;
}

void MainWindow::RUpdateDefaultUnitFailed(const QString& /*section*/)
{
    QMessageBox::warning(this, tr("Update Failed"), tr("Cannot change the base unit for section Finance because related entries already exist."));
}

// RScrollToEntry - Scroll to and select the specified entry (slot)
// -------------------------------
// Behavior:
//  1. Skip if entry_id is null (no target to scroll to)
//  2. Find and select the entry in the view
//  3. Scroll to center the IssuedTime column
//  4. Close any persistent editor on that row
void MainWindow::RScrollToEntry(const QUuid& node_id, const QUuid& entry_id)
{
    if (entry_id.isNull())
        return;

    auto widget { sc_->leaf_wgt_hash.value(node_id, nullptr) };
    assert(widget);

    auto* view { widget->View() };
    auto index { widget->Model()->GetIndex(entry_id) };

    if (!index.isValid())
        return;

    view->setCurrentIndex(index);
    view->scrollTo(index.siblingAtColumn(std::to_underlying(EntryEnum::kIssuedTime)), QAbstractItemView::PositionAtCenter);
    view->closePersistentEditor(index);
}

void MainWindow::ClearMainwindow()
{
    WriteConfig();

    if (!section_settings_)
        return;

    if (settlement_widget_) {
        settlement_widget_->close();
        delete settlement_widget_;
        settlement_widget_.clear();
    }

    print_template_.clear();
    app_settings_.clear();
    section_settings_.clear();

    sc_f_.Clear();
    sc_i_.Clear();
    sc_t_.Clear();
    sc_p_.Clear();
    sc_sale_.Clear();
    sc_purchase_.Clear();

    ui->tabWidget->clear();
}

void MainWindow::SetAction(bool enable) const
{
    ui->actionAppendNode->setEnabled(enable);
    ui->actionMarkAll->setEnabled(enable);
    ui->actionMarkNone->setEnabled(enable);
    ui->actionMarkToggle->setEnabled(enable);
    ui->actionEditName->setEnabled(enable);
    ui->actionInsertNode->setEnabled(enable);
    ui->actionJump->setEnabled(enable);
    ui->actionPreferences->setEnabled(enable);
    ui->actionSearch->setEnabled(enable);
    ui->actionRemove->setEnabled(enable);
    ui->actionAppendEntry->setEnabled(enable);
    ui->actionExportExcel->setEnabled(enable);
    ui->actionStatement->setEnabled(enable);
    ui->actionSettlement->setEnabled(enable);
    ui->actionResetColor->setEnabled(enable);
    ui->actionNewGroup->setEnabled(enable);
}

void MainWindow::IniSectionGroup()
{
    section_group_ = new QButtonGroup(this);

    section_group_->addButton(ui->rBtnFinance, std::to_underlying(Section::kFinance));
    section_group_->addButton(ui->rBtnTask, std::to_underlying(Section::kTask));
    section_group_->addButton(ui->rBtnInventory, std::to_underlying(Section::kInventory));
    section_group_->addButton(ui->rBtnPartner, std::to_underlying(Section::kPartner));
    section_group_->addButton(ui->rBtnSale, std::to_underlying(Section::kSale));
    section_group_->addButton(ui->rBtnPurchase, std::to_underlying(Section::kPurchase));
}

void MainWindow::IniMarkGroup()
{
    mark_group_ = new QActionGroup(this);

    ui->actionMarkAll->setData(std::to_underlying(EntryAction::kMarkAll));
    ui->actionMarkNone->setData(std::to_underlying(EntryAction::kMarkNone));
    ui->actionMarkToggle->setData(std::to_underlying(EntryAction::kMarkToggle));

    mark_group_->addAction(ui->actionMarkAll);
    mark_group_->addAction(ui->actionMarkNone);
    mark_group_->addAction(ui->actionMarkToggle);

    connect(mark_group_, &QActionGroup::triggered, this, [this](QAction* action) {
        const int action_id { action->data().toInt() };
        RActionEntry(static_cast<EntryAction>(action_id));
    });
}

void MainWindow::BranchRemove(TreeModel* tree_model, const QModelIndex& index, const QUuid& node_id)
{
    QMessageBox msg {};
    msg.setIcon(QMessageBox::Question);
    msg.setText(tr("Remove %1").arg(tree_model->Path(node_id)));
    msg.setInformativeText(tr("The branch will be removed, and its direct children will be promoted to the same level."));
    msg.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);

    if (msg.exec() == QMessageBox::Ok) {
        const auto message { JsonGen::BranchRemove(sc_->info.section, node_id) };
        WebSocket::Instance()->SendMessage(kBranchRemove, message);
        tree_model->removeRows(index.row(), 1, index.parent());
        node_pending_removal_.remove(node_id);
    }
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    if (index == 0)
        return;

    const auto node_id { ui->tabWidget->tabBar()->tabData(index).value<TabInfo>().id };

    if (start_ != Section::kFinance && start_ != Section::kTask)
        WidgetUtils::FreeWidget(node_id, sc_->rpt_wgt_hash);

    RFreeWidget(node_id);
}

void MainWindow::RFreeWidget(const QUuid& node_id)
{
    WidgetUtils::FreeWidget(node_id, sc_->leaf_wgt_hash);
    LeafSStation::Instance()->DeregisterModel(node_id);
}

void MainWindow::SetTabWidget()
{
    auto* tab_widget { ui->tabWidget };
    auto* tab_bar { tab_widget->tabBar() };

    tab_bar->setDocumentMode(true);
    tab_bar->setExpanding(false);

    tab_widget->setMovable(true);
    tab_widget->setTabsClosable(true);
    tab_widget->setElideMode(Qt::ElideNone);

    const int start_section { app_settings_->value(kStartSection, 0).toInt() };

    start_ = Section(start_section);
    section_group_->button(start_section)->setChecked(true);
}

void MainWindow::SetTableView(QTableView* view, int stretch_column, int lhs_node_column) const
{
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAlternatingRowColors(true);
    view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::CurrentChanged);

    view->setColumnHidden(std::to_underlying(EntryEnum::kId), kIsHidden);
    view->setColumnHidden(lhs_node_column, kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kUserId), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kCreateBy), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kCreateTime), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kUpdateTime), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kUpdateBy), kIsHidden);

    auto* h_header { view->horizontalHeader() };
    ResizeColumn(h_header, stretch_column);

    auto* v_header { view->verticalHeader() };
    v_header->setDefaultSectionSize(kRowHeight);
    v_header->setSectionResizeMode(QHeaderView::Fixed);
    v_header->setHidden(true);

    view->setSortingEnabled(true);
}

void MainWindow::on_actionStatement_triggered()
{
    assert(start_ == Section::kSale || start_ == Section::kPurchase);

    auto* model { new StatementModel(sc_->entry_hub, sc_->info, this) };

    const int unit { std::to_underlying(UnitO::kMonthly) };
    const auto start { QDateTime(QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1), kStartTime) };
    const auto end { QDateTime(QDate(QDate::currentDate().year(), QDate::currentDate().month(), QDate::currentDate().daysInMonth()), kEndTime) };

    auto* widget { new StatementWidget(model, unit, false, start, end, this) };

    const int tab_index { ui->tabWidget->addTab(widget, tr("Statement")) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    const QUuid report_id { QUuid::createUuidV7() };
    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, report_id }));

    auto* view { widget->View() };
    SetStatementView(view, std::to_underlying(StatementEnum::kPlaceholder));
    DelegateStatement(view, sc_->section_config);

    connect(widget, &StatementWidget::SStatementPrimary, this, &MainWindow::RStatementPrimary);
    connect(widget, &StatementWidget::SStatementSecondary, this, &MainWindow::RStatementSecondary);
    connect(widget, &StatementWidget::SResetModel, model, &StatementModel::RResetModel);

    RegisterRptWgt(report_id, widget);
}

void MainWindow::on_actionSettlement_triggered()
{
    assert(start_ == Section::kSale || start_ == Section::kPurchase);

    if (settlement_widget_) {
        ui->tabWidget->setCurrentWidget(settlement_widget_);
        settlement_widget_->activateWindow();
        return;
    }

    const auto start { QDateTime(QDate(QDate::currentDate().year() - 1, 1, 1), kStartTime) };
    const auto end { QDateTime(QDate(QDate::currentDate().year(), 12, 31), kEndTime) };

    auto* model { new SettlementModel(sc_->entry_hub, sc_->info, this) };
    model->ResetModel(start, end);

    auto* primary_model { new SettlementPrimaryModel(sc_->entry_hub, sc_->info, this) };

    settlement_widget_ = new SettlementWidget(model, primary_model, start, end, this);

    const int tab_index { ui->tabWidget->addTab(settlement_widget_, tr("Settlement")) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    const QUuid report_id { QUuid::createUuidV7() };
    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, report_id }));

    auto* view { settlement_widget_->View() };
    auto primary_view { settlement_widget_->PrimaryView() };
    SetStatementView(view, std::to_underlying(SettlementEnum::kDescription));
    SetStatementView(primary_view, std::to_underlying(SettlementEnum::kDescription));

    view->setColumnHidden(std::to_underlying(SettlementEnum::kId), false);
    primary_view->setColumnHidden(std::to_underlying(SettlementEnum::kId), false);

    DelegateSettlement(view, sc_->section_config);
    DelegateSettlementPrimary(primary_view, sc_->section_config);

    connect(model, &SettlementModel::SResetModel, primary_model, &SettlementPrimaryModel::RResetModel);
    connect(model, &SettlementModel::SSyncFinished, primary_model, &SettlementPrimaryModel::RSyncFinished);
    connect(model, &SettlementModel::SResizeColumnToContents, view, &QTableView::resizeColumnToContents);

    connect(primary_model, &SettlementPrimaryModel::SSyncDouble, model, &SettlementModel::RSyncDouble);
    connect(model, &SettlementModel::SUpdateAmount, static_cast<TreeModelP*>(sc_p_.tree_model.data()), &TreeModelP::RUpdateAmount);
    connect(settlement_widget_, &SettlementWidget::SNodeLocation, this, &MainWindow::RNodeLocation);

    RegisterRptWgt(report_id, settlement_widget_);
}

void MainWindow::CreateLeafExternalReference(TreeModel* tree_model, CSectionInfo& info, const QUuid& node_id, int unit)
{
    assert(tree_model);
    assert(tree_model->Contains(node_id));

    CString name { tr("Record-") + tree_model->Name(node_id) };

    const Section section { info.section };

    auto* model { new EntryRefModel(sc_->entry_hub, info, unit, this) };

    const auto start { QDateTime(QDate(QDate::currentDate().year() - 1, 1, 1), kStartTime) };
    const auto end { QDateTime(QDate(QDate::currentDate().year(), 12, 31), kEndTime) };
    auto* widget { new RefWidget(model, node_id, start, end, this) };

    const int tab_index { ui->tabWidget->addTab(widget, name) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    const QUuid report_id { QUuid::createUuidV7() };
    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { section, report_id }));

    auto* view { widget->View() };
    SetTableView(view, std::to_underlying(EntryRefEnum::kDescription), std::to_underlying(EntryRefEnum::kPIId));
    DelegateLeafExternalReference(view, sc_sale_.section_config);

    connect(view, &QTableView::doubleClicked, this, &MainWindow::REntryRefDoubleClicked);
    connect(widget, &RefWidget::SResetModel, model, &EntryRefModel::RResetModel);

    RegisterRptWgt(report_id, widget);
}

void MainWindow::RegisterRptWgt(const QUuid& report_id, ReportWidget* widget)
{
    sc_->rpt_wgt_hash.insert(report_id, widget);
    ui->tabWidget->setCurrentWidget(widget);
    widget->activateWindow();
}

void MainWindow::WriteConfig()
{
    if (app_settings_) {
        WidgetUtils::WriteConfig(ui->splitter, &QSplitter::saveState, app_settings_, kSplitter, kState);
        WidgetUtils::WriteConfig(this, &QMainWindow::saveState, app_settings_, kMainwindow, kState, 0);
        WidgetUtils::WriteConfig(this, &QMainWindow::saveGeometry, app_settings_, kMainwindow, kGeometry);
        WidgetUtils::WriteConfig(app_settings_, std::to_underlying(start_), kStart, kSection);
    }

    if (section_settings_) {
        WidgetUtils::WriteConfig(sc_f_.tree_view->header(), &QHeaderView::saveState, section_settings_, kFinance, kHeaderState);
        WidgetUtils::WriteConfig(sc_i_.tree_view->header(), &QHeaderView::saveState, section_settings_, kInventory, kHeaderState);
        WidgetUtils::WriteConfig(sc_p_.tree_view->header(), &QHeaderView::saveState, section_settings_, kPartner, kHeaderState);
        WidgetUtils::WriteConfig(sc_t_.tree_view->header(), &QHeaderView::saveState, section_settings_, kTask, kHeaderState);
        WidgetUtils::WriteConfig(sc_sale_.tree_view->header(), &QHeaderView::saveState, section_settings_, kSale, kHeaderState);
        WidgetUtils::WriteConfig(sc_purchase_.tree_view->header(), &QHeaderView::saveState, section_settings_, kPurchase, kHeaderState);
    }
}

SectionContext* MainWindow::GetSectionContex(Section section)
{
    const static QMap<Section, SectionContext*> section_map {
        { Section::kFinance, &sc_f_ },
        { Section::kPartner, &sc_p_ },
        { Section::kInventory, &sc_i_ },
        { Section::kTask, &sc_t_ },
        { Section::kSale, &sc_sale_ },
        { Section::kPurchase, &sc_purchase_ },
    };

    auto it = section_map.constFind(section);
    if (it == section_map.cend()) {
        qCritical() << "SectionTriple: Unknown section";
        return nullptr;
    }

    return it.value();
}

void MainWindow::InitSystemTray()
{
    tray_icon_ = new QSystemTrayIcon(this);

#if defined(Q_OS_WIN)
    tray_icon_->setIcon(QIcon(":/logo/logo/logo.ico"));
#elif defined(Q_OS_MACOS)
    tray_icon_->setIcon(QIcon(":/logo/logo/logo.icns"));

    connect(qApp, &QApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
        if (state == Qt::ApplicationActive && !this->isVisible()) {
            this->showNormal();
            this->raise();
            this->activateWindow();
        }
    });
#else
    tray_icon_->setIcon(QIcon(":/logo/logo/logo.png"));
#endif

    tray_icon_->setToolTip("YTX");

    tray_menu_ = new QMenu(this);
    tray_menu_->addAction(QObject::tr("Show Window"), this, [this] {
        this->showNormal();
        this->raise();
        this->activateWindow();
    });

    tray_menu_->addSeparator();
    tray_menu_->addAction(QObject::tr("Quit"), qApp, &QCoreApplication::quit);

    tray_icon_->setContextMenu(tray_menu_);
    tray_icon_->show();

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    connect(tray_icon_, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            this->showNormal();
            this->raise();
            this->activateWindow();
        }
    });
#endif
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (tray_icon_ && tray_icon_->isVisible()) {
        hide();
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::DelegateLeafExternalReference(QTableView* table_view, CSectionConfig& config) const
{
    auto* price { new DoubleSpinNoneZeroR(config.rate_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kUnitPrice), price);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kDiscountPrice), price);

    auto* quantity { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kkCount), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kkMeasure), quantity);

    auto* amount { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kInitial), amount);

    auto* issued_time { new IssuedTimeR(sc_sale_.section_config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kIssuedTime), issued_time);

    auto partner_tree_model { sc_p_.tree_model };
    auto* external_sku { new NodePathR(partner_tree_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kExternalSku), external_sku);

    auto* section { new SectionR(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kSection), section);

    if (start_ == Section::kInventory) {
        auto* name { new NodeNameR(partner_tree_model, table_view) };
        table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kPIId), name);
    }

    if (start_ == Section::kPartner) {
        auto* internal_sku { new NodeNameR(sc_i_.tree_model, table_view) };
        table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kPIId), internal_sku);
    }
}

void MainWindow::SetStatementView(QTableView* view, int stretch_column) const
{
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAlternatingRowColors(true);

    auto* h_header { view->horizontalHeader() };
    ResizeColumn(h_header, stretch_column);

    auto* v_header { view->verticalHeader() };
    v_header->setDefaultSectionSize(kRowHeight);
    v_header->setSectionResizeMode(QHeaderView::Fixed);
    v_header->setHidden(true);
}

void MainWindow::DelegateStatement(QTableView* table_view, CSectionConfig& config) const
{
    auto* quantity { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCCount), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCMeasure), quantity);

    auto* amount { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCGrossAmount), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCSettlement), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kPBalance), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCBalance), amount);

    auto* name { new NodeNameR(sc_p_.tree_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kPartner), name);
}

void MainWindow::DelegateSettlement(QTableView* table_view, CSectionConfig& config) const
{
    auto* amount { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kInitialTotal), amount);

    auto* status { new Status(QEvent::MouseButtonDblClick, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kStatus), status);

    auto* line { new Line(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kDescription), line);

    auto* issued_time { new TreeIssuedTime(kDateFST, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kIssuedTime), issued_time);

    auto model { sc_p_.tree_model };
    const int unit { start_ == Section::kSale ? std::to_underlying(UnitP::kCustomer) : std::to_underlying(UnitP::kVendor) };

    auto* filter_model { model->IncludeUnitModel(unit) };
    auto* node { new TableComboFilter(model, filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kPartner), node);
}

void MainWindow::DelegateSettlementPrimary(QTableView* table_view, CSectionConfig& config) const
{
    auto* amount { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kInitialTotal), amount);

    auto* employee { new NodeNameR(sc_p_.tree_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kPartner), employee);

    auto* status { new Status(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kStatus), status);

    auto* issued_time { new IssuedTimeR(kDateFST, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kIssuedTime), issued_time);
}

void MainWindow::DelegateStatementPrimary(QTableView* table_view, CSectionConfig& config) const
{
    auto* quantity { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementPrimaryEnum::kCount), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementPrimaryEnum::kMeasure), quantity);

    auto* amount { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementPrimaryEnum::kInitialTotal), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementPrimaryEnum::kFinalTotal), amount);

    auto* employee { new NodeNameR(sc_p_.tree_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementPrimaryEnum::kEmployee), employee);

    auto* status { new Status(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementPrimaryEnum::kStatus), status);

    auto* issued_time { new IssuedTimeR(sc_sale_.section_config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementPrimaryEnum::kIssuedTime), issued_time);
}

void MainWindow::DelegateStatementSecondary(QTableView* table_view, CSectionConfig& config) const
{
    auto* quantity { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kCount), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kMeasure), quantity);

    auto* amount { new DoubleSpinNoneZeroR(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kInitialTotal), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kFinalTotal), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kUnitPrice), amount);

    auto* status { new Status(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kStatus), status);

    auto* issued_time { new IssuedTimeR(sc_sale_.section_config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kIssuedTime), issued_time);

    auto* external_sku { new NodePathR(sc_p_.tree_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kSupportNode), external_sku);

    auto tree_model_i { sc_i_.tree_model };

    auto* int_filter_model { tree_model_i->IncludeUnitModel(std::to_underlying(UnitI::kInternal)) };
    auto* internal_sku { new FilterUnit(tree_model_i, int_filter_model, table_view) };

    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kRhsNode), internal_sku);
}

void MainWindow::SetUniqueConnection() const
{
    connect(ui->actionQuit, &QAction::triggered, qApp, &QCoreApplication::quit);

    connect(section_group_, &QButtonGroup::idClicked, this, &MainWindow::RSectionGroup);

    connect(WebSocket::Instance(), &WebSocket::SInitializeContext, this, &MainWindow::RInitializeContext);
    connect(WebSocket::Instance(), &WebSocket::SLeafRemoveDenied, this, &MainWindow::RLeafRemoveDenied);
    connect(WebSocket::Instance(), &WebSocket::SSharedConfig, this, &MainWindow::RSharedConfig);
    connect(WebSocket::Instance(), &WebSocket::SDefaultUnit, this, &MainWindow::RDefaultUnit);
    connect(WebSocket::Instance(), &WebSocket::SUpdateDefaultUnitFailed, this, &MainWindow::RUpdateDefaultUnitFailed);
    connect(WebSocket::Instance(), &WebSocket::SDocumentDir, this, &MainWindow::RDocumentDir);
    connect(WebSocket::Instance(), &WebSocket::SConnectResult, this, &MainWindow::RConnectResult);
    connect(WebSocket::Instance(), &WebSocket::SLoginResult, this, &MainWindow::RLoginResult);
    connect(WebSocket::Instance(), &WebSocket::SRemoteHostClosed, this, &MainWindow::on_actionLogout_triggered);
    connect(WebSocket::Instance(), &WebSocket::SScrollToEntry, this, &MainWindow::RScrollToEntry);
}

void MainWindow::InitContextFinance()
{
    auto& info { sc_f_.info };
    auto& section_config { sc_f_.section_config };
    auto& shared_config { sc_f_.shared_config };
    auto& entry_hub { sc_f_.entry_hub };
    auto& tree_model { sc_f_.tree_model };
    auto& tree_view { sc_f_.tree_view };
    auto& tree_widget { sc_f_.tree_widget };

    info.section = Section::kFinance;
    info.node = kFinanceNode;
    info.path = kFinancePath;
    info.entry = kFinanceEntry;

    QStringList unit_list { "CNY", "HKD", "USD", "GBP", "JPY", "CAD", "AUD", "EUR" };
    QStringList unit_symbol_list { "¥", "$", "$", "£", "¥", "$", "$", "€" };

    for (int i = 0; i != unit_list.size(); ++i) {
        info.unit_map.insert(i, unit_list.at(i));
        info.unit_symbol_map.insert(i, unit_symbol_list.at(i));
    }

    info.rule_map.insert(Rule::kDDCI, Rule::kStrDDCI);
    info.rule_map.insert(Rule::kDICD, Rule::kStrDICD);

    info.kind_map.insert(std::to_underlying(NodeKind::kBranch), kBranchKind);
    info.kind_map.insert(std::to_underlying(NodeKind::kLeaf), kLeafKind);

    info.unit_model = MainWindowUtils::CreateModelFromMap(info.unit_map, this);
    info.rule_model = MainWindowUtils::CreateModelFromMap(info.rule_map, this);

    ReadSectionConfig(section_config, kFinance);

    entry_hub = new EntryHubF(info, this);
    tree_model = new TreeModelF(info, app_config_.separator, shared_config.default_unit, this);

    WebSocket::Instance()->RegisterTreeModel(Section::kFinance, tree_model);
    WebSocket::Instance()->RegisterEntryHub(Section::kFinance, entry_hub);

    tree_widget = new TreeWidgetF(tree_model, info, shared_config, section_config, this);
    tree_view = tree_widget->View();

    connect(tree_model, &TreeModel::STotalsUpdated, tree_widget, &TreeWidget::RTotalsUpdated, Qt::UniqueConnection);
}

void MainWindow::InitContextInventory()
{
    auto& info { sc_i_.info };
    auto& section_config { sc_i_.section_config };
    auto& shared_config { sc_i_.shared_config };
    auto& entry_hub { sc_i_.entry_hub };
    auto& tree_model { sc_i_.tree_model };
    auto& tree_view { sc_i_.tree_view };
    auto& tree_widget { sc_i_.tree_widget };

    info.section = Section::kInventory;
    info.node = kInventoryNode;
    info.path = kInventoryPath;
    info.entry = kInventoryEntry;

    info.unit_map.insert(std::to_underlying(UnitI::kInternal), kUnitInternal);
    info.unit_map.insert(std::to_underlying(UnitI::kPosition), kUnitPosition);
    info.unit_map.insert(std::to_underlying(UnitI::kExternal), kUnitExternal);

    info.rule_map.insert(Rule::kDDCI, Rule::kStrDDCI);
    info.rule_map.insert(Rule::kDICD, Rule::kStrDICD);

    info.kind_map.insert(std::to_underlying(NodeKind::kBranch), kBranchKind);
    info.kind_map.insert(std::to_underlying(NodeKind::kLeaf), kLeafKind);

    info.unit_model = MainWindowUtils::CreateModelFromMap(info.unit_map, this);
    info.rule_model = MainWindowUtils::CreateModelFromMap(info.rule_map, this);

    ReadSectionConfig(section_config, kInventory);

    entry_hub = new EntryHubI(info, this);
    tree_model = new TreeModelI(info, app_config_.separator, shared_config.default_unit, this);

    WebSocket::Instance()->RegisterTreeModel(Section::kInventory, tree_model);
    WebSocket::Instance()->RegisterEntryHub(Section::kInventory, entry_hub);

    tree_widget = new TreeWidgetI(tree_model, section_config, this);
    tree_view = tree_widget->View();

    connect(tree_model, &TreeModel::STotalsUpdated, tree_widget, &TreeWidget::RTotalsUpdated, Qt::UniqueConnection);
}

void MainWindow::InitContextTask()
{
    auto& info { sc_t_.info };
    auto& section_config { sc_t_.section_config };
    auto& shared_config { sc_t_.shared_config };
    auto& entry_hub { sc_t_.entry_hub };
    auto& tree_model { sc_t_.tree_model };
    auto& tree_view { sc_t_.tree_view };
    auto& tree_widget { sc_t_.tree_widget };

    info.section = Section::kTask;
    info.node = kTaskNode;
    info.path = kTaskPath;
    info.entry = kTaskEntry;

    info.unit_map.insert(std::to_underlying(UnitT::kInternal), kUnitInternal);
    info.unit_map.insert(std::to_underlying(UnitT::kExternal), kUnitExternal);

    info.rule_map.insert(Rule::kDDCI, Rule::kStrDDCI);
    info.rule_map.insert(Rule::kDICD, Rule::kStrDICD);

    info.kind_map.insert(std::to_underlying(NodeKind::kBranch), kBranchKind);
    info.kind_map.insert(std::to_underlying(NodeKind::kLeaf), kLeafKind);

    info.unit_model = MainWindowUtils::CreateModelFromMap(info.unit_map, this);
    info.rule_model = MainWindowUtils::CreateModelFromMap(info.rule_map, this);

    ReadSectionConfig(section_config, kTask);

    entry_hub = new EntryHubT(info, this);
    tree_model = new TreeModelT(info, app_config_.separator, shared_config.default_unit, this);

    WebSocket::Instance()->RegisterTreeModel(Section::kTask, tree_model);
    WebSocket::Instance()->RegisterEntryHub(Section::kTask, entry_hub);

    const QDate today { QDate::currentDate() };
    const QDateTime start_dt(today, kStartTime);
    const QDateTime end_dt(today.addDays(1), kStartTime);

    tree_widget = new TreeWidgetTO(Section::kTask, tree_model, start_dt, end_dt, this);
    tree_view = tree_widget->View();
}

void MainWindow::InitContextPartner()
{
    auto& info { sc_p_.info };
    auto& section_config { sc_p_.section_config };
    auto& shared_config { sc_p_.shared_config };
    auto& entry_hub { sc_p_.entry_hub };
    auto& tree_model { sc_p_.tree_model };
    auto& tree_view { sc_p_.tree_view };
    auto& tree_widget { sc_p_.tree_widget };

    info.section = Section::kPartner;
    info.node = kPartnerNode;
    info.path = kPartnerPath;
    info.entry = kPartnerEntry;

    info.unit_map.insert(std::to_underlying(UnitP::kCustomer), kUnitCustomer);
    info.unit_map.insert(std::to_underlying(UnitP::kEmployee), kUnitEmployee);
    info.unit_map.insert(std::to_underlying(UnitP::kVendor), kUnitVendor);

    info.kind_map.insert(std::to_underlying(NodeKind::kBranch), kBranchKind);
    info.kind_map.insert(std::to_underlying(NodeKind::kLeaf), kLeafKind);

    info.unit_model = MainWindowUtils::CreateModelFromMap(info.unit_map, this);

    ReadSectionConfig(section_config, kPartner);

    entry_hub = new EntryHubP(info, this);
    tree_model = new TreeModelP(info, app_config_.separator, shared_config.default_unit, this);

    WebSocket::Instance()->RegisterTreeModel(Section::kPartner, tree_model);
    WebSocket::Instance()->RegisterEntryHub(Section::kPartner, entry_hub);

    tree_widget = new TreeWidgetP(tree_model, this);
    tree_view = tree_widget->View();
}

void MainWindow::InitContextSale()
{
    auto& info { sc_sale_.info };
    auto& section_config { sc_sale_.section_config };
    auto& shared_config { sc_sale_.shared_config };
    auto& entry_hub { sc_sale_.entry_hub };
    auto& tree_model { sc_sale_.tree_model };
    auto& tree_view { sc_sale_.tree_view };
    auto& tree_widget { sc_sale_.tree_widget };

    info.section = Section::kSale;
    info.node = kSaleNode;
    info.path = kSalePath;
    info.entry = kSaleEntry;
    info.settlement = kSaleSettlement;

    info.rule_map.insert(Rule::kRO, Rule::kStrRO);
    info.rule_map.insert(Rule::kTO, Rule::kStrTO);

    info.unit_map.insert(std::to_underlying(UnitO::kImmediate), kUnitImmediate);
    info.unit_map.insert(std::to_underlying(UnitO::kMonthly), kUnitMonthly);
    info.unit_map.insert(std::to_underlying(UnitO::kPending), kUnitPending);

    info.kind_map.insert(std::to_underlying(NodeKind::kBranch), kBranchKind);
    info.kind_map.insert(std::to_underlying(NodeKind::kLeaf), kLeafKind);

    info.unit_model = MainWindowUtils::CreateModelFromMap(info.unit_map, this);
    info.unit_model->sort(0, Qt::DescendingOrder);

    info.rule_model = MainWindowUtils::CreateModelFromMap(info.rule_map, this);

    ReadSectionConfig(section_config, kSale);

    auto* entry_hub_o = new EntryHubO(info, this);
    auto* tree_model_o = new TreeModelO(info, app_config_.separator, shared_config.default_unit, this);

    entry_hub = entry_hub_o;
    tree_model = tree_model_o;

    const QDate today { QDate::currentDate() };
    const QDateTime start_dt(today, kStartTime);
    const QDateTime end_dt(today.addDays(1), kStartTime);

    WebSocket::Instance()->RegisterTreeModel(Section::kSale, tree_model);
    WebSocket::Instance()->RegisterEntryHub(Section::kSale, entry_hub);

    tree_widget = new TreeWidgetTO(Section::kSale, tree_model, start_dt, end_dt, this);
    tree_view = tree_widget->View();

    connect(tree_model_o, &TreeModelO::SUpdateAmount, static_cast<TreeModelP*>(sc_p_.tree_model.data()), &TreeModelP::RUpdateAmount);
    connect(entry_hub_o, &EntryHubO::SSyncPrice, static_cast<EntryHubP*>(sc_p_.entry_hub.data()), &EntryHubP::RPriceSList);
}

void MainWindow::InitContextPurchase()
{
    auto& info { sc_purchase_.info };
    auto& section_config { sc_purchase_.section_config };
    auto& shared_config { sc_purchase_.shared_config };
    auto& entry_hub { sc_purchase_.entry_hub };
    auto& tree_model { sc_purchase_.tree_model };
    auto& tree_view { sc_purchase_.tree_view };
    auto& tree_widget { sc_purchase_.tree_widget };

    info.section = Section::kPurchase;
    info.node = kPurchaseNode;
    info.path = kPurchasePath;
    info.entry = kPurchaseEntry;
    info.settlement = kPurchaseSettlement;

    info.rule_map.insert(Rule::kRO, Rule::kStrRO);
    info.rule_map.insert(Rule::kTO, Rule::kStrTO);

    info.unit_map.insert(std::to_underlying(UnitO::kImmediate), kUnitImmediate);
    info.unit_map.insert(std::to_underlying(UnitO::kMonthly), kUnitMonthly);
    info.unit_map.insert(std::to_underlying(UnitO::kPending), kUnitPending);

    info.kind_map.insert(std::to_underlying(NodeKind::kBranch), kBranchKind);
    info.kind_map.insert(std::to_underlying(NodeKind::kLeaf), kLeafKind);

    info.unit_model = MainWindowUtils::CreateModelFromMap(info.unit_map, this);
    info.unit_model->sort(0, Qt::DescendingOrder);

    info.rule_model = MainWindowUtils::CreateModelFromMap(info.rule_map, this);

    ReadSectionConfig(section_config, kPurchase);

    auto* entry_hub_o = new EntryHubO(info, this);
    auto* tree_model_o = new TreeModelO(info, app_config_.separator, shared_config.default_unit, this);

    entry_hub = entry_hub_o;
    tree_model = tree_model_o;

    const QDate today { QDate::currentDate() };
    const QDateTime start_dt(today, kStartTime);
    const QDateTime end_dt(today.addDays(1), kStartTime);

    WebSocket::Instance()->RegisterTreeModel(Section::kPurchase, tree_model);
    WebSocket::Instance()->RegisterEntryHub(Section::kPurchase, entry_hub);

    tree_widget = new TreeWidgetTO(Section::kPurchase, tree_model, start_dt, end_dt, this);
    tree_view = tree_widget->View();

    connect(tree_model_o, &TreeModelO::SUpdateAmount, static_cast<TreeModelP*>(sc_p_.tree_model.data()), &TreeModelP::RUpdateAmount);
    connect(entry_hub_o, &EntryHubO::SSyncPrice, static_cast<EntryHubP*>(sc_p_.entry_hub.data()), &EntryHubP::RPriceSList);
}

void MainWindow::SetIcon() const
{
    ui->actionInsertNode->setIcon(QIcon(":/solarized_dark/solarized_dark/insert.png"));
    ui->actionEditName->setIcon(QIcon(":/solarized_dark/solarized_dark/edit.png"));
    ui->actionRemove->setIcon(QIcon(":/solarized_dark/solarized_dark/remove.png"));
    ui->actionAbout->setIcon(QIcon(":/solarized_dark/solarized_dark/about.png"));
    ui->actionAppendNode->setIcon(QIcon(":/solarized_dark/solarized_dark/append.png"));
    ui->actionJump->setIcon(QIcon(":/solarized_dark/solarized_dark/jump.png"));
    ui->actionPreferences->setIcon(QIcon(":/solarized_dark/solarized_dark/settings.png"));
    ui->actionSearch->setIcon(QIcon(":/solarized_dark/solarized_dark/search.png"));
    ui->actionMarkAll->setIcon(QIcon(":/solarized_dark/solarized_dark/mark-all.png"));
    ui->actionMarkNone->setIcon(QIcon(":/solarized_dark/solarized_dark/mark-none.png"));
    ui->actionMarkToggle->setIcon(QIcon(":/solarized_dark/solarized_dark/mark-toggle.png"));
    ui->actionAppendEntry->setIcon(QIcon(":/solarized_dark/solarized_dark/append_trans.png"));
    ui->actionStatement->setIcon(QIcon(":/solarized_dark/solarized_dark/statement.png"));
    ui->actionSettlement->setIcon(QIcon(":/solarized_dark/solarized_dark/settle.png"));
    ui->actionResetColor->setIcon(QIcon(":/solarized_dark/solarized_dark/reset_color.png"));
    ui->actionNewGroup->setIcon(QIcon(":/solarized_dark/solarized_dark/new-group.png"));
}

void MainWindow::SetTreeView(QTreeView* tree_view, CSectionInfo& info) const
{
    const auto section { info.section };

    if (section == Section::kSale || section == Section::kPurchase) {
        tree_view->setColumnHidden(std::to_underlying(NodeEnumO::kPartner), kIsHidden);
        tree_view->setColumnHidden(std::to_underlying(NodeEnumO::kSettlementId), kIsHidden);
    }

    tree_view->setColumnHidden(std::to_underlying(NodeEnum::kId), kIsHidden);
    tree_view->setColumnHidden(std::to_underlying(NodeEnum::kUserId), kIsHidden);
    tree_view->setColumnHidden(std::to_underlying(NodeEnum::kCreateBy), kIsHidden);
    tree_view->setColumnHidden(std::to_underlying(NodeEnum::kCreateTime), kIsHidden);
    tree_view->setColumnHidden(std::to_underlying(NodeEnum::kUpdateTime), kIsHidden);
    tree_view->setColumnHidden(std::to_underlying(NodeEnum::kUpdateBy), kIsHidden);

    tree_view->setSelectionMode(QAbstractItemView::SingleSelection);
    tree_view->setDragDropMode(QAbstractItemView::DragDrop);
    tree_view->setEditTriggers(QAbstractItemView::DoubleClicked);
    tree_view->setDropIndicatorShown(true);
    tree_view->setSortingEnabled(true);
    tree_view->setContextMenuPolicy(Qt::CustomContextMenu);
    tree_view->setExpandsOnDoubleClick(true);

    auto* header { tree_view->header() };
    ResizeColumn(header, NodeUtils::DescriptionColumn(section));
    header->setStretchLastSection(false);
    header->setDefaultAlignment(Qt::AlignCenter);
}

void MainWindow::on_actionInsertNode_triggered()
{
    assert(sc_->tree_widget);

    auto current_index { sc_->tree_view->currentIndex() };
    if (!current_index.isValid())
        return;

    auto parent_index { current_index.parent() };
    parent_index = parent_index.isValid() ? parent_index : QModelIndex();

    const QUuid parent_id { parent_index.isValid() ? parent_index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() : QUuid() };
    InsertNodeFunction(parent_index, parent_id, current_index.row() + 1);
}

void MainWindow::on_actionAppendNode_triggered()
{
    assert(sc_->tree_widget);

    auto parent_index { sc_->tree_view->currentIndex() };
    parent_index = parent_index.isValid() ? parent_index : QModelIndex();

    auto* parent_node { sc_->tree_model->GetNodeByIndex(parent_index) };
    if (parent_node->kind != std::to_underlying(NodeKind::kBranch))
        return;

    InsertNodeFunction(parent_index, parent_node->id, 0);
}

void MainWindow::on_actionJump_triggered()
{
    if (start_ == Section::kSale || start_ == Section::kPurchase || start_ == Section::kPartner)
        return;

    auto* leaf_widget { dynamic_cast<LeafWidget*>(ui->tabWidget->currentWidget()) };
    if (!leaf_widget)
        return;

    const auto index { leaf_widget->View()->currentIndex() };
    if (!index.isValid())
        return;

    const int row { index.row() };
    const int rhs_node_column { EntryUtils::LinkedNodeColumn(start_) };

    const auto rhs_node_id { index.sibling(row, rhs_node_column).data().toUuid() };
    if (rhs_node_id.isNull())
        return;

    const auto entry_id { index.sibling(row, std::to_underlying(EntryEnum::kId)).data().toUuid() };

    ShowLeafWidget(rhs_node_id, entry_id);
}

void MainWindow::RTreeViewCustomContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos);

    auto* menu = new QMenu(this);
    menu->addAction(ui->actionInsertNode);
    menu->addAction(ui->actionAppendNode);
    menu->addSeparator();
    menu->addAction(ui->actionEditName);
    menu->addAction(ui->actionResetColor);
    menu->addSeparator();
    menu->addAction(ui->actionRemove);

    menu->exec(QCursor::pos());
}

void MainWindow::on_actionEditName_triggered()
{
    if (start_ == Section::kSale || start_ == Section::kPurchase)
        return;

    assert(sc_->tree_widget);

    const auto index { sc_->tree_view->currentIndex() };
    if (!index.isValid())
        return;

    const auto node_id { index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() };

    auto model { sc_->tree_model };

    const auto& parent { index.parent() };
    const auto parent_id { parent.isValid() ? parent.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() : QUuid() };
    auto parent_path { model->Path(parent_id) };

    if (!parent_path.isEmpty())
        parent_path += app_config_.separator;

    CString name { model->Name(node_id) };
    const auto children_name { model->ChildrenName(parent_id) };

    auto* edit_name { new EditNodeName(name, parent_path, children_name, this) };
    connect(edit_name, &QDialog::accepted, this, [=]() { model->UpdateName(node_id, edit_name->GetName()); });
    edit_name->exec();
}

void MainWindow::InsertNodeFIPT(Node* node, const QModelIndex& parent, const QUuid& parent_id, int row)
{
    auto tree_model { sc_->tree_model };
    auto unit_model { sc_->info.unit_model };

    auto parent_path { tree_model->Path(parent_id) };
    if (!parent_path.isEmpty())
        parent_path += app_config_.separator;

    QDialog* dialog {};

    const auto children_name { tree_model->ChildrenName(parent_id) };
    const auto arg { InsertNodeArgFIPT { node, unit_model, parent_path, children_name } };

    switch (start_) {
    case Section::kFinance:
        dialog = new InsertNodeFinance(arg, this);
        break;
    case Section::kTask:
        dialog = new InsertNodeTask(arg, this);
        break;
    case Section::kPartner:
        dialog = new InsertNodeP(arg, this);
        break;
    case Section::kInventory:
        dialog = new InsertNodeI(arg, sc_->section_config.rate_decimal, this);
        break;
    default:
        return NodePool::Instance().Recycle(node, start_);
    }

    connect(dialog, &QDialog::accepted, this, [=, this]() {
        if (tree_model->InsertNode(row, parent, node)) {
            auto index { tree_model->index(row, 0, parent) };
            sc_->tree_view->setCurrentIndex(index);
        }
    });

    connect(dialog, &QDialog::rejected, this, [=, this]() { NodePool::Instance().Recycle(node, start_); });
    dialog->exec();
}

void MainWindow::RLeafExternalReference(const QUuid& node_id, int unit)
{
    assert(sc_->tree_widget);
    assert(sc_->tree_model->Kind(node_id) == std::to_underlying(NodeKind::kLeaf)
        && "Node kind should be 'kLeafNode' at this point. The kind check should be performed in the delegate DoubleSpinUnitRPS.");

    switch (start_) {
    case Section::kInventory:
        LeafExternalReferenceI(node_id, unit);
        break;
    case Section::kPartner:
        LeafExternalReferenceS(node_id, unit);
        break;
    default:
        break;
    }
}

void MainWindow::LeafExternalReferenceI(const QUuid& node_id, int unit)
{
    if (unit != std::to_underlying(UnitI::kInternal) || start_ != Section::kInventory)
        return;

    CreateLeafExternalReference(sc_->tree_model, sc_->info, node_id, unit);
}

void MainWindow::LeafExternalReferenceS(const QUuid& node_id, int unit)
{
    if (start_ != Section::kPartner)
        return;

    CreateLeafExternalReference(sc_->tree_model, sc_->info, node_id, unit);
}

void MainWindow::RUpdateName(const QUuid& node_id, const QString& name, bool branch)
{
    auto model { sc_->tree_model };
    auto* widget { ui->tabWidget };

    auto* tab_bar { widget->tabBar() };
    int count { widget->count() };

    QSet<QUuid> nodes;

    if (branch) {
        nodes = model->ChildrenId(node_id);
    } else {
        nodes.insert(node_id);

        if (start_ == Section::kPartner)
            UpdatePartnerReference(nodes, branch);

        if (!sc_->leaf_wgt_hash.contains(node_id))
            return;
    }

    for (int index = 0; index != count; ++index) {
        const auto node_id { tab_bar->tabData(index).value<TabInfo>().id };

        if (widget->isTabVisible(index) && nodes.contains(node_id)) {
            const auto path { model->Path(node_id) };

            if (!branch) {
                tab_bar->setTabText(index, name);
            }

            tab_bar->setTabToolTip(index, path);
        }
    }

    if (start_ == Section::kPartner)
        UpdatePartnerReference(nodes, branch);
}

void MainWindow::RUpdateConfig(const AppConfig& app, const SharedConfig& shared, const SectionConfig& section)
{
    UpdateAppConfig(app);
    UpdateSectionConfig(section);
    UpdateSharedConfig(shared);
}

void MainWindow::UpdateAppConfig(CAppConfig& app)
{
    if (app_config_ == app)
        return;

    auto new_separator { app.separator };
    auto old_separator { app_config_.separator };

    if (old_separator != new_separator) {
        sc_f_.tree_model->UpdateSeparator(old_separator, new_separator);
        sc_p_.tree_model->UpdateSeparator(old_separator, new_separator);
        sc_i_.tree_model->UpdateSeparator(old_separator, new_separator);
        sc_t_.tree_model->UpdateSeparator(old_separator, new_separator);

        auto* widget { ui->tabWidget };
        int count { ui->tabWidget->count() };

        for (int index = 0; index != count; ++index)
            widget->setTabToolTip(index, widget->tabToolTip(index).replace(old_separator, new_separator));
    }

    if (app_config_.language != app.language) {
        MainWindowUtils::Message(QMessageBox::Information, tr("Language Changed"),
            tr("The language has been changed. Please restart the application for the changes to take effect."), kThreeThousand);
    }

    app_config_ = app;

    app_settings_->beginGroup(kUi);
    app_settings_->setValue(kLanguage, app.language);
    app_settings_->setValue(kSeparator, app.separator);
    app_settings_->setValue(kTheme, app.theme);
    app_settings_->endGroup();

    app_settings_->beginGroup(kPrint);
    app_settings_->setValue(kPrinter, app.printer);
    app_settings_->endGroup();

    app_settings_->beginGroup(kExport);
    app_settings_->setValue(kCompanyName, app.company_name);
    app_settings_->endGroup();
}

void MainWindow::UpdateSectionConfig(CSectionConfig& section)
{
    auto& current_section { sc_->section_config };
    if (current_section == section)
        return;

    const bool resize_column { current_section.amount_decimal != section.amount_decimal || current_section.rate_decimal != section.rate_decimal
        || current_section.date_format != section.date_format };

    current_section = section;
    sc_->tree_widget->UpdateStatus();

    const static QMap<Section, QString> section_string_map {
        { Section::kFinance, kFinance },
        { Section::kPartner, kPartner },
        { Section::kInventory, kInventory },
        { Section::kTask, kTask },
        { Section::kSale, kSale },
        { Section::kPurchase, kPurchase },
    };

    const QString text { section_string_map.value(start_) };
    section_settings_->beginGroup(text);

    if (start_ == Section::kFinance || start_ == Section::kInventory) {
        section_settings_->setValue(kStaticLabel, section.static_label);
        section_settings_->setValue(kStaticNode, section.static_node);
        section_settings_->setValue(kDynamicLabel, section.dynamic_label);
        section_settings_->setValue(kDynamicNodeLhs, section.dynamic_node_lhs);
        section_settings_->setValue(kOperation, section.operation);
        section_settings_->setValue(kDynamicNodeRhs, section.dynamic_node_rhs);
    }

    section_settings_->setValue(kDateFormat, section.date_format);
    section_settings_->setValue(kAmountDecimal, section.amount_decimal);
    section_settings_->setValue(kRateDecimal, section.rate_decimal);

    section_settings_->endGroup();

    if (resize_column) {
        auto* current_widget { ui->tabWidget->currentWidget() };

        if (const auto* leaf_widget = dynamic_cast<LeafWidget*>(current_widget)) {
            auto* header { leaf_widget->View()->horizontalHeader() };

            int column { std::to_underlying(EntryEnum::kDescription) };
            ResizeColumn(header, column);
            return;
        }

        if (dynamic_cast<TreeWidget*>(current_widget)) {
            auto* header { sc_->tree_view->header() };
            ResizeColumn(header, NodeUtils::DescriptionColumn(start_));
        }
    }
}

void MainWindow::UpdateSharedConfig(CSharedConfig& shared)
{
    auto& current_shared { sc_->shared_config };
    if (current_shared == shared)
        return;

    if (current_shared.document_dir != shared.document_dir) {
        const auto message { JsonGen::DocumentDir(sc_->info.section, shared.document_dir) };
        WebSocket::Instance()->SendMessage(kDocumentDir, message);
        current_shared.document_dir = shared.document_dir;
    }

    if (current_shared.default_unit != shared.default_unit) {
        const auto message { JsonGen::DefaultUnit(sc_->info.section, shared.default_unit) };
        WebSocket::Instance()->SendMessage(kDefaultUnit, message);
    }
}

void MainWindow::UpdateAccountInfo(const QString& user, const QString& database, const QString& expire_date)
{
    ui->actionEmail->setText(tr("Email") + ": " + user);
    ui->actionWorkspace->setText(tr("Workspace") + ": " + database);
    ui->actionExpireDate->setText(tr("Expire Date") + ": " + expire_date);
}

void MainWindow::ClearAccountInfo()
{
    ui->actionEmail->setText(tr("Email"));
    ui->actionWorkspace->setText(tr("Workspace"));
    ui->actionExpireDate->setText(tr("Expire Date"));
}

void MainWindow::UpdatePartnerReference(const QSet<QUuid>& partner_nodes, bool branch) const
{
    auto* widget { ui->tabWidget };
    auto partner_model { sc_->tree_model };
    auto* order_model { static_cast<TreeModelO*>(sc_sale_.tree_model.data()) };
    auto* tab_bar { widget->tabBar() };
    const int count { widget->count() };

    // 使用 QtConcurrent::run 启动后台线程
    auto future = QtConcurrent::run([=]() -> QVector<std::tuple<int, QString, QString>> {
        QVector<std::tuple<int, QString, QString>> updates;

        // 遍历所有选项卡，计算需要更新的项
        for (int index = 0; index != count; ++index) {
            const auto& data { tab_bar->tabData(index).value<TabInfo>() };
            bool update { data.section == Section::kSale || data.section == Section::kPurchase };

            if (!widget->isTabVisible(index) && update) {
                const auto order_node_id { data.id };
                if (order_node_id.isNull())
                    continue;

                const auto order_partner { order_model->Partner(order_node_id) };
                if (!partner_nodes.contains(order_partner))
                    continue;

                QString name { partner_model->Name(order_partner) };
                QString path { partner_model->Path(order_partner) };

                // 收集需要更新的信息
                updates.append(std::make_tuple(index, name, path));
            }
        }

        return updates;
    });

    auto watcher { std::make_unique<QFutureWatcher<QVector<std::tuple<int, QString, QString>>>>() };

    // 获取原始指针用于信号连接
    auto* raw_watcher = watcher.get();

    connect(
        raw_watcher, &QFutureWatcher<QVector<std::tuple<int, QString, QString>>>::finished, this, [watcher = std::move(watcher), tab_bar, branch]() mutable {
            const auto& updates = watcher->result();

            for (const auto& [index, name, path] : updates) {
                if (!branch)
                    tab_bar->setTabText(index, name);
                tab_bar->setTabToolTip(index, path);
            }
        });

    raw_watcher->setFuture(future);
}

void MainWindow::ResizeColumn(QHeaderView* header, int stretch_column) const
{
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(stretch_column, QHeaderView::Stretch);
}

void MainWindow::ReadLocalConfig()
{
    app_settings_ = QSharedPointer<QSettings>::create(
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() + kYTX + kDotSuffixINI, QSettings::IniFormat);

    app_settings_->beginGroup(kUi);
    app_config_.language = app_settings_->value(kLanguage, QLocale::system().name()).toString();
    app_config_.theme = app_settings_->value(kTheme, kSolarizedDark).toString();
    app_config_.separator = app_settings_->value(kSeparator, kDash).toString();
    app_settings_->endGroup();

    app_settings_->beginGroup(kPrint);
    app_config_.printer = app_settings_->value(kPrinter).toString();
    app_settings_->endGroup();

    app_settings_->beginGroup(kExport);
    app_config_.company_name = app_settings_->value(kCompanyName).toString();
    app_settings_->endGroup();

    LoginInfo::Instance().ReadConfig(app_settings_);
    WebSocket::Instance()->ReadConfig(app_settings_);
    WebSocket::Instance()->Connect();

    LoadAndInstallTranslator(app_config_.language);

    const QString theme { QStringLiteral("file:///:/theme/theme/%1.qss").arg(app_config_.theme) };
    qApp->setStyleSheet(theme);
}

void MainWindow::LoadAndInstallTranslator(CString& language)
{
    static const QSet<QString> supported_languages { kZhCN, kEnUS };

    const QString lang { supported_languages.contains(language) ? language : kEnUS };

    QLocale locale(lang);
    QLocale::setDefault(locale);

    if (lang.startsWith("en", Qt::CaseInsensitive))
        return;

    const QString ytx_language { QStringLiteral(":/I18N/I18N/ytx_%1.qm").arg(lang) };
    if (ytx_translator_.load(ytx_language))
        qApp->installTranslator(&ytx_translator_);

    const QString qt_language { QStringLiteral(":/I18N/I18N/qt_%1.qm").arg(lang) };
    if (qt_translator_.load(qt_language))
        qApp->installTranslator(&qt_translator_);
}

void MainWindow::ReadSectionConfig(SectionConfig& section, CString& section_name)
{
    section_settings_->beginGroup(section_name);

    if (section_name == kFinance || section_name == kInventory) {
        section.static_label = section_settings_->value(kStaticLabel, {}).toString();
        section.static_node = section_settings_->value(kStaticNode, QUuid()).toUuid();
        section.dynamic_label = section_settings_->value(kDynamicLabel, {}).toString();
        section.dynamic_node_lhs = section_settings_->value(kDynamicNodeLhs, QUuid()).toUuid();
        section.operation = section_settings_->value(kOperation, kPlus).toString();
        section.dynamic_node_rhs = section_settings_->value(kDynamicNodeRhs, QUuid()).toUuid();
    }

    section.date_format = section_settings_->value(kDateFormat, kDateTimeFST).toString();
    section.amount_decimal = section_settings_->value(kAmountDecimal, 2).toInt();
    section.rate_decimal = section_settings_->value(kRateDecimal, 2).toInt();

    section_settings_->endGroup();
}

void MainWindow::on_actionSearch_triggered()
{
    SearchNodeModel* node {};
    SearchEntryModel* entry {};
    SearchDialog* dialog {};

    switch (start_) {
    case Section::kFinance:
        node = new SearchNodeModelF(sc_->info, sc_->tree_model, this);
        entry = new SearchEntryModelF(sc_->info, this);
        dialog = new SearchDialogF(sc_->tree_model, node, entry, sc_->section_config, sc_->info, this);
        break;
    case Section::kInventory:
        node = new SearchNodeModelI(sc_->info, sc_->tree_model, this);
        entry = new SearchEntryModelI(sc_->info, this);
        dialog = new SearchDialogI(sc_->tree_model, node, entry, sc_->section_config, sc_->info, this);
        break;
    case Section::kTask:
        node = new SearchNodeModelT(sc_->info, sc_->tree_model, this);
        entry = new SearchEntryModelT(sc_->info, this);
        dialog = new SearchDialogT(sc_->tree_model, node, entry, sc_->section_config, sc_->info, this);
        break;
    case Section::kPartner:
        node = new SearchNodeModelP(sc_->info, sc_->tree_model, this);
        entry = new SearchEntryModelP(sc_->info, this);
        dialog = new SearchDialogP(sc_->tree_model, node, entry, sc_i_.tree_model, sc_->section_config, sc_->info, this);
        break;
    case Section::kSale:
    case Section::kPurchase:
        node = new SearchNodeModelO(sc_->info, sc_->tree_model, sc_p_.tree_model, this);
        entry = new SearchEntryModelO(sc_->info, this);
        dialog = new SearchDialogO(sc_->tree_model, node, entry, sc_i_.tree_model, sc_p_.tree_model, sc_->section_config, sc_->info, this);
        break;
    default:
        break;
    }

    dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);

    connect(dialog, &SearchDialog::SNodeLocation, this, &MainWindow::RNodeLocation);
    connect(dialog, &SearchDialog::SEntryLocation, this, &MainWindow::REntryLocation);
    connect(sc_->entry_hub, &EntryHub::SSearchEntry, entry, &SearchEntryModel::RSearchEntry);
    connect(dialog, &QDialog::rejected, this, [=, this]() { sc_->dialog_list.removeOne(dialog); });

    sc_->dialog_list.append(dialog);
    dialog->show();
}

void MainWindow::RNodeLocation(const QUuid& node_id)
{
    // Ignore report widget
    if (node_id.isNull())
        return;

    auto widget { sc_->tree_widget };
    ui->tabWidget->setCurrentWidget(widget);

    if (start_ == Section::kSale || start_ == Section::kPurchase || start_ == Section::kTask) {
        sc_->tree_model->FetchOneNode(node_id);
    }

    auto index { sc_->tree_model->GetIndex(node_id) };
    widget->activateWindow();
    widget->View()->setCurrentIndex(index);
}

void MainWindow::REntryLocation(const QUuid& entry_id, const QUuid& lhs_node_id, const QUuid& rhs_node_id)
{
    QUuid id { lhs_node_id };

    auto Contains = [&](QUuid node_id) {
        if (sc_->leaf_wgt_hash.contains(node_id)) {
            id = node_id;
            return true;
        }
        return false;
    };

    if (!Contains(lhs_node_id) && !Contains(rhs_node_id)) {
        const auto message { JsonGen::LeafAcked(sc_->info.section, id, entry_id) };
        WebSocket::Instance()->SendMessage(kLeafAcked, message);

        switch (start_) {
        case Section::kSale:
        case Section::kPurchase:
            sc_->tree_model->FetchOneNode(lhs_node_id);
            CreateLeafO(sc_, lhs_node_id);
            break;
        case Section::kTask:
            sc_->tree_model->FetchOneNode(lhs_node_id);
        case Section::kFinance:
        case Section::kInventory:
        case Section::kPartner:
            CreateLeafFIPT(sc_, id);
            break;
        default:
            break;
        }
    }

    ActivateLeafTab(id);
}

void MainWindow::OrderNodeLocation(Section section, const QUuid& node_id)
{
    switch (section) {
    case Section::kSale:
        RSectionGroup(std::to_underlying(Section::kSale));
        ui->rBtnSale->setChecked(true);
        break;
    case Section::kPurchase:
        RSectionGroup(std::to_underlying(Section::kPurchase));
        ui->rBtnPurchase->setChecked(true);
        break;
    default:
        return;
    }

    ui->tabWidget->setCurrentWidget(sc_->tree_widget);

    sc_->tree_model->FetchOneNode(node_id);
    sc_->tree_widget->activateWindow();

    auto index { sc_->tree_model->GetIndex(node_id) };
    sc_->tree_view->setCurrentIndex(index);
}

void MainWindow::RSyncPartner(const QUuid& node_id, int column, const QVariant& value)
{
    if (column != std::to_underlying(NodeEnumO::kPartner))
        return;

    const auto partner_id { value.toUuid() };

    auto model { sc_p_.tree_model };
    auto* widget { ui->tabWidget };
    auto* tab_bar { widget->tabBar() };
    int count { widget->count() };

    for (int index = 0; index != count; ++index) {
        if (widget->isTabVisible(index) && tab_bar->tabData(index).value<TabInfo>().id == node_id) {
            tab_bar->setTabText(index, model->Name(partner_id));
            tab_bar->setTabToolTip(index, model->Path(partner_id));
        }
    }
}

void MainWindow::on_actionPreferences_triggered()
{
    auto model { sc_->tree_model };

    auto* preference { new Preferences(model, sc_->info, app_config_, sc_->shared_config, sc_->section_config, this) };
    connect(preference, &Preferences::SUpdateConfig, this, &MainWindow::RUpdateConfig);
    preference->exec();
}

void MainWindow::on_actionAbout_triggered()
{
    static About* dialog = nullptr;

    if (!dialog) {
        dialog = new About(this);
        dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(dialog, &QDialog::destroyed, this, [=]() { dialog = nullptr; });
    }

    dialog->show();
    dialog->activateWindow();
}

void MainWindow::on_tabWidget_tabBarDoubleClicked(int index) { RNodeLocation(ui->tabWidget->tabBar()->tabData(index).value<TabInfo>().id); }

void MainWindow::RActionEntry(EntryAction action)
{
    auto* leaf_widget { dynamic_cast<LeafWidget*>(ui->tabWidget->currentWidget()) };
    assert(leaf_widget);

    auto table_model { leaf_widget->Model() };
    table_model->ActionEntry(action);
}

void MainWindow::RConnectResult(bool result)
{
    ui->actionReconnect->setEnabled(!result);
    ui->actionLogin->setEnabled(result);
    ui->actionLogout->setEnabled(false);
    ui->actionRegister->setEnabled(result);

    if (result) {
        on_actionLogin_triggered();
    } else {
        QMessageBox::warning(this, tr("Connection Failed"), tr("Unable to connect to the server. Please check your network or contact the administrator."));
    }
}

void MainWindow::RLoginResult(bool result)
{
    ui->actionLogin->setEnabled(!result);
    ui->actionLogout->setEnabled(result);
}

void MainWindow::SwitchSection(Section section, const QUuid& last_tab) const
{
    auto* tab_widget { ui->tabWidget };
    auto* tab_bar { tab_widget->tabBar() };
    const int count { tab_widget->count() };

    for (int index = 0; index != count; ++index) {
        const auto tab { tab_bar->tabData(index).value<TabInfo>() };
        tab_widget->setTabVisible(index, tab.section == section);

        if (!last_tab.isNull() && tab.id == last_tab)
            tab_widget->setCurrentIndex(index);
    }

    MainWindowUtils::SwitchDialog(sc_, true);
}

void MainWindow::UpdateLastTab() const
{
    if (!sc_)
        return;

    auto index { ui->tabWidget->currentIndex() };
    sc_->info.last_tab_id = ui->tabWidget->tabBar()->tabData(index).value<TabInfo>().id;
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    auto* widget { ui->tabWidget->currentWidget() };
    assert(widget);

    const bool is_tree { IsTreeWidget(widget) };
    const bool is_leaf_fist { IsLeafWidgetFIPT(widget) };
    const bool is_leaf_order { IsLeafWidgetO(widget) };
    const bool is_order_section { start_ == Section::kSale || start_ == Section::kPurchase };
    const bool is_color_section { start_ == Section::kTask || start_ == Section::kInventory };

    bool finished {};

    if (is_leaf_order) {
        const auto node_id { ui->tabWidget->tabBar()->tabData(index).value<TabInfo>().id };
        finished = sc_->tree_model->Status(node_id);
    }

    ui->actionAppendNode->setEnabled(is_tree);
    ui->actionEditName->setEnabled(is_tree && !is_order_section);
    ui->actionResetColor->setEnabled(is_tree && is_color_section);

    ui->actionMarkAll->setEnabled(is_leaf_fist);
    ui->actionMarkNone->setEnabled(is_leaf_fist);
    ui->actionMarkToggle->setEnabled(is_leaf_fist);
    ui->actionJump->setEnabled(is_leaf_fist);

    ui->actionStatement->setEnabled(is_order_section);
    ui->actionSettlement->setEnabled(is_order_section);
    ui->actionNewGroup->setEnabled(is_order_section);

    ui->actionAppendEntry->setEnabled(is_leaf_fist || (is_leaf_order && !finished));
    ui->actionRemove->setEnabled(is_tree || is_leaf_fist || (is_leaf_order && !finished));
}

void MainWindow::on_actionAppendEntry_triggered()
{
    auto* widget { ui->tabWidget->currentWidget() };
    assert(widget);

    if (auto* leaf_widget = dynamic_cast<LeafWidget*>(widget)) {
        WidgetUtils::AppendEntryFIST(leaf_widget, start_);
    }
}

void MainWindow::on_actionExportExcel_triggered()
{
    CString& source {};

    QString destination { QFileDialog::getSaveFileName(this, tr("Export Excel"), QDir::homePath(), QStringLiteral("*.xlsx")) };
    if (!MainWindowUtils::PrepareNewFile(destination, kDotSuffixXLSX))
        return;

    auto future = QtConcurrent::run([source, destination, this]() {
        // QSqlDatabase source_db;
        // if (!PublicUtils::AddDatabase(source_db, source, kSourceConnection))
        //     return false;

        try {
            const QStringList list { tr("Ancestor"), tr("Descendant"), tr("Distance") };

            YXlsx::Document d(destination);

            auto book1 { d.GetWorkbook() };
            book1->AppendSheet(sc_->info.node);
            book1->GetCurrentWorksheet()->WriteRow(1, 1, sc_->info.node_header);
            MainWindowUtils::ExportExcel(sc_->info.node, book1->GetCurrentWorksheet());

            book1->AppendSheet(sc_->info.path);
            book1->GetCurrentWorksheet()->WriteRow(1, 1, list);
            MainWindowUtils::ExportExcel(sc_->info.path, book1->GetCurrentWorksheet(), false);

            book1->AppendSheet(sc_->info.entry);
            book1->GetCurrentWorksheet()->WriteRow(1, 1, sc_->info.full_entry_header);
            const bool where { start_ == Section::kPartner ? false : true };
            MainWindowUtils::ExportExcel(sc_->info.entry, book1->GetCurrentWorksheet(), where);

            d.Save();
            // PublicUtils::RemoveDatabase(kSourceConnection);
            return true;
        } catch (...) {
            qWarning() << "Export failed due to an unknown exception.";
            // PublicUtils::RemoveDatabase(kSourceConnection);
            return false;
        }
    });

    auto* watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [watcher, destination]() {
        watcher->deleteLater();

        bool success { watcher->future().result() };
        if (success) {
            MainWindowUtils::Message(QMessageBox::Information, tr("Export Completed"), tr("Export completed successfully."), kThreeThousand);
        } else {
            QFile::remove(destination);
            MainWindowUtils::Message(QMessageBox::Critical, tr("Export Failed"), tr("Export failed. The file has been deleted."), kThreeThousand);
        }
    });

    watcher->setFuture(future);
}

void MainWindow::on_actionLogin_triggered()
{
    static Login* dialog = nullptr;

    if (!dialog) {
        dialog = new Login(app_settings_, this);
        dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(dialog, &QDialog::destroyed, this, [=]() { dialog = nullptr; });
    }

    dialog->setModal(true);
    dialog->show();
    dialog->activateWindow();
}

void MainWindow::on_actionResetColor_triggered()
{
    if (start_ != Section::kInventory && start_ != Section::kTask)
        return;

    assert(sc_->tree_widget);

    const auto index { sc_->tree_view->currentIndex() };
    if (!index.isValid())
        return;

    auto model { sc_->tree_model };
    model->ResetColor(index);
}

void MainWindow::on_actionRegister_triggered()
{
    auto* regist { new RegisterDialog(this) };
    regist->exec();
}

void MainWindow::on_actionReconnect_triggered() { WebSocket::Instance()->Connect(); }

void MainWindow::on_actionLogout_triggered()
{
    ClearMainwindow();
    ClearAccountInfo();

    WebSocket::Instance()->Clear();
    LeafSStation::Instance()->Clear();

    SetAction(false);

    ui->actionReconnect->setEnabled(true);
    ui->actionLogin->setEnabled(false);
    ui->actionLogout->setEnabled(false);
    ui->actionRegister->setEnabled(false);
}

void MainWindow::on_actionCheckforUpdates_triggered()
{
    const QString url { "https://api.github.com/repos/ytxerp/ytx-client/releases/latest" };

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "YTX-Updater");

    QNetworkReply* reply = network_manager_->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::warning(this, tr("Update Check"), tr("Failed to check updates:\n%1").arg(reply->errorString()));
            return;
        }

        const QByteArray data { reply->readAll() };
        QJsonDocument doc { QJsonDocument::fromJson(data) };
        if (!doc.isObject()) {
            QMessageBox::warning(this, tr("Update Check"), tr("Invalid update information received."));
            return;
        }

        const QJsonObject obj { doc.object() };
        const QString latest_tag { obj.value("tag_name").toString() };

        const bool is_chinese { app_config_.language.startsWith("zh", Qt::CaseInsensitive) };
        const QString download_url
            = is_chinese ? "https://gitee.com/ytxerp/ytx-client/releases/latest" : "https://github.com/ytxerp/ytx-client/releases/latest";

        if (MainWindowUtils::CompareVersion(latest_tag, QCoreApplication::applicationVersion()) > 0) {
            QMessageBox::StandardButton btn = QMessageBox::information(
                this, tr("Update Available"), tr("A new version %1 is available!\n\nDownload now?").arg(latest_tag), QMessageBox::Yes | QMessageBox::No);

            if (btn == QMessageBox::Yes) {
                QDesktopServices::openUrl(QUrl(download_url));
            }
        } else {
            QMessageBox::information(this, tr("No Update"), tr("You are using the latest version."));
        }
    });
}
