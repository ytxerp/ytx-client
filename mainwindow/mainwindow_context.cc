#include <QtCore/qdir.h>
#include <QtCore/qstandardpaths.h>
#include <QtWidgets/qheaderview.h>

#include "entryhub/entryhubf.h"
#include "entryhub/entryhubi.h"
#include "entryhub/entryhubt.h"
#include "global/logininfo.h"
#include "global/printhub.h"
#include "mainwindow.h"
#include "tree/model/treemodelf.h"
#include "tree/model/treemodelo.h"
#include "tree/model/treemodelp.h"
#include "tree/model/treemodelt.h"
#include "tree/widget/treewidgetf.h"
#include "tree/widget/treewidgetit.h"
#include "tree/widget/treewidgeto.h"
#include "tree/widget/treewidgetp.h"
#include "ui_mainwindow.h"
#include "utils/mainwindowutils.h"
#include "websocket/websocket.h"

bool MainWindow::RInitializeContext(const QString& expire_date)
{
    {
        LoginInfo& login_info { LoginInfo::Instance() };
        UpdateAccountInfo(login_info.Email(), login_info.Workspace(), expire_date);

        if (!section_settings_) {
            const QString ini_file { QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator()
                + MainWindowUtils::SectionFile(login_info.Email(), login_info.Workspace()) + kDotSuffixINI };

            section_settings_ = QSharedPointer<QSettings>::create(ini_file, QSettings::IniFormat);
        }
    }

    {
        InitContextFinance();
        InitContextTask();
        InitContextInventory();
        InitContextPartner();
        InitContextSale();
        InitContextPurchase();
    }

    {
        CreateSection(sc_f_, tr("Finance"));
        CreateSection(sc_p_, tr("Partner"));
        CreateSection(sc_i_, tr("Inventory"));
        CreateSection(sc_t_, tr("Task"));
        CreateSection(sc_sale_, tr("Sale"));
        CreateSection(sc_purchase_, tr("Purchase"));
    }

    {
        RSectionGroup(std::to_underlying(start_));
        SetAction(true);
        on_tabWidget_currentChanged(0);
    }

    {
        // Delay template and model setup until event loop starts (non-blocking post-init)
        QTimer::singleShot(0, this, [this]() {
            PrintHub::Instance().ScanTemplate();
            PrintHub::Instance().SetAppConfig(&app_config_);
            PrintHub::Instance().SetPartnerModel(sc_p_.tree_model);
            PrintHub::Instance().SetInventoryModel(sc_i_.tree_model);
        });
    }

    return true;
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

void MainWindow::UpdateAccountInfo(const QString& user, const QString& database, const QString& expire_date)
{
    ui->actionEmail->setText(tr("Email") + ": " + user);
    ui->actionWorkspace->setText(tr("Workspace") + ": " + database);
    ui->actionExpireDate->setText(tr("Expire Date") + ": " + expire_date);
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
        TreeConnectO(view, tree_model, entry_hub);
        break;
    default:
        break;
    }
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

    for (Currency c : {
             Currency::USD,
             Currency::EUR,
             Currency::JPY,
             Currency::GBP,
             Currency::AUD,
             Currency::CAD,
             Currency::CHF,
             Currency::CNY,
             Currency::HKD,
             Currency::NZD,
             Currency::SEK,
             Currency::NOK,
             Currency::SGD,
             Currency::KRW,
             Currency::MXN,
             Currency::INR,
         }) {
        const int key { std::to_underlying(c) };
        info.unit_map.insert(key, currency_code(c));
        info.unit_symbol_map.insert(key, currency_symbol(c));
    }

    info.rule_map.insert(Rule::kDDCI, Rule::kStrDDCI);
    info.rule_map.insert(Rule::kDICD, Rule::kStrDICD);

    info.kind_map.insert(std::to_underlying(NodeKind::kBranch), kBranchKind);
    info.kind_map.insert(std::to_underlying(NodeKind::kLeaf), kLeafKind);

    info.unit_model = TemplateUtils::CreateModelFromMap(info.unit_map, this);
    info.rule_model = TemplateUtils::CreateModelFromMap(info.rule_map, this);

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

    info.unit_model = TemplateUtils::CreateModelFromMap(info.unit_map, this);
    info.rule_model = TemplateUtils::CreateModelFromMap(info.rule_map, this);

    ReadSectionConfig(section_config, kInventory);

    entry_hub = new EntryHubI(info, this);
    tree_model = new TreeModelI(info, app_config_.separator, shared_config.default_unit, this);

    WebSocket::Instance()->RegisterTreeModel(Section::kInventory, tree_model);
    WebSocket::Instance()->RegisterEntryHub(Section::kInventory, entry_hub);

    tree_widget = new TreeWidgetIT(tree_model, section_config, this);
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

    info.unit_map.insert(std::to_underlying(UnitT::kAction), kUnitAction);
    info.unit_map.insert(std::to_underlying(UnitT::kTarget), kUnitTarget);
    info.unit_map.insert(std::to_underlying(UnitT::kSource), kUnitSource);

    info.rule_map.insert(Rule::kDDCI, Rule::kStrDDCI);
    info.rule_map.insert(Rule::kDICD, Rule::kStrDICD);

    info.kind_map.insert(std::to_underlying(NodeKind::kBranch), kBranchKind);
    info.kind_map.insert(std::to_underlying(NodeKind::kLeaf), kLeafKind);

    info.unit_model = TemplateUtils::CreateModelFromMap(info.unit_map, this);
    info.rule_model = TemplateUtils::CreateModelFromMap(info.rule_map, this);

    ReadSectionConfig(section_config, kTask);

    entry_hub = new EntryHubT(info, this);
    tree_model = new TreeModelT(info, app_config_.separator, shared_config.default_unit, this);

    WebSocket::Instance()->RegisterTreeModel(Section::kTask, tree_model);
    WebSocket::Instance()->RegisterEntryHub(Section::kTask, entry_hub);

    tree_widget = new TreeWidgetIT(tree_model, section_config, this);
    tree_view = tree_widget->View();

    connect(tree_model, &TreeModel::STotalsUpdated, tree_widget, &TreeWidget::RTotalsUpdated, Qt::UniqueConnection);
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

    info.unit_model = TemplateUtils::CreateModelFromMap(info.unit_map, this);

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

    info.unit_model = TemplateUtils::CreateModelFromMap(info.unit_map, this);
    info.unit_model->sort(0, Qt::DescendingOrder);

    info.rule_model = TemplateUtils::CreateModelFromMap(info.rule_map, this);

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

    tree_widget = new TreeWidgetO(Section::kSale, tree_model, start_dt, end_dt, this);
    tree_view = tree_widget->View();

    connect(tree_model_o, &TreeModelO::SUpdateAmount, static_cast<TreeModelP*>(sc_p_.tree_model.data()), &TreeModelP::RUpdateAmount);
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

    info.unit_model = TemplateUtils::CreateModelFromMap(info.unit_map, this);
    info.unit_model->sort(0, Qt::DescendingOrder);

    info.rule_model = TemplateUtils::CreateModelFromMap(info.rule_map, this);

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

    tree_widget = new TreeWidgetO(Section::kPurchase, tree_model, start_dt, end_dt, this);
    tree_view = tree_widget->View();

    connect(tree_model_o, &TreeModelO::SUpdateAmount, static_cast<TreeModelP*>(sc_p_.tree_model.data()), &TreeModelP::RUpdateAmount);
}
