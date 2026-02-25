#include "mainwindow.h"
#include "reference/orderreferencemodelp.h"
#include "reference/orderreferencewidget.h"
#include "reference/salereferencemodeli.h"

void MainWindow::ROrderReference(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* ptr { widget.data() };

    Q_ASSERT(qobject_cast<OrderReferenceWidget*>(ptr));
    auto* d_widget { static_cast<OrderReferenceWidget*>(ptr) };

    auto* model { d_widget->Model() };
    model->ResetModel(array);
}

void MainWindow::ROrderReferencePrimary(const QUuid& node_id, int unit)
{
    const bool allowed { (start_ == Section::kInventory && unit == std::to_underlying(NodeUnit::IInternal))
        || (start_ == Section::kPartner && unit == std::to_underlying(NodeUnit::PCustomer)) };

    if (!allowed)
        return;

    CreateSaleReference(node_id, unit);
}

void MainWindow::CreateSaleReference(const QUuid& node_id, int unit)
{
    Q_ASSERT(sc_ && sc_->tree_model);

    auto tree_model { sc_->tree_model };
    const auto& info { sc_->info };

    const QString title { QString("%1-%2").arg(tr("Record"), tree_model->Name(node_id)) };

    const Section section { info.section };
    const QUuid widget_id { QUuid::createUuidV7() };

    // The widget will take ownership of the model
    OrderReferenceModel* model {};

    {
        switch (section) {
        case Section::kInventory:
            model = new SaleReferenceModelI(info, nullptr);
            break;
        case Section::kPartner:
            model = new OrderReferenceModelP(info, node_id, sc_i_.tree_model, sc_p_.entry_hub, nullptr);
            break;
        case Section::kSale:
        case Section::kPurchase:
        case Section::kFinance:
        case Section::kTask:
            return;
        }
    }

    auto* widget { new OrderReferenceWidget(model, section, widget_id, node_id, unit, this) };

    const int tab_index { sc_->tab_widget->addTab(widget, title) };
    auto* tab_bar { sc_->tab_widget->tabBar() };

    tab_bar->setTabData(tab_index, widget_id);

    auto* view { widget->View() };

    {
        switch (section) {
        case Section::kInventory:
            SetTableViewSaleReferenceI(view);
            DelegateSaleReferenceI(view, sc_i_.section_config);
            break;
        case Section::kPartner:
            SetTableViewSaleReferenceP(view);
            DelegateSaleReferenceP(view, sc_p_.section_config);
            break;
        case Section::kSale:
        case Section::kPurchase:
        case Section::kFinance:
        case Section::kTask:
            return;
        }
    }

    RegisterWidget(widget, widget_id, WidgetRole::kSaleReference);
}
