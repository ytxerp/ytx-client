#include "mainwindow.h"
#include "periodclose/periodclosedialog.h"
#include "periodclose/periodclosemodel.h"
#include "utils/mainwindowutils.h"

void MainWindow::on_actionPeriodClose_triggered()
{
    qInfo() << Q_FUNC_INFO;

    static QPointer<PeriodCloseDialog> dialog {};

    if (!dialog) {
        auto* table_model { new PeriodCloseModel(sc_f_.info, this) };

        dialog = new PeriodCloseDialog(Section::kFinance, sc_f_.tree_model, table_model, this);
        utils::ManageDialog(sc_f_.widget_hash, dialog);

        auto* view { dialog->View() };
        InitTableView(
            view, std::to_underlying(FullEntryEnum::kId), std::to_underlying(FullEntryEnum::kVersion), std::to_underlying(FullEntryEnum::kDescription));
        DelegatePeriodClose(view);
    }

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}