#include "component/constantstring.h"
#include "mainwindow.h"
#include "order/settlement_view/settlementviewwidget.h"

void MainWindow::on_actionSettlementView_triggered()
{
    qInfo() << Q_FUNC_INFO;

    const QUuid widget_id { QUuid::createUuidV7() };
    const Section section { sc_->section };

    Q_ASSERT(IsOrderSection(section));

    DoubleNoneZeroR* amount_dlg { new DoubleNoneZeroR(sc_->section_config.amount_decimal, string_const::kEightDigits, this) };
    auto* widget { new SettlementViewWidget(section, widget_id, sc_p_.tree_model->LeafPath(), amount_dlg, this) };

    {
        const int tab_index { sc_->tab_widget->addTab(widget, tr("Settlement View")) };
        auto* tab_bar { sc_->tab_widget->tabBar() };
        tab_bar->setTabData(tab_index, widget_id);
    }

    {
        auto* summary_view { widget->SummaryView() };
        auto* data_view { widget->DataView() };
        InitTableView(data_view, -1);
        InitSummaryView(summary_view, data_view);
    }

    RegisterWidget(sc_, widget, widget_id, WidgetRole::kSettlementView);
}

void MainWindow::RSettlementView(Section section, QUuid widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContext(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* d_widget { static_cast<SettlementViewWidget*>(widget.data()) };

    auto* model { d_widget->DataModel() };
    model->Rebuild(array);
}
