#include "inventory_heat/inventoryheatdialog.h"
#include "inventory_heat/inventoryheatenum.h"
#include "mainwindow.h"
#include "utils/mainwindowutils.h"

void MainWindow::on_actionHeat_triggered()
{
    qInfo() << Q_FUNC_INFO;

    auto* model { new InventoryHeatModel(header_info_.inventory_heat, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* dialog { new InventoryHeatDialog(model, widget_id, this) };

    {
        auto* view { dialog->View() };
        InitTableView(view, -1, -1, std::to_underlying(InventoryHeatEnum::kPlaceholder));
    }

    utils::ManageDialog(sc_i_.widget_hash, dialog, widget_id);

    dialog->show();
}

void MainWindow::RInventoryHeatAck(const QUuid& widget_id, const QJsonArray& array)
{
    auto widget { sc_i_.widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* d_widget { static_cast<InventoryHeatDialog*>(widget.data()) };

    auto* model { d_widget->Model() };
    model->ResetModel(array);
}