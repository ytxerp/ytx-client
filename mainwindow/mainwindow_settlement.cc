#include "billing/settlement/tablemodelsettlement.h"
#include "billing/settlement/tablewidgetsettlement.h"
#include "billing/settlement/treemodelsettlement.h"
#include "billing/settlement/treewidgetsettlement.h"
#include "enum/settlementenum.h"
#include "global/resourcepool.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils/mainwindowutils.h"
#include "utils/templateutils.h"

void MainWindow::on_actionSettlement_triggered()
{
    qInfo() << "[UI]" << "on_actionSettlement_triggered";

    Q_ASSERT(IsOrderSection(start_));

    auto* model { new TreeModelSettlement(sc_->info, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new TreeWidgetSettlement(model, widget_id, start_, this) };

    {
        const int tab_index { ui->tabWidget->addTab(widget, tr("Settlement")) };
        auto* tab_bar { ui->tabWidget->tabBar() };

        tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, widget_id }));
    }

    {
        auto* view { widget->View() };

        connect(view, &QTableView::doubleClicked, this, &MainWindow::RSettlementTableViewDoubleClicked);
        SetSettlementView(view, std::to_underlying(SettlementEnum::kDescription));
        DelegateSettlement(view, sc_->section_config);
    }

    RegisterWidget(widget_id, widget);
}

void MainWindow::SettlementItemTab(const QUuid& parent_widget_id, const Settlement& settlement, SyncState sync_state)
{
    Q_ASSERT(IsOrderSection(start_));

    auto* model { new TableModelSettlement(sc_->info, SettlementStatus(settlement.status), this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new TableWidgetSettlement(sc_p_.tree_model, model, sc_->section_config, settlement, widget_id, parent_widget_id, start_, sync_state, this) };
    connect(model, &TableModelSettlement::SSyncAmount, widget, &TableWidgetSettlement::RSyncAmount);
    connect(widget, &TableWidgetSettlement::SUpdatePartner, this, &MainWindow::RUpdatePartner);

    {
        const QString name { sc_p_.tree_model->Name(settlement.partner_id) };
        const QString label { sync_state == SyncState::kSynced ? QString("%1-%2").arg(tr("Settlement"), name) : tr("Settlement") };

        const int tab_index { ui->tabWidget->addTab(widget, label) };
        auto* tab_bar { ui->tabWidget->tabBar() };

        tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, widget_id }));
    }

    {
        auto* view { widget->View() };
        SetSettlementItemView(view, std::to_underlying(SettlementItemEnum::kDescription));
        DelegateSettlementNode(view, sc_->section_config);
    }

    RegisterWidget(widget_id, widget);
}

void MainWindow::RUpdatePartner(const QUuid& widget_id, const QUuid& partner_id)
{
    auto model { sc_p_.tree_model };
    auto* widget { ui->tabWidget };
    auto* tab_bar { widget->tabBar() };
    int count { widget->count() };

    for (int index = 0; index != count; ++index) {
        if (widget->isTabVisible(index) && tab_bar->tabData(index).value<TabInfo>().id == widget_id) {
            const QString name { model->Name(partner_id) };
            const QString label { QString("%1-%2").arg(tr("Settlement"), name) };

            tab_bar->setTabText(index, label);
        }
    }
}

void MainWindow::RSettlementTableViewDoubleClicked(const QModelIndex& index)
{
    qInfo() << "[UI]" << "RSettlementTableViewDoubleClicked";

    auto* current_widget { ui->tabWidget->currentWidget() };

    Q_ASSERT(qobject_cast<TreeWidgetSettlement*>(current_widget));
    auto* settlement_widget { static_cast<TreeWidgetSettlement*>(current_widget) };

    if (index.column() != std::to_underlying(SettlementEnum::kAmount))
        return;

    auto* settlement { static_cast<Settlement*>(index.internalPointer()) };

    const QUuid settlement_widget_id { settlement_widget->WidgetId() };

    SettlementItemTab(settlement_widget_id, *settlement, SyncState::kSynced);
}

void MainWindow::RSettlementItemAcked(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id, nullptr) };
    if (!widget)
        return;

    auto* ptr { widget.data() };

    Q_ASSERT(qobject_cast<TableWidgetSettlement*>(ptr));
    auto* d_widget { static_cast<TableWidgetSettlement*>(ptr) };

    auto* model { d_widget->Model() };
    model->ResetModel(array);
}

