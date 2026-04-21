#include "mainwindow.h"
#include "periodclose/periodclosedialog.h"
#include "search/entry/searchentrymodelf.h"
#include "utils/mainwindowutils.h"

void MainWindow::on_actionPeriodClose_triggered()
{
    auto* entry_model { new SearchEntryModelF(sc_->info, sc_->tag_hash, this) };
    auto* dialog { new PeriodCloseDialog(sc_f_.tree_model, entry_model, this) };

    utils::ManageDialog(sc_->widget_hash, dialog);
    dialog->setWindowModality(Qt::ApplicationModal);

    {
        auto* view { dialog->View() };
        InitTableView(
            view, std::to_underlying(FullEntryEnum::kId), std::to_underlying(FullEntryEnum::kVersion), std::to_underlying(FullEntryEnum::kDescription));

        DelegatePeriodClose(view);
    }

    dialog->show();
}