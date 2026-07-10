#include "billing/settlement/settlementenum.h"
#include "billing/settlement/settlementprimarymodel.h"
#include "billing/settlement/settlementprimarywidget.h"
#include "billing/settlement/settlementsecondarymodel.h"
#include "billing/settlement/settlementsecondarywidget.h"
#include "global/resourcepool.h"
#include "mainwindow.h"
#include "utils/mainwindowutils.h"
#include "utils/templateutils.h"

void MainWindow::on_actionSettlement_triggered()
{
    qInfo() << Q_FUNC_INFO;

    Q_ASSERT(IsOrderSection(start_));

    auto* model { new settlement::PrimaryModel(header_info_.settlement, start_, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new SettlementPrimaryWidget(model, widget_id, start_, this) };

    {
        const int tab_index { sc_->tab_widget->addTab(widget, tr("Settlement")) };
        auto* tab_bar { sc_->tab_widget->tabBar() };

        tab_bar->setTabData(tab_index, widget_id);
    }

    {
        auto* view { widget->View() };

        connect(view, &QTableView::doubleClicked, this, &MainWindow::RSettlementTableViewDoubleClicked);
        InitTableView(view, std::to_underlying(settlement::PrimaryField::kId), std::to_underlying(settlement::PrimaryField::kVersion),
            std::to_underlying(settlement::PrimaryField::kDescription));
        DelegateSettlement(view, sc_->section_config);
    }

    RegisterWidget(widget, widget_id, WidgetRole::kSettlement);
}

void MainWindow::CreateSettlementSecondary(const QUuid& primary_widget_id, const settlement::PrimaryRow& primary_row)
{
    Q_ASSERT(IsOrderSection(start_));

    auto* model { new settlement::SecondaryModel(header_info_.settlement_item, primary_row.status, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new SettlementSecondaryWidget(sc_p_.tree_model, model, sc_->section_config, primary_row, widget_id, primary_widget_id, start_, this) };
    connect(model, &settlement::SecondaryModel::SSyncAmount, widget, &SettlementSecondaryWidget::RSyncAmount);
    connect(widget, &SettlementSecondaryWidget::SUpdatePartner, this, &MainWindow::RUpdatePartner);

    {
        const QString name { sc_p_.tree_model->Name(primary_row.partner_id) };
        const QString label { primary_row.sync_state == SyncState::kSynced ? QString("%1-%2").arg(tr("Settlement"), name) : tr("Settlement") };

        const int tab_index { sc_->tab_widget->addTab(widget, label) };
        auto* tab_bar { sc_->tab_widget->tabBar() };

        tab_bar->setTabData(tab_index, widget_id);
    }

    {
        auto* view { widget->View() };
        InitTableView(view, std::to_underlying(settlement::SecondaryField::kId), -1, std::to_underlying(settlement::SecondaryField::kDescription));
        DelegateSettlementNode(view, sc_->section_config);
    }

    RegisterWidget(widget, widget_id, WidgetRole::kSettlement);
}

void MainWindow::RUpdatePartner(const QUuid& widget_id, const QUuid& partner_id)
{
    auto widget { sc_->tab_widget };
    auto* tab_bar { widget->tabBar() };
    int count { widget->count() };

    for (int index = 0; index != count; ++index) {
        if (tab_bar->tabData(index).toUuid() == widget_id) {
            const QString name { sc_p_.tree_model->Name(partner_id) };
            const QString label { QString("%1-%2").arg(tr("Settlement"), name) };

            tab_bar->setTabText(index, label);
        }
    }
}

void MainWindow::RSettlementTableViewDoubleClicked(const QModelIndex& index)
{
    qInfo() << Q_FUNC_INFO;

    auto* current_widget { sc_->tab_widget->currentWidget() };

    Q_ASSERT(qobject_cast<SettlementPrimaryWidget*>(current_widget));
    auto* settlement_widget { static_cast<SettlementPrimaryWidget*>(current_widget) };

    if (index.column() != std::to_underlying(settlement::PrimaryField::kPartner))
        return;

    auto* settlement { static_cast<settlement::PrimaryRow*>(index.internalPointer()) };

    const QUuid settlement_widget_id { settlement_widget->WidgetId() };

    CreateSettlementSecondary(settlement_widget_id, *settlement);
}

void MainWindow::RAckSettlementItem(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* ptr { widget.data() };

    Q_ASSERT(qobject_cast<SettlementSecondaryWidget*>(ptr));
    auto* d_widget { static_cast<SettlementSecondaryWidget*>(ptr) };

    auto* model { d_widget->Model() };
    model->ResetModel(array);
}

void MainWindow::RInsertSettlement(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const auto widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const auto parent_widget_id { QUuid(obj.value(kParentWidgetId).toString()) };
    const QJsonObject settlement_obj { obj.value(kSettlement).toObject() };
    const int version { settlement_obj.value(kVersion).toInt() };

    auto* sc { GetSectionContex(section) };

    {
        auto widget { sc->widget_hash.value(widget_id).widget };
        if (widget) {
            auto* ptr { widget.data() };

            Q_ASSERT(qobject_cast<SettlementSecondaryWidget*>(ptr));
            auto* d_widget { static_cast<SettlementSecondaryWidget*>(ptr) };

            d_widget->InsertSucceeded(version);
        }
    }

    {
        auto parent_widget { sc->widget_hash.value(parent_widget_id).widget };
        if (parent_widget) {
            auto* ptr { parent_widget.data() };

            Q_ASSERT(qobject_cast<SettlementPrimaryWidget*>(ptr));
            auto* d_parent_widget { static_cast<SettlementPrimaryWidget*>(ptr) };

            auto* model { d_parent_widget->Model() };

            {
                auto* settlement { ResourcePool<settlement::PrimaryRow>::Instance().Allocate() };
                settlement->ReadJson(settlement_obj);

                model->InsertSucceeded(settlement);
            }

            {
                auto* view { d_parent_widget->View() };

                const int last_row { model->rowCount() - 1 };
                const QModelIndex last_index { model->index(last_row, std::to_underlying(settlement::PrimaryField::kPartner)) };

                view->setCurrentIndex(last_index);
                view->selectionModel()->select(last_index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
        }
    }
}

void MainWindow::RRecallSettlement(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const auto widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const auto parent_widget_id { QUuid(obj.value(kParentWidgetId).toString()) };
    const QUuid settlement_id { QUuid(obj.value(kSettlementId).toString()) };
    const QJsonObject settlement { obj.value(kSettlement).toObject() };
    const int version { settlement.value(kVersion).toInt() };

    auto* sc { GetSectionContex(section) };

    {
        auto widget { sc->widget_hash.value(widget_id).widget };
        if (widget) {
            auto* ptr { widget.data() };

            Q_ASSERT(qobject_cast<SettlementSecondaryWidget*>(ptr));
            auto* d_widget { static_cast<SettlementSecondaryWidget*>(ptr) };

            d_widget->RecallSucceeded(version);
        }
    }

    {
        auto parent_widget { sc->widget_hash.value(parent_widget_id).widget };
        if (parent_widget) {
            auto* ptr { parent_widget.data() };

            Q_ASSERT(qobject_cast<SettlementPrimaryWidget*>(ptr));
            auto* d_parent_widget { static_cast<SettlementPrimaryWidget*>(ptr) };

            auto* model { d_parent_widget->Model() };
            model->RecallSucceeded(settlement_id, settlement);
        }
    }
}

void MainWindow::RUpdateSettlement(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QJsonObject settlement { obj.value(kSettlement).toObject() };
    const auto widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const auto parent_widget_id { QUuid(obj.value(kParentWidgetId).toString()) };
    const QUuid settlement_id { QUuid(obj.value(kSettlementId).toString()) };
    const int version { settlement.value(kVersion).toInt() };

    auto* sc { GetSectionContex(section) };

    {
        auto widget { sc->widget_hash.value(widget_id).widget };
        if (widget) {
            auto* ptr { widget.data() };

            Q_ASSERT(qobject_cast<SettlementSecondaryWidget*>(ptr));
            auto* d_widget { static_cast<SettlementSecondaryWidget*>(ptr) };

            d_widget->UpdateSucceeded(version);
        }
    }

    {
        auto parent_widget { sc->widget_hash.value(parent_widget_id).widget };
        if (parent_widget) {
            auto* ptr { parent_widget.data() };

            Q_ASSERT(qobject_cast<SettlementPrimaryWidget*>(ptr));
            auto* d_parent_widget { static_cast<SettlementPrimaryWidget*>(ptr) };

            auto* model { d_parent_widget->Model() };
            model->UpdateSucceeded(settlement_id, settlement);
        }
    }
}

void MainWindow::RAckSettlement(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* ptr { widget.data() };
    Q_ASSERT(qobject_cast<SettlementPrimaryWidget*>(ptr));

    auto* d_widget { static_cast<SettlementPrimaryWidget*>(ptr) };

    auto* model { d_widget->Model() };
    model->ResetModel(array);
}

void MainWindow::DeleteSettlement(SettlementPrimaryWidget* widget)
{
    auto* view { widget->View() };
    Q_ASSERT(view != nullptr);

    if (!utils::HasSelection(view))
        return;

    const QModelIndex current_index { view->currentIndex() };
    if (!current_index.isValid())
        return;

    auto* settlement { static_cast<settlement::PrimaryRow*>(current_index.internalPointer()) };

    if (settlement->status == SettlementStatus::kReleased) {
        utils::ShowNotification(QMessageBox::Information, tr("Settlement Released"),
            tr("This settlement has already been released and cannot be deleted.\nYou need to recall it first before making changes."),
            time_const::kAutoCloseMs);
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
