#include "mainwindow.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QNetworkReply>
#include <QQueue>
#include <QScrollBar>
#include <QUrl>
#include <QtConcurrent>

#include "component/constant.h"
#include "component/signalblocker.h"
#include "component/stringinitializer.h"
#include "dialog/about.h"
#include "dialog/preferences.h"
#include "document.h"
#include "global/tablesstation.h"
#include "ui_mainwindow.h"
#include "utils/mainwindowutils.h"
#include "utils/templateutils.h"
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
    InitStatusLabel();

    SetTabWidget();
    SetIcon();
    SetUniqueConnection();
    SetRemoveShortcut();
    SetAction(false);

    StringInitializer::SetHeader(sc_f_.info, sc_i_.info, sc_t_.info, sc_p_.info, sc_sale_.info, sc_purchase_.info);

    Utils::SetConnectionStatus(connection_label_, ConnectionStatus::Connecting);
    Utils::SetLoginStatus(login_label_, LoginStatus::LoggedOut);
    Utils::ReadConfig(ui->splitter, &QSplitter::restoreState, app_settings_, kSplitter, kState);
    Utils::ReadConfig(this, &QMainWindow::restoreState, app_settings_, kMainwindow, kState, 0);
    Utils::ReadConfig(this, &QMainWindow::restoreGeometry, app_settings_, kMainwindow, kGeometry);

    QTimer::singleShot(0, this, &::MainWindow::InitilizeContext);
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

QStringList MainWindow::ChildrenName(const Node* node) const
{
    Q_ASSERT(node != nullptr);

    QStringList list {};

    if (!node || node->kind != NodeKind::kBranch || node->children.isEmpty())
        return list;

    list.reserve(node->children.size());

    for (const auto* child : std::as_const(node->children)) {
        Q_ASSERT(child != nullptr);

        list.emplaceBack(child->name);
    }

    return list;
}

QSet<QUuid> MainWindow::LeafChildrenId(const Node* node) const
{
    Q_ASSERT(node != nullptr);

    QSet<QUuid> set {};

    if (!node || node->kind != NodeKind::kBranch || node->children.isEmpty())
        return set;

    QQueue<const Node*> queue {};
    queue.enqueue(node);

    while (!queue.isEmpty()) {
        auto* queue_node = queue.dequeue();
        const NodeKind kind { queue_node->kind };

        switch (kind) {
        case NodeKind::kBranch:
            for (const auto* child : queue_node->children)
                queue.enqueue(child);
            break;
        case NodeKind::kLeaf:
            set.insert(queue_node->id);
            break;
        default:
            break;
        }
    }

    return set;
}

MainWindow::~MainWindow()
{
    WriteConfig();
    delete ui;
}

// FocusTableWidget - Activate and focus the leaf widget tab
// -------------------------------
// Used for:
//  • Node double-click → navigate to node's tab
//  • Entry jump → switch between different node tabs
//
// Note: Clears view selection after switching
void MainWindow::FocusTableWidget(const QUuid& node_id) const
{
    auto widget { sc_->table_wgt_hash.value(node_id, nullptr) };
    Q_ASSERT(widget != nullptr);

    ui->tabWidget->setCurrentWidget(widget);
    widget->activateWindow();

    widget->View()->setCurrentIndex(QModelIndex());
    widget->View()->clearSelection();
}

void MainWindow::InsertNodeFunction(const QModelIndex& parent_index)
{
    switch (start_) {
    case Section::kSale:
    case Section::kPurchase:
        InsertNodeO(parent_index);
        return;
    case Section::kFinance:
    case Section::kTask:
    case Section::kPartner:
    case Section::kInventory:
        InsertNodeFIPT(parent_index);
        return;
    }

    Q_UNREACHABLE();
}

void MainWindow::on_actionRemove_triggered()
{
    qInfo() << "[UI]" << "on_actionRemove_triggered";

    auto* widget { ui->tabWidget->currentWidget() };

    {
        auto* d_widget { qobject_cast<TreeWidgetSettlement*>(widget) };
        if (d_widget) {
            RemoveSettlement(d_widget);
            return;
        }
    }

    if (qobject_cast<TreeWidget*>(widget)) {
        RemoveNode();
        return;
    }

    if (auto* leaf_widget { qobject_cast<TableWidget*>(widget) }) {
        RemoveEntry(leaf_widget);
    }
}

