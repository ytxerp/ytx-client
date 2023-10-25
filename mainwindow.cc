#include "mainwindow.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QHeaderView>
#include <QMessageBox>
#include <QNetworkReply>
#include <QQueue>
#include <QResource>
#include <QScrollBar>
#include <QtConcurrent>

#include "component/arg/insertnodeargfist.h"
#include "component/constant.h"
#include "component/enumclass.h"
#include "component/signalblocker.h"
#include "component/stringinitializer.h"
#include "delegate/checkbox.h"
#include "delegate/dboolstring.h"
#include "delegate/document.h"
#include "delegate/double.h"
#include "delegate/doubleguard.h"
#include "delegate/filterunit.h"
#include "delegate/int.h"
#include "delegate/lineguard.h"
#include "delegate/plaintextguard.h"
#include "delegate/readonly/colorr.h"
#include "delegate/readonly/dintstringr.h"
#include "delegate/readonly/doublespinr.h"
#include "delegate/readonly/doublespinrnonezero.h"
#include "delegate/readonly/doublespinunitr.h"
#include "delegate/readonly/issuedtimer.h"
#include "delegate/readonly/nodenamer.h"
#include "delegate/readonly/nodepathr.h"
#include "delegate/readonly/sectionr.h"
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
#include "dialog/insertnode/insertnodeitem.h"
#include "dialog/insertnode/insertnodestakeholder.h"
#include "dialog/insertnode/insertnodetask.h"
#include "dialog/login.h"
#include "dialog/preferences.h"
#include "dialog/registerdialog.h"
#include "dialog/removenode/removeleafnodedialog.h"
#include "document.h"
#include "entryhub/entryhubf.h"
#include "entryhub/entryhubi.h"
#include "entryhub/entryhubo.h"
#include "entryhub/entryhubs.h"
#include "entryhub/entryhubt.h"
#include "global/leafsstation.h"
#include "global/logininfo.h"
#include "global/nodepool.h"
#include "global/websocket.h"
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
#include "search/dialog/searchdialogs.h"
#include "search/dialog/searchdialogt.h"
#include "search/entry/searchentrymodelf.h"
#include "search/entry/searchentrymodeli.h"
#include "search/entry/searchentrymodelo.h"
#include "search/entry/searchentrymodels.h"
#include "search/entry/searchentrymodelt.h"
#include "search/node/searchnodemodelf.h"
#include "search/node/searchnodemodeli.h"
#include "search/node/searchnodemodelo.h"
#include "search/node/searchnodemodels.h"
#include "search/node/searchnodemodelt.h"
#include "table/model/leafmodelf.h"
#include "table/model/leafmodeli.h"
#include "table/model/leafmodels.h"
#include "table/model/leafmodelt.h"
#include "tree/model/treemodelf.h"
#include "tree/model/treemodeli.h"
#include "tree/model/treemodels.h"
#include "tree/model/treemodelt.h"
#include "tree/widget/treewidgetf.h"
#include "tree/widget/treewidgeti.h"
#include "tree/widget/treewidgets.h"
#include "tree/widget/treewidgetto.h"
#include "ui_mainwindow.h"
#include "utils/jsongen.h"
#include "utils/mainwindowutils.h"
#include "utils/widgetutils.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QResource::registerResource(MainWindowUtils::ResourceFile());
    ReadLocalConfig();

    ui->setupUi(this);
    SignalBlocker blocker(this);

    SetTabWidget();
    IniSectionGroup();
    StringInitializer::SetHeader(sc_f_.info, sc_i_.info, sc_t_.info, sc_s_.info, sc_sale_.info, sc_purchase_.info);
    SetAction();
    SetUniqueConnection();

    WidgetUtils::ReadConfig(ui->splitter, &QSplitter::restoreState, local_settings_, kSplitter, kState);
    WidgetUtils::ReadConfig(this, &QMainWindow::restoreState, local_settings_, kMainwindow, kState, 0);
    WidgetUtils::ReadConfig(this, &QMainWindow::restoreGeometry, local_settings_, kMainwindow, kGeometry);

    EnableAction(false);
    InitSystemTray();

#ifdef Q_OS_WIN
    ui->actionRemove->setShortcut(Qt::Key_Delete);
    qApp->setWindowIcon(QIcon(":/logo/logo/logo.ico"));
#elif defined(Q_OS_MACOS)
    ui->actionRemove->setShortcut(Qt::Key_Backspace);
    qApp->setWindowIcon(QIcon(":/logo/logo/logo.icns"));
#endif
}

MainWindow::~MainWindow()
{
    WriteConfig();
    delete ui;
}

bool MainWindow::RInitializeContext()
{
    LoginInfo& login_info { LoginInfo::Instance() };
    UpdateAccountInfo(login_info.Email(), login_info.Workspace());

    this->setWindowTitle(local_config_.company_name);

    if (!section_settings_)
        section_settings_ = QSharedPointer<QSettings>::create(
            QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() + login_info.Workspace() + kDotSuffixINI,
            QSettings::IniFormat);

    InitContextFinance();
    InitContextTask();
    InitContextItem();
    InitContextStakeholder();
    InitContextSale();
    InitContextPurchase();

    CreateSection(sc_f_, tr("Finance"));
    CreateSection(sc_s_, tr("Stakeholder"));
    CreateSection(sc_i_, tr("Item"));
    CreateSection(sc_t_, tr("Task"));
    CreateSection(sc_sale_, tr("Sale"));
    CreateSection(sc_purchase_, tr("Purchase"));

    RSectionGroup(static_cast<int>(start_));

    EnableAction(true);
    on_tabWidget_currentChanged(0);

    QTimer::singleShot(0, this, [this]() { MainWindowUtils::ReadPrintTmplate(print_template_); });
    return true;
}

void MainWindow::on_actionInsertNode_triggered()
{
    assert(sc_->tree_widget);

    auto current_index { sc_->tree_view->currentIndex() };
    current_index = current_index.isValid() ? current_index : QModelIndex();

    auto parent_index { current_index.parent() };
    parent_index = parent_index.isValid() ? parent_index : QModelIndex();

    const QUuid parent_id { parent_index.isValid() ? parent_index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() : QUuid() };
    InsertNodeFunction(parent_index, parent_id, current_index.row() + 1);
}

void MainWindow::RTreeViewDoubleClicked(const QModelIndex& index)
{
    if (index.column() != 0)
        return;

    const int kind_column { NodeUtils::KindColumn(start_) };
    const int kind { index.siblingAtColumn(kind_column).data().toInt() };
    if (kind == kBranch)
        return;

    const int unit_column { NodeUtils::UnitColumn(start_) };
    const int unit { index.siblingAtColumn(unit_column).data().toInt() };
    if (start_ == Section::kItem && unit == std::to_underlying(UnitI::kExternal))
        return;

    const auto node_id { index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() };
    assert(!node_id.isNull());

    CreateLeafWidget(node_id);
    SwitchToLeaf(node_id);
}