void MainWindow::RSettlementInserted(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QJsonObject meta { obj.value(kMeta).toObject() };
    const auto widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const auto parent_widget_id { QUuid(obj.value(kParentWidgetId).toString()) };
    const QJsonObject settlement_obj { obj.value(kSettlement).toObject() };
    const int version { settlement_obj.value(kVersion).toInt() };

    auto* sc { GetSectionContex(section) };

    {
        auto widget { sc->widget_hash.value(widget_id, nullptr) };
        if (widget) {
            auto* ptr { widget.data() };

            Q_ASSERT(qobject_cast<TableWidgetSettlement*>(ptr));
            auto* d_widget { static_cast<TableWidgetSettlement*>(ptr) };

            d_widget->InsertSucceeded(version);
        }
    }

    {
        auto parent_widget { sc->widget_hash.value(parent_widget_id, nullptr) };
        if (parent_widget) {
            auto* ptr { parent_widget.data() };

            Q_ASSERT(qobject_cast<TreeWidgetSettlement*>(ptr));
            auto* d_parent_widget { static_cast<TreeWidgetSettlement*>(ptr) };

            auto* model { d_parent_widget->Model() };

            {
                auto* settlement { ResourcePool<Settlement>::Instance().Allocate() };
                settlement->ReadJson(settlement_obj);

                model->InsertSucceeded(settlement, meta);
            }

            {
                auto* view { d_parent_widget->View() };

                const int last_row { model->rowCount() - 1 };
                const QModelIndex last_index { model->index(last_row, std::to_underlying(SettlementEnum::kPartner)) };

                view->setCurrentIndex(last_index);
                view->selectionModel()->select(last_index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
        }
    }
}

void MainWindow::RSettlementRecalled(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QJsonObject meta { obj.value(kMeta).toObject() };
    const auto widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const auto parent_widget_id { QUuid(obj.value(kParentWidgetId).toString()) };
    const QUuid settlement_id { QUuid(obj.value(kSettlementId).toString()) };
    const QJsonObject settlement { obj.value(kSettlement).toObject() };
    const int version { settlement.value(kVersion).toInt() };

    auto* sc { GetSectionContex(section) };

    {
        auto widget { sc->widget_hash.value(widget_id, nullptr) };
        if (widget) {
            auto* ptr { widget.data() };

            Q_ASSERT(qobject_cast<TableWidgetSettlement*>(ptr));
            auto* d_widget { static_cast<TableWidgetSettlement*>(ptr) };

            d_widget->RecallSucceeded(version);
        }
    }

    {
        auto parent_widget { sc->widget_hash.value(parent_widget_id, nullptr) };
        if (parent_widget) {
            auto* ptr { parent_widget.data() };

            Q_ASSERT(qobject_cast<TreeWidgetSettlement*>(ptr));
            auto* d_parent_widget { static_cast<TreeWidgetSettlement*>(ptr) };

            auto* model { d_parent_widget->Model() };
            model->RecallSucceeded(settlement_id, settlement, meta);
        }
    }
}

void MainWindow::RSettlementUpdated(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QJsonObject meta { obj.value(kMeta).toObject() };
    const QJsonObject settlement { obj.value(kSettlement).toObject() };
    const auto widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const auto parent_widget_id { QUuid(obj.value(kParentWidgetId).toString()) };
    const QUuid settlement_id { QUuid(obj.value(kSettlementId).toString()) };
    const int version { settlement.value(kVersion).toInt() };

    auto* sc { GetSectionContex(section) };

    {
        auto widget { sc->widget_hash.value(widget_id, nullptr) };
        if (widget) {
            auto* ptr { widget.data() };

            Q_ASSERT(qobject_cast<TableWidgetSettlement*>(ptr));
            auto* d_widget { static_cast<TableWidgetSettlement*>(ptr) };

            d_widget->UpdateSucceeded(version);
        }
    }

    {
        auto parent_widget { sc->widget_hash.value(parent_widget_id, nullptr) };
        if (parent_widget) {
            auto* ptr { parent_widget.data() };

            Q_ASSERT(qobject_cast<TreeWidgetSettlement*>(ptr));
            auto* d_parent_widget { static_cast<TreeWidgetSettlement*>(ptr) };

            auto* model { d_parent_widget->Model() };
            model->UpdateSucceeded(settlement_id, settlement, meta);
        }
    }
}

void MainWindow::RSettlement(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id, nullptr) };
    if (!widget)
        return;

    auto* ptr { widget.data() };
    Q_ASSERT(qobject_cast<TreeWidgetSettlement*>(ptr));

    auto* d_widget { static_cast<TreeWidgetSettlement*>(ptr) };

    auto* model { d_widget->Model() };
    model->ResetModel(array);
}

void MainWindow::DeleteSettlement(TreeWidgetSettlement* widget)
{
    auto* view { widget->View() };
    Q_ASSERT(view != nullptr);

    if (!Utils::HasSelection(view))
        return;

    const QModelIndex current_index { view->currentIndex() };
    if (!current_index.isValid())
        return;

    auto* settlement { static_cast<Settlement*>(current_index.internalPointer()) };

    if (settlement->status == SettlementStatus::kSettled) {
        Utils::ShowNotification(QMessageBox::Information, tr("Settlement Released"),
            tr("This settlement has already been released and cannot be deleted.\nYou need to recall it first before making changes."), kThreeThousand);
        return;
    }

    auto* model { widget->Model() };
    Q_ASSERT(model != nullptr);

    const int current_row { current_index.row() };
    if (!model->removeRows(current_row, 1)) {
        qDebug() << "Failed to remove row:" << current_row;
        return;
    }

    const int new_row_count { model->rowCount() };
    if (new_row_count == 0)
        return;

    QModelIndex new_index {};
    if (current_row < new_row_count) {
        new_index = model->index(current_row, 0);
    } else {
        new_index = model->index(new_row_count - 1, 0);
    }

    if (new_index.isValid()) {
        view->setCurrentIndex(new_index);
        view->selectionModel()->select(new_index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        view->closePersistentEditor(new_index);
    }
}
