#include "finance/settlement/settlementenum.h"
#include "finance/settlement/settlementprimarymodel.h"
#include "finance/settlement/settlementprimarywidget.h"
#include "finance/settlement/settlementsecondarymodel.h"
#include "finance/settlement/settlementsecondarywidget.h"
#include "mainwindow.h"

void MainWindow::on_actionSettlement_triggered()
{
    qInfo() << Q_FUNC_INFO;

    Q_ASSERT(IsOrderSection(start_));

    auto* model { new settlement::PrimaryModel(header_info_.settlement_primary, start_, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new SettlementPrimaryWidget(model, widget_id, start_, this) };

    {
        const int tab_index { sc_->tab_widget->addTab(widget, tr("Settlement")) };
        auto* tab_bar { sc_->tab_widget->tabBar() };

        tab_bar->setTabData(tab_index, widget_id);
    }

    {
        auto* view { widget->View() };

        connect(view, &QTableView::doubleClicked, this, &MainWindow::ROpenSettlementSecondaryWidget);
        connect(widget, &SettlementPrimaryWidget::SCreateSettlementSecondaryWidget, this, &MainWindow::RCreateSettlementSecondaryWidget);

        InitTableView(view, std::to_underlying(settlement::PrimaryField::kDescription));
        DelegateSettlement(view, sc_->section_config);
    }

    RegisterWidget(widget, widget_id, WidgetRole::kSettlement);
}

void MainWindow::CreateSettlementSecondary(const settlement::PrimaryRow& primary_row, settlement::PrimaryModel* primary_model)
{
    Q_ASSERT(IsOrderSection(start_));

    auto* model { new settlement::SecondaryModel(header_info_.settlement_secondary, primary_row.status, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new SettlementSecondaryWidget(sc_p_.tree_model, model, sc_->section_config, primary_row, widget_id, start_, this) };
    connect(model, &settlement::SecondaryModel::SSyncAmount, widget, &SettlementSecondaryWidget::RSyncAmount);
    connect(widget, &SettlementSecondaryWidget::SUpdatePartner, this, &MainWindow::RUpdatePartner);
    connect(widget, &SettlementSecondaryWidget::SInsertPrimaryRow, primary_model, &settlement::PrimaryModel::RInsertePrimaryRow);
    connect(widget, &SettlementSecondaryWidget::SUpdatePrimaryRow, primary_model, &settlement::PrimaryModel::RUpdatePrimaryRow);

    {
        const QString name { sc_p_.tree_model->Name(primary_row.partner_id) };
        const QString label { primary_row.sync_state == SyncState::kSynced ? QString("%1-%2").arg(tr("Settlement"), name) : tr("Settlement") };

        const int tab_index { sc_->tab_widget->addTab(widget, label) };
        auto* tab_bar { sc_->tab_widget->tabBar() };

        tab_bar->setTabData(tab_index, widget_id);
    }

    {
        auto* view { widget->View() };
        InitTableView(view, std::to_underlying(settlement::SecondaryField::kDescription));
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

void MainWindow::ROpenSettlementSecondaryWidget(const QModelIndex& index)
{
    qInfo() << Q_FUNC_INFO;

    auto* current_widget { sc_->tab_widget->currentWidget() };

    Q_ASSERT(qobject_cast<SettlementPrimaryWidget*>(current_widget));
    auto* settlement_widget { static_cast<SettlementPrimaryWidget*>(current_widget) };

    if (index.column() != std::to_underlying(settlement::PrimaryField::kPartner))
        return;

    auto* settlement { static_cast<settlement::PrimaryRow*>(index.internalPointer()) };
    auto* primary_model { settlement_widget->Model() };

    CreateSettlementSecondary(*settlement, primary_model);
}

void MainWindow::RCreateSettlementSecondaryWidget(settlement::PrimaryModel* model)
{
    settlement::PrimaryRow settlement {};

    settlement.issued_time = QDateTime::currentDateTime();
    settlement.id = QUuid::createUuidV7();

    CreateSettlementSecondary(settlement, model);
}

void MainWindow::RSettlementSecondary(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* ptr { widget.data() };

    Q_ASSERT(qobject_cast<SettlementSecondaryWidget*>(ptr));
    auto* d_widget { static_cast<SettlementSecondaryWidget*>(ptr) };

    auto* model { d_widget->Model() };
    model->Rebuild(array);
}

void MainWindow::RInsertSettlement(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const auto widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const QJsonObject settlement_obj { obj.value(kSettlement).toObject() };
    const int version { settlement_obj.value(kVersion).toInt() };

    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (widget) {
        auto* ptr { widget.data() };

        Q_ASSERT(qobject_cast<SettlementSecondaryWidget*>(ptr));
        auto* d_widget { static_cast<SettlementSecondaryWidget*>(ptr) };

        d_widget->InsertSucceeded(version);
    }
}

void MainWindow::RRecallSettlement(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const auto widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const QJsonObject update { obj.value(kUpdate).toObject() };
    const int version { obj.value(kVersion).toInt() };

    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (widget) {
        auto* ptr { widget.data() };

        Q_ASSERT(qobject_cast<SettlementSecondaryWidget*>(ptr));
        auto* d_widget { static_cast<SettlementSecondaryWidget*>(ptr) };

        d_widget->RecallSucceeded(version);
    }
}

void MainWindow::RUpdateSettlement(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QJsonObject settlement { obj.value(kSettlement).toObject() };
    const auto widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const QJsonObject update { obj.value(kUpdate).toObject() };
    const int version { obj.value(kVersion).toInt() };

    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (widget) {
        auto* ptr { widget.data() };

        Q_ASSERT(qobject_cast<SettlementSecondaryWidget*>(ptr));
        auto* d_widget { static_cast<SettlementSecondaryWidget*>(ptr) };

        d_widget->UpdateSucceeded(version);
    }
}

void MainWindow::RSettlementPrimary(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* ptr { widget.data() };
    Q_ASSERT(qobject_cast<SettlementPrimaryWidget*>(ptr));

    auto* d_widget { static_cast<SettlementPrimaryWidget*>(ptr) };

    auto* model { d_widget->Model() };
    model->Rebuild(array);
}