void MainWindow::CreateLeafWidget(const QUuid& node_id)
{
    auto& leaf_wgt_hash { sc_->leaf_wgt_hash };

    if (sc_->leaf_wgt_hash.contains(node_id))
        return;

    const auto message { JsonGen::TableData(sc_->info.section_str, node_id) };
    WebSocket::Instance().SendMessage(kLeafAcked, message);

    if (start_ == Section::kSale || start_ == Section::kPurchase) {
        CreateLeafO(sc_->tree_model, leaf_wgt_hash, sc_->info, sc_->section_config, node_id);
    } else {
        CreateLeafFIST(sc_->tree_model, sc_->entry_hub, leaf_wgt_hash, sc_->info, sc_->section_config, node_id);
    }
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
    case Section::kItem:
        sc_ = &sc_i_;
        break;
    case Section::kTask:
        sc_ = &sc_t_;
        break;
    case Section::kStakeholder:
        sc_ = &sc_s_;
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

void MainWindow::RStatementPrimary(const QUuid& party_id, int unit, const QDateTime& start, const QDateTime& end)
{
    auto* model { new StatementPrimaryModel(sc_->entry_hub, sc_->info, party_id, this) };
    auto* widget { new StatementWidget(model, unit, false, start, end, this) };

    const QString name { tr("StatementPrimary-") + sc_s_.tree_model->Name(party_id) };
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

void MainWindow::RStatementSecondary(const QUuid& party_id, int unit, const QDateTime& start, const QDateTime& end)
{
    auto tree_model_s { sc_s_.tree_model };

    auto* model { new StatementSecondaryModel(
        sc_->entry_hub, sc_->info, party_id, sc_i_.tree_model->LeafPath(), tree_model_s, local_config_.company_name, this) };
    auto* widget { new StatementWidget(model, unit, true, start, end, this) };

    const QString name { tr("StatementSecondary-") + tree_model_s->Name(party_id) };
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

void MainWindow::SwitchToLeaf(const QUuid& node_id, const QUuid& entry_id) const
{
    auto widget { sc_->leaf_wgt_hash.value(node_id, nullptr) };
    assert(widget);

    ui->tabWidget->setCurrentWidget(widget);
    widget->activateWindow();

    auto* view { widget->View() };
    view->setCurrentIndex(QModelIndex());

    if (entry_id.isNull())
        return;

    auto index { widget->Model()->GetIndex(entry_id) };

    if (!index.isValid())
        return;

    view->setCurrentIndex(index);
    view->scrollTo(index.siblingAtColumn(std::to_underlying(EntryEnum::kIssuedTime)), QAbstractItemView::PositionAtCenter);
    view->closePersistentEditor(index);
}

void MainWindow::CreateLeafFIST(TreeModel* tree_model, EntryHub* hub, LeafWgtHash& wgt_hash, CSectionInfo& info, CSectionConfig& config, CUuid& node_id)
{
    assert(tree_model);
    assert(tree_model->Contains(node_id));

    CString name { tree_model->Name(node_id) };
    const Section section { info.section };
    const bool rule { tree_model->Rule(node_id) };

    LeafModel* entry_model {};
    LeafModelArg arg { hub, info, node_id, rule };

    switch (section) {
    case Section::kFinance:
        entry_model = new LeafModelF(arg, this);
        break;
    case Section::kItem:
        entry_model = new LeafModelI(arg, this);
        break;
    case Section::kTask:
        entry_model = new LeafModelT(arg, this);
        break;
    case Section::kStakeholder:
        entry_model = new LeafModelS(arg, this);
        break;
    default:
        break;
    }

    LeafWidgetFIST* widget { new LeafWidgetFIST(entry_model, this) };

    const int tab_index { ui->tabWidget->addTab(widget, name) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { section, node_id }));
    tab_bar->setTabToolTip(tab_index, tree_model->Path(node_id));

    auto* view { widget->View() };

    SetTableView(view, std::to_underlying(EntryEnum::kDescription), std::to_underlying(EntryEnum::kLhsNode));

    switch (section) {
    case Section::kFinance:
        TableDelegateF(view, tree_model, config, node_id);
        TableConnectF(view, entry_model, tree_model);
        break;
    case Section::kItem:
        TableDelegateI(view, tree_model, config, node_id);
        TableConnectI(view, entry_model, tree_model);
        break;
    case Section::kTask:
        TableDelegateT(view, tree_model, config, node_id);
        TableConnectT(view, entry_model, tree_model);
        break;
    case Section::kStakeholder:
        TableDelegateS(view, config);
        TableConnectS(view, entry_model);
        break;
    default:
        break;
    }

    wgt_hash.insert(node_id, widget);
    LeafSStation::Instance().RegisterModel(node_id, entry_model);
}

void MainWindow::InsertNodeO(Node* node, const QModelIndex& parent, int row)
{
    auto* tree_model_order { static_cast<TreeModelO*>(sc_->tree_model.data()) };
    const QUuid node_id { node->id };

    LeafModelArg model_arg { sc_->entry_hub, sc_->info, node_id, true };
    auto* leaf_model_order { new LeafModelO(model_arg, node, sc_i_.tree_model, sc_s_.entry_hub, this) };

    auto print_manager = QSharedPointer<PrintManager>::create(local_config_, sc_i_.tree_model, sc_s_.tree_model);

    auto widget_arg { InsertNodeArgO { node, sc_->entry_hub, leaf_model_order, sc_s_.tree_model, sc_->section_config, start_ } };
    auto* widget { new LeafWidgetO(widget_arg, true, print_template_, print_manager, this) };
    auto* view { widget->View() };

    TableConnectO(view, leaf_model_order, tree_model_order, widget);

    connect(widget, &LeafWidgetO::SSaveOrder, this, [=, this]() {
        if (tree_model_order->InsertNode(row, parent, node)) {
            auto index = tree_model_order->index(row, 0, parent);
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
    SwitchToLeaf(node_id);
}

void MainWindow::CreateLeafO(TreeModel* tree_model, LeafWgtHash& wgt_hash, CSectionInfo& info, CSectionConfig& config, const QUuid& node_id)
{
    const Section section { info.section };

    auto* tree_model_o { static_cast<TreeModelO*>(tree_model) };
    if (!tree_model_o)
        return;

    NodeO* node { static_cast<NodeO*>(tree_model_o->GetNode(node_id)) };
    if (!node)
        return;

    const auto party_id { node->party };

    assert(!party_id.isNull());
    auto tree_model_s { sc_s_.tree_model };
    auto tree_model_i { sc_i_.tree_model };

    LeafModelArg model_arg { sc_->entry_hub, info, node_id, node->direction_rule };
    LeafModelO* model { new LeafModelO(model_arg, node, tree_model_i, sc_s_.entry_hub, this) };

    auto print_manager = QSharedPointer<PrintManager>::create(local_config_, tree_model_i, tree_model_s);

    auto widget_arg { InsertNodeArgO { node, sc_->entry_hub, model, tree_model_s, sc_->section_config, section } };
    LeafWidgetO* widget { new LeafWidgetO(widget_arg, false, print_template_, print_manager, this) };

    const int tab_index { ui->tabWidget->addTab(widget, tree_model_s->Name(party_id)) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { section, node_id }));
    tab_bar->setTabToolTip(tab_index, tree_model_s->Path(party_id));

    auto* view { widget->View() };
    SetTableView(view, std::to_underlying(EntryEnumO::kDescription), std::to_underlying(EntryEnumO::kLhsNode));

    TableConnectO(view, model, tree_model_o, widget);
    TableDelegateO(view, config);

    wgt_hash.insert(node_id, widget);
}

void MainWindow::TableConnectF(QTableView* table_view, LeafModel* table_model, TreeModel* tree_model) const
{
    connect(table_model, &LeafModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);

    connect(table_model, &LeafModel::SSyncDelta, tree_model, &TreeModel::RSyncDelta);

    connect(table_model, &LeafModel::SRemoveOneEntry, &LeafSStation::Instance(), &LeafSStation::RRemoveOneEntry);
    connect(table_model, &LeafModel::SAppendOneEntry, &LeafSStation::Instance(), &LeafSStation::RAppendOneEntry);
    connect(table_model, &LeafModel::SUpdateBalance, &LeafSStation::Instance(), &LeafSStation::RUpdateBalance);
}

void MainWindow::TableConnectI(QTableView* table_view, LeafModel* table_model, TreeModel* tree_model) const
{
    connect(table_model, &LeafModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);

    connect(table_model, &LeafModel::SSyncDelta, tree_model, &TreeModel::RSyncDelta);

    connect(table_model, &LeafModel::SRemoveOneEntry, &LeafSStation::Instance(), &LeafSStation::RRemoveOneEntry);
    connect(table_model, &LeafModel::SAppendOneEntry, &LeafSStation::Instance(), &LeafSStation::RAppendOneEntry);
    connect(table_model, &LeafModel::SUpdateBalance, &LeafSStation::Instance(), &LeafSStation::RUpdateBalance);
}

void MainWindow::TableConnectT(QTableView* table_view, LeafModel* table_model, TreeModel* tree_model) const
{
    connect(table_model, &LeafModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);

    connect(table_model, &LeafModel::SSyncDelta, tree_model, &TreeModel::RSyncDelta);

    connect(table_model, &LeafModel::SRemoveOneEntry, &LeafSStation::Instance(), &LeafSStation::RRemoveOneEntry);
    connect(table_model, &LeafModel::SAppendOneEntry, &LeafSStation::Instance(), &LeafSStation::RAppendOneEntry);
    connect(table_model, &LeafModel::SUpdateBalance, &LeafSStation::Instance(), &LeafSStation::RUpdateBalance);
}

void MainWindow::TableConnectO(QTableView* table_view, LeafModelO* leaf_model_order, TreeModelO* tree_model, LeafWidgetO* widget) const
{
    connect(leaf_model_order, &LeafModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);

    connect(leaf_model_order, &LeafModel::SSyncDelta, widget, &LeafWidgetO::RSyncDelta);

    connect(widget, &LeafWidgetO::SSyncParty, leaf_model_order, &LeafModelO::RSyncParty);
    connect(widget, &LeafWidgetO::SSyncFinished, leaf_model_order, &LeafModelO::RSyncFinished);
    connect(widget, &LeafWidgetO::SSyncFinished, tree_model, &TreeModelO::RSyncFinished);

    connect(widget, &LeafWidgetO::SSyncParty, this, &MainWindow::RSyncParty);
    connect(widget, &LeafWidgetO::SEnableAction, this, &MainWindow::REnableAction);
}

void MainWindow::TableConnectS(QTableView* table_view, LeafModel* entry_model) const
{
    connect(entry_model, &LeafModel::SResizeColumnToContents, table_view, &QTableView::resizeColumnToContents);
}

void MainWindow::TableDelegateF(QTableView* table_view, TreeModel* tree_model, CSectionConfig& config, const QUuid& node_id) const
{
    auto* issued_time { new TableIssuedTime(config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kIssuedTime), issued_time);

    auto* lhs_rate { new Double(config.rate_decimal, 0, kMaxNumeric_16_8, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kLhsRate), lhs_rate);

    auto* line { new LineGuard(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kCode), line);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kDescription), line);

    auto* document { new Document(sc_->global_config.document_dir, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kDocument), document);

    auto* is_checked { new CheckBox(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumF::kIsChecked), is_checked);

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

    auto* line { new LineGuard(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumI::kCode), line);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumI::kDescription), line);

    auto* document { new Document(sc_->global_config.document_dir, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumI::kDocument), document);

    auto* is_checked { new CheckBox(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumI::kIsChecked), is_checked);

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

    auto* line { new LineGuard(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumT::kCode), line);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumT::kDescription), line);

    auto* document { new Document(sc_->global_config.document_dir, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumT::kDocument), document);

    auto* is_checked { new CheckBox(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumT::kIsChecked), is_checked);

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
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumS::kIssuedTime), issued_time);

    auto* unit_price { new Double(config.rate_decimal, 0, kMaxNumeric_12_4, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumS::kUnitPrice), unit_price);

    auto* line { new LineGuard(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumS::kCode), line);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumS::kDescription), line);

    auto tree_model_i { sc_i_.tree_model };

    auto* ext_filter_model { tree_model_i->IncludeUnitModel(std::to_underlying(UnitI::kExternal)) };
    auto* ext_item { new FilterUnit(tree_model_i, ext_filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumS::kExternalItem), ext_item);

    auto* document { new Document(sc_->global_config.document_dir, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumS::kDocument), document);

    auto* is_checked { new CheckBox(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumS::kIsChecked), is_checked);

    auto* int_filter_model { tree_model_i->IncludeUnitModel(std::to_underlying(UnitI::kInternal)) };
    auto* int_item { new FilterUnit(tree_model_i, int_filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumS::kRhsNode), int_item);
}

