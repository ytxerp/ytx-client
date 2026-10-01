#include "dashboard/order_overview/orderoverviewwidget.h"
#include "mainwindow.h"

void MainWindow::on_actionOrderOverview_triggered()
{
    qInfo() << Q_FUNC_INFO;

    Q_ASSERT(IsOrderSection(start_));

    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new OrderOverviewWidget(header_info_.order_overview, widget_id, start_, this) };

    {
        const int tab_index { sc_->tab_widget->addTab(widget, tr("Order Overview")) };
        auto* tab_bar { sc_->tab_widget->tabBar() };

        tab_bar->setTabData(tab_index, widget_id);
    }

    {
        auto* view { widget->OverviewView() };
        InitTableView(view, std::to_underlying(order_overview::RowField::kPlaceholder));
        DelegateOrderOverview(view);
        view->horizontalHeader()->hide();
    }

    {
        auto* view { widget->FilterView() };
        DelegateOrderFilterview(view, sc_->info);
    }

    RegisterWidget(widget, widget_id, WidgetRole::kOrderOverview);
}

void MainWindow::ROrderOverview(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* ptr { widget.data() };

    Q_ASSERT(qobject_cast<OrderOverviewWidget*>(ptr));
    auto* d_widget { static_cast<OrderOverviewWidget*>(ptr) };

    auto* model { d_widget->OverviewModel() };
    model->Rebuild(array);
}