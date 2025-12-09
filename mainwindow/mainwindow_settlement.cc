#include "billing/settlement/settlementmodel.h"
#include "billing/settlement/settlementnodemodel.h"
#include "billing/settlement/settlementnodewidget.h"
#include "enum/settlementenum.h"
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

void MainWindow::RSettlementNode(const std::shared_ptr<Settlement>& settlement, bool is_persisted, std::shared_ptr<SettlementNodeList>& list_cache)
{
    assert(IsOrderSection(start_));

    auto* model { new SettlementNodeModel(sc_->info, settlement->partner, settlement->id, list_cache, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new SettlementNodeWidget(sc_p_.tree_model, model, settlement, is_persisted, start_, widget_id, this) };

    {
        const int tab_index { ui->tabWidget->addTab(widget, tr("SettlementNode")) };
        auto* tab_bar { ui->tabWidget->tabBar() };

        tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, widget_id }));
    }

    RegisterWidget(widget_id, widget);
}

void MainWindow::RSettlement(Section section, const QUuid& widget_id, const QJsonArray& entry_array, const QJsonArray& unsettled_order)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id, nullptr) };
    if (!widget)
        return;

    auto* d_widget { static_cast<SettlementWidget*>(widget.data()) };
    auto* model { d_widget->Model() };
    model->ResetModel(entry_array);
    d_widget->ResetUnsettledOrder(unsettled_order);
}
