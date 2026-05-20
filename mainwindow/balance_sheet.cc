#include "charts/balance_sheet/balancesheetdialog.h"
#include "charts/balance_sheet/balancesheetenum.h"
#include "charts/balance_sheet/balancesheetmodel.h"
#include "mainwindow.h"
#include "utils/mainwindowutils.h"

void MainWindow::on_actionBalanceSheet_triggered()
{
    qInfo() << Q_FUNC_INFO;

    auto* model { new BalanceSheetModel(header_info_.balance_sheet, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* dialog { new BalanceSheetDialog(sc_f_.tree_model, model, widget_id, this) };

    {
        auto* view { dialog->View() };
        InitTreeView(view, std::to_underlying(BalanceSheetEnum::kId), std::to_underlying(BalanceSheetEnum::kDescription));
        DelegateBalanceSheet(view);
    }

    utils::ManageDialog(sc_f_.widget_hash, dialog, widget_id);

    dialog->show();
}