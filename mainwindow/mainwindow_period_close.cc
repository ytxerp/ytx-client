#include "finance/period_close/periodclosedialog.h"
#include "mainwindow.h"
#include "utils/mainwindowutils.h"

void MainWindow::on_actionPeriodClose_triggered()
{
    qInfo() << Q_FUNC_INFO;
    static QPointer<PeriodCloseDialog> dialog {};

    if (!dialog) {
        dialog = new PeriodCloseDialog(sc_f_.tree_model, sc_f_.info.full_entry_header);
        utils::ManageDialog(sc_f_.widget_hash, dialog);

        auto* view { dialog->View() };
        InitTableView(view, std::to_underlying(FullEntryEnumF::kDescription));
        DelegatePeriodClose(view);
    }

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}