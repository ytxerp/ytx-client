#include "history/orderenum.h"
#include "history/orderhistorywidget.h"
#include "history/ordermodelp.h"
#include "history/salesmodeli.h"
#include "mainwindow.h"

void MainWindow::ROrderHistory(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* ptr { widget.data() };

    Q_ASSERT(qobject_cast<OrderHistoryWidget*>(ptr));
    auto* d_widget { static_cast<OrderHistoryWidget*>(ptr) };

    auto* model { d_widget->Model() };
    model->Rebuild(array);
}

void MainWindow::RShowOrderHistoryWidget(const QUuid& node_id, NodeUnit unit)
{
    bool allowed { false };

    switch (start_) {
    case Section::kInventory:
        allowed = (unit == NodeUnit::IItem);
        break;
    case Section::kPartner:
        allowed = (unit == NodeUnit::PCustomer || unit == NodeUnit::PVendor);
        break;
    case Section::kFinance:
    case Section::kSale:
    case Section::kPurchase:
    case Section::kTask:
    default:
        return;
    }

    if (!allowed)
        return;

    Q_ASSERT(sc_ && sc_->tree_model);

    auto tree_model { sc_->tree_model };
    const auto& info { sc_->info };

    const QString title { QString("%1-%2").arg(tr("Record"), tree_model->Name(node_id)) };

    const Section section { info.section };
    const QUuid widget_id { QUuid::createUuidV7() };

    // The widget will take ownership of the model
    history::OrderModel* model {};

    {
        switch (section) {
        case Section::kInventory:
            model = new history::SalesModelI(info, nullptr);
            break;
        case Section::kPartner:
            model = new history::OrderModelP(info, node_id, sc_i_.tree_model, nullptr);
            break;
        case Section::kSale:
        case Section::kPurchase:
        case Section::kFinance:
        case Section::kTask:
            return;
        }
    }

    auto* widget { new OrderHistoryWidget(model, section, widget_id, node_id, unit, this) };

    const int tab_index { sc_->tab_widget->addTab(widget, title) };
    auto* tab_bar { sc_->tab_widget->tabBar() };

    tab_bar->setTabData(tab_index, widget_id);

    auto* view { widget->View() };

    {
        switch (section) {
        case Section::kInventory:
            InitTableView(view, std::to_underlying(history::SalesColumnI::kDescription));
            DelegateSalesHistoryI(view, sc_i_.section_config);
            break;
        case Section::kPartner:
            InitTableView(view, std::to_underlying(history::OrderColumnP::kDescription));
            DelegateOrderHistoryP(view, sc_p_.section_config);
            break;
        case Section::kSale:
        case Section::kPurchase:
        case Section::kFinance:
        case Section::kTask:
            return;
        }
    }

    RegisterWidget(widget, widget_id, WidgetRole::kOrderHistory);
}