void MainWindow::TableDelegateO(QTableView* table_view, CSectionConfig& config) const
{
    auto tree_model_i { sc_i_.tree_model };
    auto* int_filter_model { tree_model_i->IncludeUnitModel(std::to_underlying(UnitI::kInternal)) };

    auto* int_item { new FilterUnit(tree_model_i, int_filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kRhsNode), int_item);

    auto* ext_filter_model { tree_model_i->IncludeUnitModel(std::to_underlying(UnitI::kExternal)) };
    auto* ext_item { new FilterUnit(tree_model_i, ext_filter_model, table_view) };

    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kExternalItem), ext_item);

    auto* color { new ColorR(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kColor), color);

    auto* line { new LineGuard(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kDescription), line);

    auto* price { new Double(config.rate_decimal, kMinNumeric_12_4, kMaxNumeric_12_4, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kUnitPrice), price);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kDiscountPrice), price);

    auto* quantity { new Double(config.amount_decimal, kMinNumeric_12_4, kMaxNumeric_12_4, kCoefficient8, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kFirst), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryEnumO::kSecond), quantity);

    auto* amount { new DoubleSpinRNoneZero(config.amount_decimal, kCoefficient16, table_view) };
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
    const int index = tab_widget->addTab(tree_widget, name);

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
    case Section::kStakeholder:
        TreeDelegateS(view, info, config);
        TreeConnectS(view, tree_model, entry_hub);
        break;
    case Section::kItem:
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
    auto* line { new LineGuard(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kCode), line);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kDescription), line);

    auto* plain_text { new PlainTextGuard(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kNote), plain_text);

    auto* direction_rule { new DBoolString(info.rule_map, QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kDirectionRule), direction_rule);

    auto* unit { new DIntStringR(info.unit_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kUnit), unit);

    auto* kind { new DIntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kKind), kind);

    auto* final_total { new DoubleSpinUnitR(section.amount_decimal, sc_f_.global_config.default_unit, info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kFinalTotal), final_total);

    auto* initial_total { new FinanceForeignR(section.amount_decimal, sc_f_.global_config.default_unit, info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumF::kInitialTotal), initial_total);
}

void MainWindow::TreeDelegateT(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const
{
    auto* line { new LineGuard(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnum::kCode), line);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnum::kDescription), line);

    auto* plain_text { new PlainTextGuard(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnum::kNote), plain_text);

    auto* direction_rule { new DBoolString(info.rule_map, QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnum::kDirectionRule), direction_rule);

    auto* unit { new DIntStringR(info.unit_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnum::kUnit), unit);

    auto* kind { new DIntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnum::kKind), kind);

    auto* quantity { new DoubleSpinR(section.amount_decimal, kCoefficient16, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kInitialTotal), quantity);

    auto* amount { new DoubleSpinUnitR(section.amount_decimal, sc_f_.global_config.default_unit, sc_f_.info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kFinalTotal), amount);

    auto* color { new Color(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kColor), color);

    auto* is_finished { new CheckBox(QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kIsFinished), is_finished);

    auto* document { new Document(sc_t_.global_config.document_dir, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kDocument), document);

    auto* issued_time { new TreeIssuedTime(section.date_format, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumT::kIssuedTime), issued_time);
}

void MainWindow::TreeDelegateI(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const
{
    auto* line { new LineGuard(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnum::kCode), line);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnum::kDescription), line);

    auto* plain_text { new PlainTextGuard(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnum::kNote), plain_text);

    auto* direction_rule { new DBoolString(info.rule_map, QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnum::kDirectionRule), direction_rule);

    auto* unit { new DIntStringR(info.unit_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnum::kUnit), unit);

    auto* kind { new DIntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnum::kKind), kind);

    auto* quantity { new DoubleSpinR(section.amount_decimal, kCoefficient16, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kInitialTotal), quantity);

    auto* amount { new DoubleSpinUnitR(section.amount_decimal, sc_f_.global_config.default_unit, sc_f_.info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kFinalTotal), amount);

    auto* unit_price { new DoubleGuard(section.rate_decimal, 0, kMaxNumeric_12_4, kCoefficient8, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kUnitPrice), unit_price);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kCommission), unit_price);

    auto* color { new Color(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumI::kColor), color);
}

void MainWindow::TreeDelegateS(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const
{
    auto* line { new LineGuard(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumS::kCode), line);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumS::kDescription), line);

    auto* plain_text { new PlainTextGuard(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumS::kNote), plain_text);

    auto* unit { new DIntStringR(info.unit_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumS::kUnit), unit);

    auto* kind { new DIntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumS::kKind), kind);

    auto* amount { new DoubleSpinUnitR(section.amount_decimal, sc_f_.global_config.default_unit, sc_f_.info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumS::kFinalTotal), amount);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumS::kInitialTotal), amount);

    auto* payment_term { new Int(0, 36500, tree_view) }; // one hundred years
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumS::kPaymentTerm), payment_term);
}

void MainWindow::TreeDelegateO(QTreeView* tree_view, CSectionInfo& info, CSectionConfig& section) const
{
    auto* line { new LineGuard(tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kDescription), line);

    auto* direction_rule { new OrderRule(info.rule_map, QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kDirectionRule), direction_rule);

    auto* unit { new OrderUnit(info.unit_map, info.unit_model, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kUnit), unit);

    auto* kind { new DIntStringR(info.kind_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kKind), kind);

    auto* amount { new DoubleSpinUnitR(section.amount_decimal, sc_f_.global_config.default_unit, sc_f_.info.unit_symbol_map, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kInitialTotal), amount);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kFinalTotal), amount);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kDiscountTotal), amount);

    auto* quantity { new DoubleSpinRNoneZero(section.amount_decimal, kCoefficient16, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kSecondTotal), quantity);
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kFirstTotal), quantity);

    auto tree_model_s { sc_s_.tree_model };

    auto* filter_model { tree_model_s->IncludeUnitModel(std::to_underlying(UnitS::kEmp)) };
    auto* employee { new FilterUnit(tree_model_s, filter_model, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kEmployee), employee);

    auto* name { new OrderNameR(tree_model_s, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kName), name);

    auto* issued_time { new TreeIssuedTime(section.date_format, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kIssuedTime), issued_time);

    auto* is_finished { new CheckBox(QEvent::MouseButtonDblClick, tree_view) };
    tree_view->setItemDelegateForColumn(std::to_underlying(NodeEnumO::kIsFinished), is_finished);
}

void MainWindow::TreeConnectF(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SResizeColumnToContents, tree_view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRemoveEntryHash, &LeafSStation::Instance(), &LeafSStation::RRemoveEntryHash, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveMultiEntry, &LeafSStation::Instance(), &LeafSStation::RRemoveMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendMultiEntry, &LeafSStation::Instance(), &LeafSStation::RAppendMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SCheckAction, &LeafSStation::Instance(), &LeafSStation::RCheckAction, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAppendOneEntry, &LeafSStation::Instance(), &LeafSStation::RAppendOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveOneEntry, &LeafSStation::Instance(), &LeafSStation::RRemoveOneEntry, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshField, &LeafSStation::Instance(), &LeafSStation::RRefreshField, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SUpdateBalance, &LeafSStation::Instance(), &LeafSStation::RUpdateBalance, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SSyncRule, &LeafSStation::Instance(), &LeafSStation::RSyncRule, Qt::UniqueConnection);
}

void MainWindow::TreeConnectI(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SResizeColumnToContents, tree_view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRemoveEntryHash, &LeafSStation::Instance(), &LeafSStation::RRemoveEntryHash, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveMultiEntry, &LeafSStation::Instance(), &LeafSStation::RRemoveMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendMultiEntry, &LeafSStation::Instance(), &LeafSStation::RAppendMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SCheckAction, &LeafSStation::Instance(), &LeafSStation::RCheckAction, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAppendOneEntry, &LeafSStation::Instance(), &LeafSStation::RAppendOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveOneEntry, &LeafSStation::Instance(), &LeafSStation::RRemoveOneEntry, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshField, &LeafSStation::Instance(), &LeafSStation::RRefreshField, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SUpdateBalance, &LeafSStation::Instance(), &LeafSStation::RUpdateBalance, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SSyncRule, &LeafSStation::Instance(), &LeafSStation::RSyncRule, Qt::UniqueConnection);
}

void MainWindow::TreeConnectT(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SResizeColumnToContents, tree_view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRemoveEntryHash, &LeafSStation::Instance(), &LeafSStation::RRemoveEntryHash, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveMultiEntry, &LeafSStation::Instance(), &LeafSStation::RRemoveMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SAppendMultiEntry, &LeafSStation::Instance(), &LeafSStation::RAppendMultiEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SCheckAction, &LeafSStation::Instance(), &LeafSStation::RCheckAction, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SAppendOneEntry, &LeafSStation::Instance(), &LeafSStation::RAppendOneEntry, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SRemoveOneEntry, &LeafSStation::Instance(), &LeafSStation::RRemoveOneEntry, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SRefreshField, &LeafSStation::Instance(), &LeafSStation::RRefreshField, Qt::UniqueConnection);
    connect(entry_hub, &EntryHub::SUpdateBalance, &LeafSStation::Instance(), &LeafSStation::RUpdateBalance, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SSyncRule, &LeafSStation::Instance(), &LeafSStation::RSyncRule, Qt::UniqueConnection);
}