void MainWindow::ResetMainwindow()
{
    section_settings_.clear();

    {
        auto* tab_widget { ui->tabWidget };
        auto* tab_bar { tab_widget->tabBar() };
        const int count { tab_widget->count() };

        for (int index = count - 1; index >= 0; --index) {
            const auto tab { tab_bar->tabData(index).value<TabInfo>() };
            if (!tab.id.isNull())
                tab_widget->removeTab(index);
        }
    }

    {
        Utils::ResetSectionContext(sc_f_);
        Utils::ResetSectionContext(sc_i_);
        Utils::ResetSectionContext(sc_t_);
        Utils::ResetSectionContext(sc_p_);
        Utils::ResetSectionContext(sc_sale_);
        Utils::ResetSectionContext(sc_purchase_);
    }

    {
        ui->actionEmail->setText(tr("Email"));
        ui->actionWorkspace->setText(tr("Workspace"));
        ui->actionExpireDate->setText(tr("Expire Date"));
    }

    {
        ui->actionReconnect->setEnabled(true);
        ui->actionSignIn->setEnabled(false);
        ui->actionSignOut->setEnabled(false);
    }
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
        RActionEntry(EntryAction(action_id));
    });
}

void MainWindow::RFreeWidget(Section section, const QUuid& node_id)
{
    auto* sc { GetSectionContex(section) };

    Utils::CloseWidget(node_id, sc->table_wgt_hash);
    TableSStation::Instance()->DeregisterModel(node_id);
}

void MainWindow::RegisterWidget(const QUuid& widget_id, QWidget* widget)
{
    sc_->widget_hash.insert(widget_id, widget);
    ui->tabWidget->setCurrentWidget(widget);
    widget->activateWindow();
}

void MainWindow::WriteConfig()
{
    if (app_settings_) {
        Utils::WriteConfig(ui->splitter, &QSplitter::saveState, app_settings_, kSplitter, kState);
        Utils::WriteConfig(this, &QMainWindow::saveState, app_settings_, kMainwindow, kState, 0);
        Utils::WriteConfig(this, &QMainWindow::saveGeometry, app_settings_, kMainwindow, kGeometry);
        Utils::WriteConfig(app_settings_, std::to_underlying(start_), kStart, kSection);
    }
}

