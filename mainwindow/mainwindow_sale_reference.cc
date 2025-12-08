#include "enum/reference.h"
#include "mainwindow.h"
#include "reference/salereferencewidget.h"
#include "ui_mainwindow.h"

void MainWindow::RSaleReference(Section section, const QUuid& widget_id, const QJsonArray& entry_array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id, nullptr) };
    if (!widget)
        return;

    auto* d_widget { static_cast<SaleReferenceWidget*>(widget.data()) };
    auto* model { d_widget->Model() };
    model->ResetModel(entry_array);
}

void MainWindow::RSaleReferencePrimary(const QUuid& node_id, int unit)
{
    assert(sc_->tree_widget);
    assert(sc_->tree_model->Kind(node_id) == std::to_underlying(NodeKind::kLeaf)
        && "Node kind should be 'kLeafNode' at this point. The kind check should be performed in the delegate DoubleSpinUnitRPS.");

    switch (start_) {
    case Section::kInventory:
        if (unit != std::to_underlying(UnitI::kInternal))
            return;
        break;
    case Section::kPartner:
        if (unit != std::to_underlying(UnitP::kCustomer))
            return;
        break;
    default:
        return;
    }

    CreateSaleReference(sc_->tree_model, sc_->info, node_id, unit);
}

void MainWindow::RSaleReferenceSecondary(const QModelIndex& index)
{
    const auto order_id { index.siblingAtColumn(std::to_underlying(SaleReferenceEnum::kOrderId)).data().toUuid() };
    const int column { std::to_underlying(SaleReferenceEnum::kInitial) };

    assert(!order_id.isNull());

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

    RNodeLocation(order_id);
}

void MainWindow::CreateSaleReference(TreeModel* tree_model, CSectionInfo& info, const QUuid& node_id, int unit)
{
    assert(tree_model);
    assert(tree_model->Contains(node_id));

    const QString title { QString("%1-%2").arg(tr("Record"), tree_model->Name(node_id)) };

    const Section section { info.section };
    const QUuid widget_id { QUuid::createUuidV7() };

    // The widget will take ownership of the model
    auto* model { new SaleReferenceModel(info, this) };
    auto* widget { new SaleReferenceWidget(model, section, widget_id, node_id, unit, this) };

    const int tab_index { ui->tabWidget->addTab(widget, title) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { section, widget_id }));

    auto* view { widget->View() };
    SetTableViewSaleReference(view);
    DelegateSaleReference(view, sc_sale_.section_config);

    connect(view, &QTableView::doubleClicked, this, &MainWindow::RSaleReferenceSecondary);

    RegisterWidget(widget_id, widget);
}