void MainWindow::TreeConnectS(QTreeView* tree_view, TreeModel* tree_model, const EntryHub* entry_hub) const
{
    connect(tree_view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(tree_model, &TreeModel::SFreeWidget, this, &MainWindow::RFreeWidget, Qt::UniqueConnection);

    connect(tree_model, &TreeModel::SResizeColumnToContents, tree_view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);

    connect(entry_hub, &EntryHub::SCheckAction, &LeafSStation::Instance(), &LeafSStation::RCheckAction, Qt::UniqueConnection);
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
    node->unit = parent_id.isNull() ? sc_->global_config.default_unit : model->Unit(parent_id);

    model->SetParent(node, parent_id);

    if (start_ == Section::kSale || start_ == Section::kPurchase)
        InsertNodeO(node, parent, row);

    if (start_ != Section::kSale && start_ != Section::kPurchase)
        InsertNodeFIST(node, parent, parent_id, row);
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
    node->unit = parent_id.isNull() ? sc_->global_config.default_unit : model->Unit(parent_id);
    node->kind = kBranch;

    model->SetParent(node, parent_id);

    auto tree_model { sc_->tree_model };
    auto unit_model { sc_->info.unit_model };

    auto parent_path { tree_model->Path(parent_id) };
    if (!parent_path.isEmpty())
        parent_path += local_config_.separator;

    const auto children_name { tree_model->ChildrenName(parent_id) };
    const int row { current_index.row() + 1 };

    QDialog* dialog { new InsertNodeBranch(node, unit_model, parent_path, children_name, this) };

    connect(dialog, &QDialog::accepted, this, [=, this]() {
        if (tree_model->InsertNode(row, parent_index, node)) {
            auto index = tree_model->index(row, 0, parent_index);
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

    if (auto* tree_widget { dynamic_cast<TreeWidget*>(widget) }) {
        RemoveNode();
    }

    if (auto* leaf_widget { dynamic_cast<LeafWidget*>(widget) }) {
        WidgetUtils::RemoveEntry(leaf_widget);
    }
}

void MainWindow::RemoveNode()
{
    auto view { sc_->tree_view };
    assert(view);

    if (!WidgetUtils::HasSelection(view))
        return;

    const auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    auto model { sc_->tree_model };
    assert(model);

    const QUuid node_id { index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() };
    const int kind { index.siblingAtColumn(std::to_underlying(NodeEnum::kKind)).data().toInt() };

    switch (kind) {
    case kBranch: {
        RemoveBranchNode(model, index, node_id);
        break;
    }
    case kLeaf: {
        const auto message { JsonGen::LeafCheckBeforeRemove(sc_->info.section_str, node_id) };
        WebSocket::Instance().SendMessage(kLeafReference, message);
        break;
    }
    default:
        break;
    }
}

void MainWindow::RRemoveLeafNode(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    const auto id = QUuid(obj.value(kId).toString());

    const bool exteral_reference { obj.value(kExternalReference).toBool() };

    auto* section_contex = GetSectionContex(section);

    auto model = section_contex->tree_model;
    const auto index = model->GetIndex(id);

    const int unit { index.siblingAtColumn(std::to_underlying(NodeEnum::kUnit)).data().toInt() };

    auto* dialog { new RemoveLeafNodeDialog(model, section_contex->info, id, unit, exteral_reference, this) };
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(true);
    dialog->show();

    connect(dialog, &RemoveLeafNodeDialog::SRemoveNode, model, &TreeModel::RRemoveNode, Qt::SingleShotConnection);
}

void MainWindow::RGlobalConfig(const QJsonArray& arr)
{
    for (const QJsonValue& val : arr) {
        if (!val.isObject()) {
            qWarning() << "Invalid item in GlobalConfig array:" << val;
            continue;
        }

        QJsonObject obj = val.toObject();
        QString section = obj.value("section").toString();
        int default_unit = obj.value("default_unit").toInt();
        QString document_dir = obj.value("document_dir").toString();

        auto* section_contex = GetSectionContex(section);
        if (!section_contex) {
            continue;
        }

        section_contex->global_config.default_unit = default_unit;
        section_contex->global_config.document_dir = document_dir;
    }
}

void MainWindow::RDocumentDir(const QString& section, const QString& document_dir)
{
    auto* sc { GetSectionContex(section) };
    sc->global_config.document_dir = document_dir;
}

void MainWindow::RDefaultUnit(const QString& section, int unit)
{
    auto* sc { GetSectionContex(section) };
    sc->global_config.default_unit = unit;
}

void MainWindow::RUpdateDefaultUnitFailed(const QString& /*section*/)
{
    QMessageBox::warning(this, tr("Update Failed"), tr("Cannot change the base unit for section Finance because related entries already exist."));
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
    local_settings_.clear();
    section_settings_.clear();

    sc_f_.Clear();
    sc_i_.Clear();
    sc_t_.Clear();
    sc_s_.Clear();
    sc_sale_.Clear();
    sc_purchase_.Clear();

    WebSocket::Instance().Clear();
    LeafSStation::Instance().Clear();

    ui->tabWidget->clear();
}

void MainWindow::EnableAction(bool enable) const
{
    ui->actionAppendNode->setEnabled(enable);
    ui->actionCheckAll->setEnabled(enable);
    ui->actionCheckNone->setEnabled(enable);
    ui->actionCheckReverse->setEnabled(enable);
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
    section_group_->addButton(ui->rBtnFinance, 0);
    section_group_->addButton(ui->rBtnItem, 1);
    section_group_->addButton(ui->rBtnTask, 2);
    section_group_->addButton(ui->rBtnStakeholder, 3);
    section_group_->addButton(ui->rBtnSale, 4);
    section_group_->addButton(ui->rBtnPurchase, 5);
}

void MainWindow::RemoveBranchNode(TreeModel* tree_model, const QModelIndex& index, const QUuid& node_id)
{
    QMessageBox msg {};
    msg.setIcon(QMessageBox::Question);
    msg.setText(tr("Remove %1").arg(tree_model->Path(node_id)));
    msg.setInformativeText(tr("The branch will be removed, and its direct children will be promoted to the same level."));
    msg.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);

    if (msg.exec() == QMessageBox::Ok) {
        const auto message { JsonGen::RemoveBranchNode(sc_->info.section_str, node_id) };
        WebSocket::Instance().SendMessage(kBranchRemove, message);
        tree_model->removeRows(index.row(), 1, index.parent());
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
    LeafSStation::Instance().DeregisterModel(node_id);
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

    start_ = Section(local_settings_->value(kStartSection, 0).toInt());

    switch (start_) {
    case Section::kFinance:
        ui->rBtnFinance->setChecked(true);
        break;
    case Section::kStakeholder:
        ui->rBtnStakeholder->setChecked(true);
        break;
    case Section::kItem:
        ui->rBtnItem->setChecked(true);
        break;
    case Section::kTask:
        ui->rBtnTask->setChecked(true);
        break;
    case Section::kSale:
        ui->rBtnSale->setChecked(true);
        break;
    case Section::kPurchase:
        ui->rBtnPurchase->setChecked(true);
        break;
    default:
        break;
    }
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

    const int unit { std::to_underlying(UnitO::kMS) };
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
    connect(model, &SettlementModel::SUpdateAmount, static_cast<TreeModelS*>(sc_s_.tree_model.data()), &TreeModelS::RUpdateAmount);
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
    if (local_settings_) {
        WidgetUtils::WriteConfig(ui->splitter, &QSplitter::saveState, local_settings_, kSplitter, kState);
        WidgetUtils::WriteConfig(this, &QMainWindow::saveState, local_settings_, kMainwindow, kState, 0);
        WidgetUtils::WriteConfig(this, &QMainWindow::saveGeometry, local_settings_, kMainwindow, kGeometry);
        WidgetUtils::WriteConfig(local_settings_, std::to_underlying(start_), kStart, kSection);
    }

    if (section_settings_) {
        WidgetUtils::WriteConfig(sc_f_.tree_view->header(), &QHeaderView::saveState, section_settings_, kFinance, kHeaderState);
        WidgetUtils::WriteConfig(sc_i_.tree_view->header(), &QHeaderView::saveState, section_settings_, kItem, kHeaderState);
        WidgetUtils::WriteConfig(sc_s_.tree_view->header(), &QHeaderView::saveState, section_settings_, kStakeholder, kHeaderState);
        WidgetUtils::WriteConfig(sc_t_.tree_view->header(), &QHeaderView::saveState, section_settings_, kTask, kHeaderState);
        WidgetUtils::WriteConfig(sc_sale_.tree_view->header(), &QHeaderView::saveState, section_settings_, kSale, kHeaderState);
        WidgetUtils::WriteConfig(sc_purchase_.tree_view->header(), &QHeaderView::saveState, section_settings_, kPurchase, kHeaderState);
    }
}

SectionContext* MainWindow::GetSectionContex(const QString& section)
{
    const static QMap<QString, SectionContext*> section_map {
        { kFinance, &sc_f_ },
        { kStakeholder, &sc_s_ },
        { kItem, &sc_i_ },
        { kTask, &sc_t_ },
        { kSale, &sc_sale_ },
        { kPurchase, &sc_purchase_ },
    };

    auto it = section_map.constFind(section);
    if (it == section_map.cend()) {
        qCritical() << "SectionTriple: Unknown section:" << section;
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

void MainWindow::SetAppFontByDpi()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    const qreal screen_dpi = screen->logicalDotsPerInch();
    const int font_size = (screen_dpi >= 96) ? 12 : 14;

    QFont app_font = qApp->font();
    app_font.setPointSize(font_size);

    qApp->setFont(app_font);
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
    auto* price { new DoubleSpinRNoneZero(config.rate_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kUnitPrice), price);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kDiscountPrice), price);

    auto* quantity { new DoubleSpinRNoneZero(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kFirst), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kSecond), quantity);

    auto* amount { new DoubleSpinRNoneZero(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kInitial), amount);

    auto* issued_time { new IssuedTimeR(sc_sale_.section_config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kIssuedTime), issued_time);

    auto stakeholder_tree_model { sc_s_.tree_model };
    auto* external_item { new NodePathR(stakeholder_tree_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kExternalItem), external_item);

    auto* section { new SectionR(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kSection), section);

    if (start_ == Section::kItem) {
        auto* name { new NodeNameR(stakeholder_tree_model, table_view) };
        table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kPIId), name);
    }

    if (start_ == Section::kStakeholder) {
        auto* internal_item { new NodeNameR(sc_i_.tree_model, table_view) };
        table_view->setItemDelegateForColumn(std::to_underlying(EntryRefEnum::kPIId), internal_item);
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
    auto* quantity { new DoubleSpinRNoneZero(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCFirst), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCSecond), quantity);

    auto* amount { new DoubleSpinRNoneZero(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCGrossAmount), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCSettlement), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kPBalance), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kCBalance), amount);

    auto* name { new NodeNameR(sc_s_.tree_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementEnum::kParty), name);
}

