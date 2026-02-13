#include "enum/reference.h"
#include "mainwindow.h"
#include "reference/salereferencewidget.h"
#include "ui_mainwindow.h"

void MainWindow::RSaleReference(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* ptr { widget.data() };

    Q_ASSERT(qobject_cast<SaleReferenceWidget*>(ptr));
    auto* d_widget { static_cast<SaleReferenceWidget*>(ptr) };

    auto* model { d_widget->Model() };
    model->ResetModel(array);
}

void MainWindow::RSaleReferencePrimary(const QUuid& node_id, int unit)
{
    const bool allowed { (start_ == Section::kInventory && unit == std::to_underlying(NodeUnit::IInternal))
        || (start_ == Section::kPartner && unit == std::to_underlying(NodeUnit::PCustomer)) };

    if (!allowed)
        return;

    CreateSaleReference(node_id, unit);
}

void MainWindow::RSaleReferenceSecondary(const QModelIndex& index)
{
    const auto order_id { index.siblingAtColumn(std::to_underlying(SaleReferenceEnum::kOrderId)).data().toUuid() };
    const int column { std::to_underlying(SaleReferenceEnum::kInitial) };

    Q_ASSERT(!order_id.isNull());

    if (index.column() != column)
        return;

    RSectionGroup(std::to_underlying(Section::kSale));
    ui->rBtnSale->setChecked(true);

    auto* sc { GetSectionContex(Section::kSale) };
    auto tree_model { sc->tree_model };

    if (!tree_model->Contains(order_id)) {
        tree_model->AckNode(order_id);
        return;
    }

    RNodeLocation(Section::kSale, order_id);
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
    auto* model { new SaleReferenceModel(info, this) };
    auto* widget { new SaleReferenceWidget(model, section, widget_id, node_id, unit, this) };

    const int tab_index { sc_->tab_widget->addTab(widget, title) };
    auto* tab_bar { sc_->tab_widget->tabBar() };

    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { section, widget_id }));

    auto* view { widget->View() };
    SetTableViewSaleReference(view);
    DelegateSaleReference(view, sc_sale_.section_config);

    connect(view, &QTableView::doubleClicked, this, &MainWindow::RSaleReferenceSecondary);

    RegisterWidget(widget, widget_id, WidgetRole::kSaleReference);
}
