#include "billing/settlement/settlementmodel.h"
#include "enum/settlementenum.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::on_actionSettlement_triggered()
{
    assert(IsOrderSection(start_));

    auto* model { new SettlementModel(sc_->info, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new SettlementWidget(model, start_, widget_id, this) };

    const int tab_index { ui->tabWidget->addTab(widget, tr("Settlement")) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, widget_id }));

    auto* view { widget->View() };
    SetSettlementView(view, std::to_underlying(SettlementEnum::kDescription));
    DelegateSettlement(view, sc_->section_config);

    RegisterWidget(widget_id, widget);
}

void MainWindow::RSettlementNode(Settlement* settlement, std::shared_ptr<SettlementNodeList>& list) { assert(IsOrderSection(start_)); }