void MainWindow::DelegateSettlement(QTableView* table_view, CSectionConfig& config) const
{
    auto* amount { new DoubleSpinRNoneZero(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kInitialTotal), amount);

    auto* is_finished { new CheckBox(QEvent::MouseButtonDblClick, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kIsFinished), is_finished);

    auto* line { new LineGuard(table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kDescription), line);

    auto* issued_time { new TreeIssuedTime(kDateFST, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kIssuedTime), issued_time);

    auto model { sc_s_.tree_model };
    const int unit { start_ == Section::kSale ? std::to_underlying(UnitS::kCust) : std::to_underlying(UnitS::kVend) };

    auto* filter_model { model->IncludeUnitModel(unit) };
    auto* node { new TableComboFilter(model, filter_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kStakeholder), node);
}

void MainWindow::DelegateSettlementPrimary(QTableView* table_view, CSectionConfig& config) const
{
    auto* amount { new DoubleSpinRNoneZero(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kInitialTotal), amount);

    auto* employee { new NodeNameR(sc_s_.tree_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kStakeholder), employee);

    auto* is_checked { new CheckBox(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kIsFinished), is_checked);

    auto* issued_time { new IssuedTimeR(kDateFST, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(SettlementEnum::kIssuedTime), issued_time);
}

void MainWindow::DelegateStatementPrimary(QTableView* table_view, CSectionConfig& config) const
{
    auto* quantity { new DoubleSpinRNoneZero(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementPrimaryEnum::kFirst), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementPrimaryEnum::kSecond), quantity);

    auto* amount { new DoubleSpinRNoneZero(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementPrimaryEnum::kInitialTotal), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementPrimaryEnum::kFinalTotal), amount);

    auto* employee { new NodeNameR(sc_s_.tree_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementPrimaryEnum::kEmployee), employee);

    auto* is_checked { new CheckBox(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementPrimaryEnum::kIsChecked), is_checked);

    auto* issued_time { new IssuedTimeR(sc_sale_.section_config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementPrimaryEnum::kIssuedTime), issued_time);
}

void MainWindow::DelegateStatementSecondary(QTableView* table_view, CSectionConfig& config) const
{
    auto* quantity { new DoubleSpinRNoneZero(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kFirst), quantity);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kSecond), quantity);

    auto* amount { new DoubleSpinRNoneZero(config.amount_decimal, kCoefficient16, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kInitialTotal), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kFinalTotal), amount);
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kUnitPrice), amount);

    auto* is_checked { new CheckBox(QEvent::MouseButtonRelease, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kIsChecked), is_checked);

    auto* issued_time { new IssuedTimeR(sc_sale_.section_config.date_format, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kIssuedTime), issued_time);

    auto* external_item { new NodePathR(sc_s_.tree_model, table_view) };
    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kSupportNode), external_item);

    auto tree_model_i { sc_i_.tree_model };

    auto* int_filter_model { tree_model_i->IncludeUnitModel(std::to_underlying(UnitI::kInternal)) };
    auto* int_item { new FilterUnit(tree_model_i, int_filter_model, table_view) };

    table_view->setItemDelegateForColumn(std::to_underlying(StatementSecondaryEnum::kRhsNode), int_item);
}

void MainWindow::SetUniqueConnection() const
{
    connect(ui->actionCheckAll, &QAction::triggered, this, &MainWindow::RUpdateState);
    connect(ui->actionCheckNone, &QAction::triggered, this, &MainWindow::RUpdateState);
    connect(ui->actionCheckReverse, &QAction::triggered, this, &MainWindow::RUpdateState);
    connect(ui->actionQuit, &QAction::triggered, qApp, &QCoreApplication::quit);

    connect(section_group_, &QButtonGroup::idClicked, this, &MainWindow::RSectionGroup);

    connect(&WebSocket::Instance(), &WebSocket::SInitializeContext, this, &MainWindow::RInitializeContext);
    connect(&WebSocket::Instance(), &WebSocket::SRemoveLeafNode, this, &MainWindow::RRemoveLeafNode);
    connect(&WebSocket::Instance(), &WebSocket::SGlobalConfig, this, &MainWindow::RGlobalConfig);
    connect(&WebSocket::Instance(), &WebSocket::SDefaultUnit, this, &MainWindow::RDefaultUnit);
    connect(&WebSocket::Instance(), &WebSocket::SUpdateDefaultUnitFailed, this, &MainWindow::RUpdateDefaultUnitFailed);
    connect(&WebSocket::Instance(), &WebSocket::SDocumentDir, this, &MainWindow::RDocumentDir);
    connect(&WebSocket::Instance(), &WebSocket::SConnectResult, this, &MainWindow::RConnectResult);
    connect(&WebSocket::Instance(), &WebSocket::SActionLoginTriggered, this, &MainWindow::on_actionLogin_triggered);
}

void MainWindow::InitContextFinance()
{
    auto& info { sc_f_.info };
    auto& section_config { sc_f_.section_config };
    auto& global_config { sc_f_.global_config };
    auto& entry_hub { sc_f_.entry_hub };
    auto& tree_model { sc_f_.tree_model };
    auto& tree_view { sc_f_.tree_view };
    auto& tree_widget { sc_f_.tree_widget };

    info.section = Section::kFinance;
    info.section_str = kFinance;
    info.node = kFinanceNode;
    info.path = kFinancePath;
    info.entry = kFinanceEntry;

    QStringList unit_list { "CNY", "HKD", "USD", "GBP", "JPY", "CAD", "AUD", "EUR" };
    QStringList unit_symbol_list { "¥", "$", "$", "£", "¥", "$", "$", "€" };

    for (int i = 0; i != unit_list.size(); ++i) {
        info.unit_map.insert(i, unit_list.at(i));
        info.unit_symbol_map.insert(i, unit_symbol_list.at(i));
    }

    info.rule_map.insert(kDDCI, kRuleDDCI);
    info.rule_map.insert(kDICD, kRuleDICD);

    info.kind_map.insert(kBranch, kBranchKind);
    info.kind_map.insert(kLeaf, kLeafKind);

    info.unit_model = MainWindowUtils::CreateModelFromMap(info.unit_map, this);
    info.rule_model = MainWindowUtils::CreateModelFromMap(info.rule_map, this);

    ReadSectionConfig(section_config, kFinance);

    entry_hub = new EntryHubF(info, this);
    tree_model = new TreeModelF(info, local_config_.separator, global_config.default_unit, this);

    WebSocket::Instance().RegisterTreeModel(kFinance, tree_model);
    WebSocket::Instance().RegisterEntryHub(kFinance, entry_hub);

    tree_widget = new TreeWidgetF(tree_model, info, global_config, section_config, this);
    tree_view = tree_widget->View();

    connect(tree_model, &TreeModel::SSyncStatusValue, tree_widget, &TreeWidget::RSyncStatusValue, Qt::UniqueConnection);
}

void MainWindow::InitContextItem()
{
    auto& info { sc_i_.info };
    auto& section_config { sc_i_.section_config };
    auto& global_config { sc_i_.global_config };
    auto& entry_hub { sc_i_.entry_hub };
    auto& tree_model { sc_i_.tree_model };
    auto& tree_view { sc_i_.tree_view };
    auto& tree_widget { sc_i_.tree_widget };

    info.section = Section::kItem;
    info.section_str = kItem;
    info.node = kItemNode;
    info.path = kItemPath;
    info.entry = kItemEntry;

    info.unit_map.insert(std::to_underlying(UnitI::kInternal), tr("INT"));
    info.unit_map.insert(std::to_underlying(UnitI::kPlaceholder), {});
    info.unit_map.insert(std::to_underlying(UnitI::kExternal), tr("EXT"));

    info.rule_map.insert(kDDCI, kRuleDDCI);
    info.rule_map.insert(kDICD, kRuleDICD);

    info.kind_map.insert(kBranch, kBranchKind);
    info.kind_map.insert(kLeaf, kLeafKind);

    info.unit_model = MainWindowUtils::CreateModelFromMap(info.unit_map, this);
    info.rule_model = MainWindowUtils::CreateModelFromMap(info.rule_map, this);

    ReadSectionConfig(section_config, kItem);

    entry_hub = new EntryHubI(info, this);
    tree_model = new TreeModelI(info, local_config_.separator, global_config.default_unit, this);

    WebSocket::Instance().RegisterTreeModel(kItem, tree_model);
    WebSocket::Instance().RegisterEntryHub(kItem, entry_hub);

    tree_widget = new TreeWidgetI(tree_model, section_config, this);
    tree_view = tree_widget->View();

    connect(tree_model, &TreeModel::SSyncStatusValue, tree_widget, &TreeWidget::RSyncStatusValue, Qt::UniqueConnection);
}

