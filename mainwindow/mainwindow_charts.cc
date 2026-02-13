#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::on_actionEntryJournal_triggered()
{
    qInfo() << "[UI]" << "on_actionEntryJournal_triggered";

    Q_ASSERT(IsOrderSection(start_));

    auto* model { new TreeModelSettlement(sc_->info, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new TreeWidgetSettlement(model, widget_id, start_, this) };

    {
        const int tab_index { ui->tabWidget->addTab(widget, tr("Entry Journal")) };
        auto* tab_bar { ui->tabWidget->tabBar() };

        tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, widget_id }));
    }

    {
        auto* view { widget->View() };

        connect(view, &QTableView::doubleClicked, this, &MainWindow::RSettlementTableViewDoubleClicked);
        SetSettlementView(view, std::to_underlying(SettlementEnum::kDescription));
        DelegateSettlement(view, sc_->section_config);
    }

    RegisterWidget(widget, widget_id, WidgetRole::kCharts);
}
