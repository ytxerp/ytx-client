#include "billing/settlement/settlementitemmodel.h"
#include "billing/settlement/settlementitemwidget.h"
#include "billing/settlement/settlementmodel.h"
#include "billing/settlement/settlementwidget.h"
#include "enum/settlementenum.h"
#include "global/resourcepool.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::on_actionSettlement_triggered()
{
    assert(IsOrderSection(start_));

    auto* model { new SettlementModel(sc_->info, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new SettlementWidget(model, start_, widget_id, this) };
    connect(widget, &SettlementWidget::SSettlementNode, this, &MainWindow::RSettlementNode);

    {
        const int tab_index { ui->tabWidget->addTab(widget, tr("Settlement")) };
        auto* tab_bar { ui->tabWidget->tabBar() };

        tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, widget_id }));
    }

    {
        auto* view { widget->View() };
        SetSettlementView(view, std::to_underlying(SettlementEnum::kDescription));
        DelegateSettlement(view, sc_->section_config);
    }

    RegisterWidget(widget_id, widget);
}

void MainWindow::RSettlementNode(const QUuid& parent_widget_id, Settlement* settlement, bool is_persisted)
{
    assert(IsOrderSection(start_));

    auto* model { new SettlementItemModel(sc_->info, SettlementStatus(settlement->status), this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new SettlementItemWidget(sc_p_.tree_model, model, settlement, is_persisted, start_, widget_id, parent_widget_id, this) };
    connect(model, &SettlementItemModel::SSyncAmount, widget, &SettlementItemWidget::RSyncAmount);

    {
        const int tab_index { ui->tabWidget->addTab(widget, tr("SettlementItem")) };
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

void MainWindow::RSettlementItemAcked(Section section, const QUuid& widget_id, const QJsonArray& entry_array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id, nullptr) };
    if (!widget)
        return;

    auto* d_widget { static_cast<SettlementItemWidget*>(widget.data()) };
    auto* model { d_widget->Model() };
    model->ResetModel(entry_array);
}

void MainWindow::RSettlementInserted(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QJsonObject meta { obj.value(kMeta).toObject() };
    const auto widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const auto parent_widget_id { QUuid(obj.value(kParentWidgetId).toString()) };

    auto* sc { GetSectionContex(section) };

    {
        auto widget { sc->widget_hash.value(widget_id, nullptr) };
        if (widget) {
            auto* d_widget { static_cast<SettlementItemWidget*>(widget.data()) };
            d_widget->ReleaseSucceeded();
        }
    }

    {
        auto parent_widget { sc->widget_hash.value(parent_widget_id, nullptr) };
        if (parent_widget) {
            auto* d_parent_widget { static_cast<SettlementWidget*>(parent_widget.data()) };
            auto* model { d_parent_widget->Model() };

            const QJsonObject settlement_obj { obj.value(kSettlement).toObject() };
            auto* settlement { ResourcePool<Settlement>::Instance().Allocate() };
            settlement->ReadJson(settlement_obj);

            model->InsertMeta(settlement, meta);
            model->InsertRow(settlement);
        }
    }
}

void MainWindow::RSettlement(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id, nullptr) };
    if (!widget)
        return;

    auto* d_widget { static_cast<SettlementWidget*>(widget.data()) };
    auto* model { d_widget->Model() };
    model->ResetModel(array);
}