void MainWindow::InitContextTask()
{
    auto& info { sc_t_.info };
    auto& section_config { sc_t_.section_config };
    auto& global_config { sc_t_.global_config };
    auto& entry_hub { sc_t_.entry_hub };
    auto& tree_model { sc_t_.tree_model };
    auto& tree_view { sc_t_.tree_view };
    auto& tree_widget { sc_t_.tree_widget };

    info.section = Section::kTask;
    info.section_str = kTask;
    info.node = kTaskNode;
    info.path = kTaskPath;
    info.entry = kTaskEntry;

    info.unit_map.insert(std::to_underlying(UnitT::kInternal), tr("INT"));
    info.unit_map.insert(std::to_underlying(UnitT::kPlaceholder), {});
    info.unit_map.insert(std::to_underlying(UnitT::kExternal), tr("EXT"));

    info.rule_map.insert(kDDCI, kRuleDDCI);
    info.rule_map.insert(kDICD, kRuleDICD);

    info.kind_map.insert(kBranch, kBranchKind);
    info.kind_map.insert(kLeaf, kLeafKind);

    info.unit_model = MainWindowUtils::CreateModelFromMap(info.unit_map, this);
    info.rule_model = MainWindowUtils::CreateModelFromMap(info.rule_map, this);

    ReadSectionConfig(section_config, kTask);

    entry_hub = new EntryHubT(info, this);
    tree_model = new TreeModelT(info, local_config_.separator, global_config.default_unit, this);

    WebSocket::Instance().RegisterTreeModel(kTask, tree_model);
    WebSocket::Instance().RegisterEntryHub(kTask, entry_hub);

    const QDate today = QDate::currentDate();
    const QDate start_date = QDate(today.year() - 1, 1, 1);
    const QDateTime start_dt(start_date, kStartTime);
    const QDateTime end_dt(today, kEndTime);

    tree_widget = new TreeWidgetTO(kTask, tree_model, start_dt, end_dt, this);
    tree_view = tree_widget->View();
}

void MainWindow::InitContextStakeholder()
{
    auto& info { sc_s_.info };
    auto& section_config { sc_s_.section_config };
    auto& global_config { sc_s_.global_config };
    auto& entry_hub { sc_s_.entry_hub };
    auto& tree_model { sc_s_.tree_model };
    auto& tree_view { sc_s_.tree_view };
    auto& tree_widget { sc_s_.tree_widget };

    info.section = Section::kStakeholder;
    info.section_str = kStakeholder;
    info.node = kStakeholderNode;
    info.path = kStakeholderPath;
    info.entry = kStakeholderEntry;

    info.unit_map.insert(std::to_underlying(UnitS::kCust), tr("CUST"));
    info.unit_map.insert(std::to_underlying(UnitS::kEmp), tr("EMP"));
    info.unit_map.insert(std::to_underlying(UnitS::kVend), tr("VEND"));

    info.kind_map.insert(kBranch, kBranchKind);
    info.kind_map.insert(kLeaf, kLeafKind);

    info.unit_model = MainWindowUtils::CreateModelFromMap(info.unit_map, this);

    ReadSectionConfig(section_config, kStakeholder);

    entry_hub = new EntryHubS(info, this);
    tree_model = new TreeModelS(info, local_config_.separator, global_config.default_unit, this);

    WebSocket::Instance().RegisterTreeModel(kStakeholder, tree_model);
    WebSocket::Instance().RegisterEntryHub(kStakeholder, entry_hub);

    tree_widget = new TreeWidgetS(tree_model, this);
    tree_view = tree_widget->View();
}

void MainWindow::InitContextSale()
{
    auto& info { sc_sale_.info };
    auto& section_config { sc_sale_.section_config };
    auto& global_config { sc_sale_.global_config };
    auto& entry_hub { sc_sale_.entry_hub };
    auto& tree_model { sc_sale_.tree_model };
    auto& tree_view { sc_sale_.tree_view };
    auto& tree_widget { sc_sale_.tree_widget };

    info.section = Section::kSale;
    info.section_str = kSale;
    info.node = kSaleNode;
    info.path = kSalePath;
    info.entry = kSaleEntry;
    info.settlement = kSaleSettlement;

    info.rule_map.insert(true, "TO");
    info.rule_map.insert(false, "RO");

    info.unit_map.insert(std::to_underlying(UnitO::kIS), tr("IS"));
    info.unit_map.insert(std::to_underlying(UnitO::kMS), tr("MS"));
    info.unit_map.insert(std::to_underlying(UnitO::kPEND), tr("PEND"));

    info.kind_map.insert(kBranch, kBranchKind);
    info.kind_map.insert(kLeaf, kLeafKind);

    info.unit_model = MainWindowUtils::CreateModelFromMap(info.unit_map, this);
    info.unit_model->sort(0, Qt::DescendingOrder);

    info.rule_model = MainWindowUtils::CreateModelFromMap(info.rule_map, this);

    ReadSectionConfig(section_config, kSale);

    auto* entry_hub_o = new EntryHubO(info, this);
    auto* tree_model_o = new TreeModelO(info, local_config_.separator, global_config.default_unit, this);

    entry_hub = entry_hub_o;
    tree_model = tree_model_o;

    const QDate today = QDate::currentDate();
    const QDateTime start_dt(today, kStartTime);
    const QDateTime end_dt(today, kEndTime);

    WebSocket::Instance().RegisterTreeModel(kSale, tree_model);
    WebSocket::Instance().RegisterEntryHub(kSale, entry_hub);

    tree_widget = new TreeWidgetTO(kSale, tree_model, start_dt, end_dt, this);
    tree_view = tree_widget->View();

    connect(tree_model_o, &TreeModelO::SUpdateAmount, static_cast<TreeModelS*>(sc_s_.tree_model.data()), &TreeModelS::RUpdateAmount);
    connect(entry_hub_o, &EntryHubO::SSyncPrice, static_cast<EntryHubS*>(sc_s_.entry_hub.data()), &EntryHubS::RPriceSList);
}

void MainWindow::InitContextPurchase()
{
    auto& info { sc_purchase_.info };
    auto& section_config { sc_purchase_.section_config };
    auto& global_config { sc_purchase_.global_config };
    auto& entry_hub { sc_purchase_.entry_hub };
    auto& tree_model { sc_purchase_.tree_model };
    auto& tree_view { sc_purchase_.tree_view };
    auto& tree_widget { sc_purchase_.tree_widget };

    info.section = Section::kPurchase;
    info.section_str = kPurchase;
    info.node = kPurchaseNode;
    info.path = kPurchasePath;
    info.entry = kPurchaseEntry;
    info.settlement = kPurchaseSettlement;

    info.rule_map.insert(true, "TO");
    info.rule_map.insert(false, "RO");

    info.unit_map.insert(std::to_underlying(UnitO::kIS), tr("IS"));
    info.unit_map.insert(std::to_underlying(UnitO::kMS), tr("MS"));
    info.unit_map.insert(std::to_underlying(UnitO::kPEND), tr("PEND"));

    info.kind_map.insert(kBranch, kBranchKind);
    info.kind_map.insert(kLeaf, kLeafKind);

    info.unit_model = MainWindowUtils::CreateModelFromMap(info.unit_map, this);
    info.unit_model->sort(0, Qt::DescendingOrder);

    info.rule_model = MainWindowUtils::CreateModelFromMap(info.rule_map, this);

    ReadSectionConfig(section_config, kPurchase);

    auto* entry_hub_o = new EntryHubO(info, this);
    auto* tree_model_o = new TreeModelO(info, local_config_.separator, global_config.default_unit, this);

    entry_hub = entry_hub_o;
    tree_model = tree_model_o;

    const QDate today = QDate::currentDate();
    const QDateTime start_dt(today, kStartTime);
    const QDateTime end_dt(today, kEndTime);

    WebSocket::Instance().RegisterTreeModel(kPurchase, tree_model);
    WebSocket::Instance().RegisterEntryHub(kPurchase, entry_hub);

    tree_widget = new TreeWidgetTO(kPurchase, tree_model, start_dt, end_dt, this);
    tree_view = tree_widget->View();

    connect(tree_model_o, &TreeModelO::SUpdateAmount, static_cast<TreeModelS*>(sc_s_.tree_model.data()), &TreeModelS::RUpdateAmount);
    connect(entry_hub_o, &EntryHubO::SSyncPrice, static_cast<EntryHubS*>(sc_s_.entry_hub.data()), &EntryHubS::RPriceSList);
}

