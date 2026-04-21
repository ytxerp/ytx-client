#include "mainwindow.h"
#include "periodclose/periodclosedialog.h"
#include "utils/mainwindowutils.h"

void MainWindow::on_actionPeriodClose_triggered()
{
    auto* dialog { new PeriodCloseDialog(this) };
    utils::ManageDialog(sc_->widget_hash, dialog);
    dialog->setWindowModality(Qt::ApplicationModal);

    dialog->show();
}