SectionContext* MainWindow::GetSectionContex(Section section)
{
    switch (section) {
    case Section::kFinance:
        return &sc_f_;
    case Section::kPartner:
        return &sc_p_;
    case Section::kInventory:
        return &sc_i_;
    case Section::kTask:
        return &sc_t_;
    case Section::kSale:
        return &sc_sale_;
    case Section::kPurchase:
        return &sc_purchase_;
    }

    Q_UNREACHABLE();
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

void MainWindow::InitStatusLabel()
{
    login_label_ = new QLabel(this);
    ui->statusbar->addPermanentWidget(login_label_);

    connection_label_ = new QLabel(this);
    ui->statusbar->addPermanentWidget(connection_label_);
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

void MainWindow::SetUniqueConnection() const
{
    connect(ui->actionQuit, &QAction::triggered, qApp, &QCoreApplication::quit);

    connect(section_group_, &QButtonGroup::idClicked, this, &MainWindow::RSectionGroup);

    connect(WebSocket::Instance(), &WebSocket::SLeafRemoveDenied, this, &MainWindow::RLeafRemoveDenied);
    connect(WebSocket::Instance(), &WebSocket::SSharedConfig, this, &MainWindow::RSharedConfig);
    connect(WebSocket::Instance(), &WebSocket::SDefaultUnit, this, &MainWindow::RDefaultUnit);
    connect(WebSocket::Instance(), &WebSocket::SUpdateDefaultUnitFailed, this, &MainWindow::RUpdateDefaultUnitFailed);
    connect(WebSocket::Instance(), &WebSocket::SDocumentDir, this, &MainWindow::RDocumentDir);
    connect(WebSocket::Instance(), &WebSocket::SConnectionSucceeded, this, &MainWindow::RConnectionSucceeded);
    connect(WebSocket::Instance(), &WebSocket::SConnectionRefused, this, &MainWindow::RConnectionRefused);
    connect(WebSocket::Instance(), &WebSocket::SLoginSucceeded, this, &MainWindow::RLoginSucceeded);
    connect(WebSocket::Instance(), &WebSocket::SLoginFailed, this, &MainWindow::RLoginFailed);
    connect(WebSocket::Instance(), &WebSocket::SRemoteHostClosed, this, &MainWindow::RRemoteHostClosed);
    connect(WebSocket::Instance(), &WebSocket::SSelectLeafEntry, this, &MainWindow::RSelectLeafEntry);
    connect(WebSocket::Instance(), &WebSocket::SSaleReference, this, &MainWindow::RSaleReference);
    connect(WebSocket::Instance(), &WebSocket::SStatement, this, &MainWindow::RStatement);
    connect(WebSocket::Instance(), &WebSocket::SStatementNodeAcked, this, &MainWindow::RStatementNodeAcked);
    connect(WebSocket::Instance(), &WebSocket::SStatementEntryAcked, this, &MainWindow::RStatementEntryAcked);
    connect(WebSocket::Instance(), &WebSocket::SSettlement, this, &MainWindow::RSettlement);
    connect(WebSocket::Instance(), &WebSocket::SSettlementItemAcked, this, &MainWindow::RSettlementItemAcked);
    connect(WebSocket::Instance(), &WebSocket::SSettlementInserted, this, &MainWindow::RSettlementInserted);
    connect(WebSocket::Instance(), &WebSocket::SSettlementRecalled, this, &MainWindow::RSettlementRecalled);
    connect(WebSocket::Instance(), &WebSocket::SSettlementUpdated, this, &MainWindow::RSettlementUpdated);
    connect(WebSocket::Instance(), &WebSocket::SOrderReleased, this, &MainWindow::ROrderReleased);
    connect(WebSocket::Instance(), &WebSocket::SOrderRecalled, this, &MainWindow::ROrderRecalled);
    connect(WebSocket::Instance(), &WebSocket::SOrderSaved, this, &MainWindow::ROrderSaved);
    connect(WebSocket::Instance(), &WebSocket::SInvalidOperation, this, &MainWindow::RInvalidOperation);
    connect(WebSocket::Instance(), &WebSocket::SNodeSelected, this, &MainWindow::RNodeSelected);
    connect(WebSocket::Instance(), &WebSocket::SNodeLocation, this, &MainWindow::RNodeLocation);
    connect(WebSocket::Instance(), &WebSocket::STreeSyncFinished, this, &MainWindow::RTreeSyncFinished);
}

void MainWindow::SetIcon() const
{
    ui->actionInsertNode->setIcon(QIcon(":/solarized_dark/solarized_dark/insert.png"));
    ui->actionEditName->setIcon(QIcon(":/solarized_dark/solarized_dark/edit.png"));
    ui->actionRemove->setIcon(QIcon(":/solarized_dark/solarized_dark/remove.png"));
    ui->actionAbout->setIcon(QIcon(":/solarized_dark/solarized_dark/about.png"));
    ui->actionCheckforUpdates->setIcon(QIcon(":/solarized_dark/solarized_dark/update.png"));
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
    ui->actionQuit->setIcon(QIcon(":/solarized_dark/solarized_dark/quit.png"));
}

void MainWindow::on_actionInsertNode_triggered()
{
    qInfo() << "[UI]" << "on_actionInsertNode_triggered";

    auto* widget { ui->tabWidget->currentWidget() };
    if (!IsTreeWidget(widget) && !IsTableWidgetO(widget)) {
        return;
    }

    auto current_index { sc_->tree_view->currentIndex() };
    auto parent_index { current_index.parent() };

    InsertNodeFunction(parent_index);
}

void MainWindow::on_actionAppendNode_triggered()
{
    qInfo() << "[UI]" << "on_actionAppendNode_triggered";

    auto* widget { ui->tabWidget->currentWidget() };
    if (!IsTreeWidget(widget)) {
        return;
    }

    auto index { sc_->tree_view->currentIndex() };
    if (!index.isValid())
        return;

    const int kind_column { Utils::KindColumn(start_) };
    const QModelIndex kind_index { index.siblingAtColumn(kind_column) };

    // Check if the sibling index is valid
    if (!kind_index.isValid()) {
        qWarning() << "Invalid kind column:" << kind_column;
        return;
    }

    const int kind { kind_index.data().toInt() };
    if (kind != std::to_underlying(NodeKind::kBranch)) {
        return;
    }

    InsertNodeFunction(index);
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
    qInfo() << "[UI]" << "on_actionEditName_triggered";

    switch (start_) {
    case Section::kSale:
    case Section::kPurchase:
        EditNameO();
        return;
    case Section::kFinance:
    case Section::kTask:
    case Section::kPartner:
    case Section::kInventory:
        EditNameFIPT();
        return;
    }

    Q_UNREACHABLE();
}

void MainWindow::RUpdateConfig(const AppConfig& app, const SharedConfig& shared, const SectionConfig& section)
{
    UpdateAppConfig(app);
    UpdateSectionConfig(section);
    UpdateSharedConfig(shared);
}

void MainWindow::ResizeColumn(QHeaderView* header, int stretch_column) const
{
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(stretch_column, QHeaderView::Stretch);
}

void MainWindow::on_actionPreferences_triggered()
{
    qInfo() << "[UI]" << "on_actionPreferences_triggered";

    auto model { sc_->tree_model };

    auto* preference { new Preferences(model, sc_->info, app_config_, sc_->shared_config, sc_->section_config, this) };
    connect(preference, &Preferences::SUpdateConfig, this, &MainWindow::RUpdateConfig);
    preference->exec();
}

void MainWindow::on_actionAbout_triggered()
{
    qInfo() << "[UI]" << "on_actionAbout_triggered";

    static About* dialog { nullptr };

    if (!dialog) {
        dialog = new About(this);
        dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(dialog, &QDialog::destroyed, this, [=]() { dialog = nullptr; });
    }

    dialog->show();
    dialog->activateWindow();
}

void MainWindow::RActionEntry(EntryAction action)
{
    auto* current_widget { ui->tabWidget->currentWidget() };

    Q_ASSERT(qobject_cast<TableWidget*>(current_widget));
    auto* leaf_widget { static_cast<TableWidget*>(current_widget) };

    auto table_model { leaf_widget->Model() };
    table_model->ActionEntry(action);
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

    Utils::SwitchDialog(sc_, true);
}

void MainWindow::RSectionGroup(int id)
{
    qInfo() << "[UI]" << "Switched to section:" << kSectionString.value(Section(id));

    const Section section { id };
    start_ = section;

    Utils::SwitchDialog(sc_, false);
    UpdateLastTab();

    switch (section) {
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

void MainWindow::UpdateLastTab() const
{
    if (!sc_)
        return;

    auto index { ui->tabWidget->currentIndex() };
    sc_->info.last_tab_id = ui->tabWidget->tabBar()->tabData(index).value<TabInfo>().id;
}

void MainWindow::on_actionExportExcel_triggered()
{
    CString& source {};

    QString destination { QFileDialog::getSaveFileName(this, tr("Export Excel"), QDir::homePath(), QStringLiteral("*.xlsx")) };
    if (!Utils::PrepareNewFile(destination, kDotSuffixXLSX))
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
            Utils::ExportExcel(sc_->info.node, book1->GetCurrentWorksheet());

            book1->AppendSheet(sc_->info.path);
            book1->GetCurrentWorksheet()->WriteRow(1, 1, list);
            Utils::ExportExcel(sc_->info.path, book1->GetCurrentWorksheet(), false);

            book1->AppendSheet(sc_->info.entry);
            book1->GetCurrentWorksheet()->WriteRow(1, 1, sc_->info.full_entry_header);
            const bool where { start_ != Section::kPartner };
            Utils::ExportExcel(sc_->info.entry, book1->GetCurrentWorksheet(), where);

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
            Utils::Message(QMessageBox::Information, tr("Export Completed"), tr("Export completed successfully."), kThreeThousand);
        } else {
            QFile::remove(destination);
            Utils::Message(QMessageBox::Critical, tr("Export Failed"), tr("Export failed. The file has been deleted."), kThreeThousand);
        }
    });

    watcher->setFuture(future);
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

        if (Utils::CompareVersion(latest_tag, QCoreApplication::applicationVersion()) > 0) {
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