void MainWindow::SetAction() const
{
    ui->actionInsertNode->setIcon(QIcon(":/solarized_dark/solarized_dark/insert.png"));
    ui->actionEditName->setIcon(QIcon(":/solarized_dark/solarized_dark/edit.png"));
    ui->actionRemove->setIcon(QIcon(":/solarized_dark/solarized_dark/remove.png"));
    ui->actionAbout->setIcon(QIcon(":/solarized_dark/solarized_dark/about.png"));
    ui->actionAppendNode->setIcon(QIcon(":/solarized_dark/solarized_dark/append.png"));
    ui->actionJump->setIcon(QIcon(":/solarized_dark/solarized_dark/jump.png"));
    ui->actionPreferences->setIcon(QIcon(":/solarized_dark/solarized_dark/settings.png"));
    ui->actionSearch->setIcon(QIcon(":/solarized_dark/solarized_dark/search.png"));
    ui->actionCheckAll->setIcon(QIcon(":/solarized_dark/solarized_dark/check-all.png"));
    ui->actionCheckNone->setIcon(QIcon(":/solarized_dark/solarized_dark/check-none.png"));
    ui->actionCheckReverse->setIcon(QIcon(":/solarized_dark/solarized_dark/check-reverse.png"));
    ui->actionAppendEntry->setIcon(QIcon(":/solarized_dark/solarized_dark/append_trans.png"));
    ui->actionStatement->setIcon(QIcon(":/solarized_dark/solarized_dark/statement.png"));
    ui->actionSettlement->setIcon(QIcon(":/solarized_dark/solarized_dark/settle.png"));
    ui->actionResetColor->setIcon(QIcon(":/solarized_dark/solarized_dark/reset_color.png"));

    ui->actionCheckAll->setProperty(kCheck, std::to_underlying(Check::kOn));
    ui->actionCheckNone->setProperty(kCheck, std::to_underlying(Check::kOff));
    ui->actionCheckReverse->setProperty(kCheck, std::to_underlying(Check::kFlip));
}

void MainWindow::SetTreeView(QTreeView* tree_view, CSectionInfo& info) const
{
    if (info.section == Section::kSale || info.section == Section::kPurchase)
        tree_view->setColumnHidden(std::to_underlying(NodeEnumO::kParty), kIsHidden);

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
    ResizeColumn(header, std::to_underlying(NodeEnum::kDescription));
    header->setStretchLastSection(false);
    header->setDefaultAlignment(Qt::AlignCenter);
}

void MainWindow::on_actionAppendNode_triggered()
{
    assert(sc_->tree_widget);

    auto view { sc_->tree_view };
    if (!WidgetUtils::HasSelection(view))
        return;

    const auto parent_index { view->currentIndex() };
    if (!parent_index.isValid())
        return;

    const int kind { parent_index.siblingAtColumn(std::to_underlying(NodeEnum::kKind)).data().toInt() };
    if (kind != kBranch)
        return;

    const auto parent_id { parent_index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() };
    InsertNodeFunction(parent_index, parent_id, 0);
}

void MainWindow::on_actionJump_triggered()
{
    if (start_ == Section::kSale || start_ == Section::kPurchase || start_ == Section::kStakeholder)
        return;

    auto* leaf_widget { dynamic_cast<LeafWidget*>(ui->tabWidget->currentWidget()) };
    if (!leaf_widget)
        return;

    auto* view { leaf_widget->View() };
    if (!WidgetUtils::HasSelection(view))
        return;

    const auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    const int row { index.row() };
    const int rhs_node_column { EntryUtils::RhsNodeColumn(start_) };

    const auto rhs_node_id { index.sibling(row, rhs_node_column).data().toUuid() };
    if (rhs_node_id.isNull())
        return;

    const auto entry_id { index.sibling(row, std::to_underlying(EntryEnum::kId)).data().toUuid() };
    CreateLeafWidget(rhs_node_id);
    SwitchToLeaf(rhs_node_id, entry_id);
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

    auto view { sc_->tree_view };
    if (!WidgetUtils::HasSelection(view))
        return;

    const auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    const auto node_id { index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() };

    auto model { sc_->tree_model };

    const auto& parent { index.parent() };
    const auto parent_id { parent.isValid() ? parent.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() : QUuid() };
    auto parent_path { model->Path(parent_id) };

    if (!parent_path.isEmpty())
        parent_path += local_config_.separator;

    CString name { model->Name(node_id) };
    const auto children_name { model->ChildrenName(parent_id) };

    auto* edit_name { new EditNodeName(name, parent_path, children_name, this) };
    connect(edit_name, &QDialog::accepted, this, [=]() { model->UpdateName(node_id, edit_name->GetName()); });
    edit_name->exec();
}

void MainWindow::InsertNodeFIST(Node* node, const QModelIndex& parent, const QUuid& parent_id, int row)
{
    auto tree_model { sc_->tree_model };
    auto unit_model { sc_->info.unit_model };

    auto parent_path { tree_model->Path(parent_id) };
    if (!parent_path.isEmpty())
        parent_path += local_config_.separator;

    QDialog* dialog {};

    const auto children_name { tree_model->ChildrenName(parent_id) };
    const auto arg { InsertNodeArgFIST { node, unit_model, parent_path, children_name } };

    switch (start_) {
    case Section::kFinance:
        dialog = new InsertNodeFinance(arg, this);
        break;
    case Section::kTask:
        dialog = new InsertNodeTask(arg, this);
        break;
    case Section::kStakeholder:
        dialog = new InsertNodeStakeholder(arg, this);
        break;
    case Section::kItem:
        dialog = new InsertNodeItem(arg, sc_->section_config.rate_decimal, this);
        break;
    default:
        return NodePool::Instance().Recycle(node, start_);
    }

    connect(dialog, &QDialog::accepted, this, [=, this]() {
        if (tree_model->InsertNode(row, parent, node)) {
            auto index = tree_model->index(row, 0, parent);
            sc_->tree_view->setCurrentIndex(index);
        }
    });

    connect(dialog, &QDialog::rejected, this, [=, this]() { NodePool::Instance().Recycle(node, start_); });
    dialog->exec();
}

void MainWindow::RLeafExternalReference(const QUuid& node_id, int unit)
{
    assert(sc_->tree_widget);
    assert(sc_->tree_model->Kind(node_id) == kLeaf
        && "Node kind should be 'kLeafNode' at this point. The kind check should be performed in the delegate DoubleSpinUnitRPS.");

    switch (start_) {
    case Section::kItem:
        LeafExternalReferenceI(node_id, unit);
        break;
    case Section::kStakeholder:
        LeafExternalReferenceS(node_id, unit);
        break;
    default:
        break;
    }
}

void MainWindow::LeafExternalReferenceI(const QUuid& node_id, int unit)
{
    if (unit != std::to_underlying(UnitI::kInternal) || start_ != Section::kItem)
        return;

    CreateLeafExternalReference(sc_->tree_model, sc_->info, node_id, unit);
}

void MainWindow::LeafExternalReferenceS(const QUuid& node_id, int unit)
{
    if (start_ != Section::kStakeholder)
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

        if (start_ == Section::kStakeholder)
            UpdateStakeholderReference(nodes, branch);

        if (!sc_->leaf_wgt_hash.contains(node_id))
            return;
    }

    for (int index = 0; index != count; ++index) {
        const auto node_id { tab_bar->tabData(index).value<TabInfo>().id };

        if (widget->isTabVisible(index) && nodes.contains(node_id)) {
            const auto path = model->Path(node_id);

            if (!branch) {
                tab_bar->setTabText(index, name);
            }

            tab_bar->setTabToolTip(index, path);
        }
    }

    if (start_ == Section::kStakeholder)
        UpdateStakeholderReference(nodes, branch);
}

void MainWindow::RUpdateConfig(const LocalConfig& local, const GlobalConfig& global, const SectionConfig& section)
{
    UpdateLocalConfig(local);
    UpdateSectionConfig(section);
    UpdateGlobalConfig(global);
}

void MainWindow::UpdateLocalConfig(CLocalConfig& local)
{
    if (local_config_ == local)
        return;

    auto new_separator { local.separator };
    auto old_separator { local_config_.separator };

    if (old_separator != new_separator) {
        sc_f_.tree_model->UpdateSeparator(old_separator, new_separator);
        sc_s_.tree_model->UpdateSeparator(old_separator, new_separator);
        sc_i_.tree_model->UpdateSeparator(old_separator, new_separator);
        sc_t_.tree_model->UpdateSeparator(old_separator, new_separator);

        auto* widget { ui->tabWidget };
        int count { ui->tabWidget->count() };

        for (int index = 0; index != count; ++index)
            widget->setTabToolTip(index, widget->tabToolTip(index).replace(old_separator, new_separator));
    }

    if (local_config_.language != local.language) {
        MainWindowUtils::Message(QMessageBox::Information, tr("Language Changed"),
            tr("The language has been changed. Please restart the application for the changes to take effect."), kThreeThousand);
    }

    local_config_ = local;

    local_settings_->beginGroup(kYTX);
    local_settings_->setValue(kLanguage, local.language);
    local_settings_->setValue(kSeparator, local.separator);
    local_settings_->setValue(kPrinter, local.printer);
    local_settings_->setValue(kTheme, local.theme);
    local_settings_->setValue(kName, local.company_name);

    local_settings_->endGroup();
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

    section_settings_->beginGroup(sc_->info.section_str);

    if (start_ == Section::kFinance || start_ == Section::kItem) {
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

        if (const auto* tree_widget = dynamic_cast<TreeWidget*>(current_widget)) {
            auto* header { sc_->tree_view->header() };
            ResizeColumn(header, std::to_underlying(NodeEnum::kDescription));
        }
    }
}

void MainWindow::UpdateGlobalConfig(CGlobalConfig& global)
{
    auto& current_global { sc_->global_config };
    if (current_global == global)
        return;

    if (current_global.document_dir != global.document_dir) {
        const auto message { JsonGen::UpdateDocumentDir(sc_->info.section_str, global.document_dir) };
        WebSocket::Instance().SendMessage(kDocumentDir, message);
        current_global.document_dir = global.document_dir;
    }

    if (current_global.default_unit != global.default_unit) {
        const auto message { JsonGen::UpdateDefaultUnit(sc_->info.section_str, global.default_unit) };
        WebSocket::Instance().SendMessage(kDefaultUnit, message);
    }
}

