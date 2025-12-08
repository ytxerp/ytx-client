#include "billing/settlement/settlementmodel.h"
#include "enum/statementenum.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::on_actionSettlement_triggered()
{
    assert(IsOrderSection(start_));

    if (settlement_widget_) {
        ui->tabWidget->setCurrentWidget(settlement_widget_);
        settlement_widget_->activateWindow();
        return;
    }

    const auto start { QDateTime(QDate(QDate::currentDate().year() - 1, 1, 1), kStartTime) };
    const auto end { QDateTime(QDate(QDate::currentDate().year() + 1, 1, 1), kStartTime) };

    auto* model { new SettlementModel(sc_->entry_hub, sc_->info, this) };
    model->ResetModel(start, end);

    settlement_widget_ = new SettlementWidget(model, start, end, this);

    const int tab_index { ui->tabWidget->addTab(settlement_widget_, tr("Settlement")) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    const QUuid report_id { QUuid::createUuidV7() };
    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, report_id }));

    auto* view { settlement_widget_->View() };
    SetStatementView(view, std::to_underlying(SettlementEnum::kDescription));

    view->setColumnHidden(std::to_underlying(SettlementEnum::kId), false);

    DelegateSettlement(view, sc_->section_config);

    connect(model, &SettlementModel::SResizeColumnToContents, view, &QTableView::resizeColumnToContents);

    RegisterWidget(report_id, settlement_widget_);
}
