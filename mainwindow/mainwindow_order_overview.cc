#include <QScrollBar>

#include "component/constantint.h"
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

    auto* data_view { widget->OverviewView() };
    auto* filter_view { widget->FilterView() };

    InitFilterView(filter_view, data_view);

    {
        InitTableView(data_view, std::to_underlying(order_overview::RowField::kPlaceholder));
        DelegateOrderOverview(data_view);
        data_view->horizontalHeader()->hide();
    }

    {
        DelegateOrderFilterview(filter_view, sc_->info);
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

void MainWindow::InitFilterView(QTableView* filter_view, QTableView* data_view) const
{
    auto* filter_header { filter_view->horizontalHeader() };
    auto* data_header { data_view->horizontalHeader() };

    auto* filter_scrollbar { filter_view->horizontalScrollBar() };
    auto* data_scrollbar { data_view->horizontalScrollBar() };

    {
        filter_view->verticalHeader()->setDefaultSectionSize(ui_const::kRowHeight);
        filter_view->verticalHeader()->hide();

        filter_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        filter_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        filter_header->setSectionsMovable(true);
        filter_header->setSectionResizeMode(QHeaderView::Fixed);

        const int height { filter_header->sizeHint().height() + filter_view->verticalHeader()->defaultSectionSize() + filter_view->frameWidth() * 2 };

        filter_view->setFixedHeight(height);
    }

    {
        connect(data_header, &QHeaderView::sectionResized, filter_view,
            [filter_view](int logical_index, int, int new_size) { filter_view->setColumnWidth(logical_index, new_size); });

        connect(data_scrollbar, &QScrollBar::valueChanged, filter_scrollbar, &QScrollBar::setValue);

        connect(filter_header, &QHeaderView::sectionMoved, data_view, [data_header](int logical_index, int, int new_visual_index) {
            const int current_visual_index { data_header->visualIndex(logical_index) };

            if (current_visual_index != new_visual_index)
                data_header->moveSection(current_visual_index, new_visual_index);
        });
    }
}