void MainWindow::UpdateAccountInfo(const QString& user, const QString& database)
{
    ui->actionEmail->setText(tr("Email") + ": " + user);
    ui->actionWorkspace->setText(tr("Workspace") + ": " + database);
    ui->actionLogin->setEnabled(false);
    ui->actionLogout->setEnabled(true);
}

void MainWindow::ClearAccountInfo()
{
    ui->actionEmail->setText(tr("Email"));
    ui->actionWorkspace->setText(tr("Workspace"));
    ui->actionLogin->setEnabled(true);
    ui->actionLogout->setEnabled(false);
}

void MainWindow::UpdateStakeholderReference(const QSet<QUuid>& stakeholder_nodes, bool branch) const
{
    auto* widget { ui->tabWidget };
    auto stakeholder_model { sc_->tree_model };
    auto* order_model { static_cast<TreeModelO*>(sc_sale_.tree_model.data()) };
    auto* tab_bar { widget->tabBar() };
    const int count { widget->count() };

    // 使用 QtConcurrent::run 启动后台线程
    auto future = QtConcurrent::run([=]() -> QVector<std::tuple<int, QString, QString>> {
        QVector<std::tuple<int, QString, QString>> updates;

        // 遍历所有选项卡，计算需要更新的项
        for (int index = 0; index != count; ++index) {
            const auto& data { tab_bar->tabData(index).value<TabInfo>() };
            bool update = data.section == Section::kSale || data.section == Section::kPurchase;

            if (!widget->isTabVisible(index) && update) {
                const auto order_node_id = data.id;
                if (order_node_id.isNull())
                    continue;

                const auto order_party = order_model->Party(order_node_id);
                if (!stakeholder_nodes.contains(order_party))
                    continue;

                QString name = stakeholder_model->Name(order_party);
                QString path = stakeholder_model->Path(order_party);

                // 收集需要更新的信息
                updates.append(std::make_tuple(index, name, path));
            }
        }

        return updates;
    });

    auto watcher = std::make_unique<QFutureWatcher<QVector<std::tuple<int, QString, QString>>>>();

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

void MainWindow::LoadAndInstallTranslator(CString& language)
{
    if (language == kEnUS)
        return;

    const QString ytx_language { QStringLiteral(":/I18N/I18N/ytx_%1.qm").arg(language) };
    if (ytx_translator_.load(ytx_language))
        qApp->installTranslator(&ytx_translator_);

    const QString qt_language { QStringLiteral(":/I18N/I18N/qt_%1.qm").arg(language) };
    if (qt_translator_.load(qt_language))
        qApp->installTranslator(&qt_translator_);

    if (language == kZhCN)
        QLocale::setDefault(QLocale(QLocale::Chinese, QLocale::China));
}

void MainWindow::ResizeColumn(QHeaderView* header, int stretch_column) const
{
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(stretch_column, QHeaderView::Stretch);
}

void MainWindow::ReadLocalConfig()
{
    local_settings_ = QSharedPointer<QSettings>::create(
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() + kYTX + kDotSuffixINI, QSettings::IniFormat);

    QString language_code { kEnUS };
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    switch (QLocale::system().language()) {
    case QLocale::Chinese:
        language_code = kZhCN;
        break;
    default:
        break;
    }

    local_settings_->beginGroup(kYTX);
    local_config_.language = local_settings_->value(kLanguage, language_code).toString();
    local_config_.theme = local_settings_->value(kTheme, kSolarizedDark).toString();
    local_config_.separator = local_settings_->value(kSeparator, kDash).toString();
    local_config_.printer = local_settings_->value(kPrinter, {}).toString();
    local_settings_->endGroup();

    LoginInfo::Instance().ReadConfig(local_settings_);
    WebSocket::Instance().ReadConfig(local_settings_);
    WebSocket::Instance().Connect();

    LoadAndInstallTranslator(local_config_.language);

    const QString theme { QStringLiteral("file:///:/theme/theme/%1.qss").arg(local_config_.theme) };
    qApp->setStyleSheet(theme);
    SetAppFontByDpi();
}

void MainWindow::ReadSectionConfig(SectionConfig& section, CString& section_name)
{
    section_settings_->beginGroup(section_name);

    if (section_name == kFinance || section_name == kItem) {
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
    case Section::kItem:
        node = new SearchNodeModelI(sc_->info, sc_->tree_model, this);
        entry = new SearchEntryModelI(sc_->info, this);
        dialog = new SearchDialogI(sc_->tree_model, node, entry, sc_->section_config, sc_->info, this);
        break;
    case Section::kTask:
        node = new SearchNodeModelT(sc_->info, sc_->tree_model, this);
        entry = new SearchEntryModelT(sc_->info, this);
        dialog = new SearchDialogT(sc_->tree_model, node, entry, sc_->section_config, sc_->info, this);
        break;
    case Section::kStakeholder:
        node = new SearchNodeModelS(sc_->info, sc_->tree_model, this);
        entry = new SearchEntryModelS(sc_->info, this);
        dialog = new SearchDialogS(sc_->tree_model, node, entry, sc_i_.tree_model, sc_->section_config, sc_->info, this);
        break;
    case Section::kSale:
    case Section::kPurchase:
        node = new SearchNodeModelO(sc_->info, sc_->tree_model, sc_s_.tree_model, this);
        entry = new SearchEntryModelO(sc_->info, this);
        dialog = new SearchDialogO(sc_->tree_model, node, entry, sc_i_.tree_model, sc_s_.tree_model, sc_->section_config, sc_->info, this);
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
        const auto message { JsonGen::TableData(sc_->info.section_str, id) };
        WebSocket::Instance().SendMessage(kLeafAcked, message);

        switch (start_) {
        case Section::kSale:
        case Section::kPurchase:
            sc_->tree_model->FetchOneNode(lhs_node_id);
            CreateLeafO(sc_->tree_model, sc_->leaf_wgt_hash, sc_->info, sc_->section_config, lhs_node_id);
            break;
        case Section::kTask:
            sc_->tree_model->FetchOneNode(lhs_node_id);
        case Section::kFinance:
        case Section::kItem:
        case Section::kStakeholder:
            CreateLeafFIST(sc_->tree_model, sc_->entry_hub, sc_->leaf_wgt_hash, sc_->info, sc_->section_config, id);
            break;
        default:
            break;
        }
    }

    SwitchToLeaf(id, entry_id);
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

void MainWindow::RSyncParty(const QUuid& node_id, int column, const QVariant& value)
{
    if (column != std::to_underlying(NodeEnumO::kParty))
        return;

    const auto party_id { value.toUuid() };

    auto model { sc_s_.tree_model };
    auto* widget { ui->tabWidget };
    auto* tab_bar { widget->tabBar() };
    int count { widget->count() };

    for (int index = 0; index != count; ++index) {
        if (widget->isTabVisible(index) && tab_bar->tabData(index).value<TabInfo>().id == node_id) {
            tab_bar->setTabText(index, model->Name(party_id));
            tab_bar->setTabToolTip(index, model->Path(party_id));
        }
    }
}

void MainWindow::on_actionPreferences_triggered()
{
    auto model { sc_->tree_model };

    auto* preference { new Preferences(model, sc_->info, local_config_, sc_->global_config, sc_->section_config, this) };
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

void MainWindow::RUpdateState()
{
    auto* leaf_widget { dynamic_cast<LeafWidget*>(ui->tabWidget->currentWidget()) };
    assert(leaf_widget);

    auto table_model { leaf_widget->Model() };
    table_model->CheckAction(Check { QObject::sender()->property(kCheck).toInt() });
}

void MainWindow::RConnectResult(bool result)
{
    if (!result) {
        ClearMainwindow();
    }

    ui->actionReconnect->setDisabled(result);
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
    const bool is_leaf_fist { IsLeafWidgetFIST(widget) };
    const bool is_leaf_order { IsLeafWidgetO(widget) };
    const bool is_order_section { start_ == Section::kSale || start_ == Section::kPurchase };
    const bool is_color_section { start_ == Section::kTask || start_ == Section::kItem };

    bool finished {};

    if (is_leaf_order) {
        const auto node_id { ui->tabWidget->tabBar()->tabData(index).value<TabInfo>().id };
        finished = sc_->tree_model->Finished(node_id);
    }

    ui->actionAppendNode->setEnabled(is_tree);
    ui->actionEditName->setEnabled(is_tree && !is_order_section);
    ui->actionResetColor->setEnabled(is_tree && is_color_section);

    ui->actionCheckAll->setEnabled(is_leaf_fist);
    ui->actionCheckNone->setEnabled(is_leaf_fist);
    ui->actionCheckReverse->setEnabled(is_leaf_fist);
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
    LoginInfo& login_info { LoginInfo::Instance() };
    CString& source { QString("%1@%2%3").arg(login_info.Email(), login_info.Workspace(), kDotSuffixCache) };

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
            const bool where { start_ == Section::kStakeholder ? false : true };
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
        dialog = new Login(local_settings_, this);
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
    if (start_ != Section::kItem && start_ != Section::kTask)
        return;

    assert(sc_->tree_widget);

    auto view { sc_->tree_view };
    if (!WidgetUtils::HasSelection(view))
        return;

    const auto index { view->currentIndex() };
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

void MainWindow::on_actionReconnect_triggered() { WebSocket::Instance().Connect(); }

void MainWindow::on_actionLogout_triggered()
{
    ClearMainwindow();
    ClearAccountInfo();
    WebSocket::Instance().Connect();
}
