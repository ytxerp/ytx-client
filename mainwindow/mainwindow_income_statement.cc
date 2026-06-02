#include "charts/income_statement/incomestatementdialog.h"
#include "charts/income_statement/incomestatementenum.h"
#include "charts/income_statement/incomestatementmodel.h"
#include "mainwindow.h"
#include "utils/mainwindowutils.h"

void MainWindow::on_actionIncomeStatement_triggered()
{
    qInfo() << Q_FUNC_INFO;

    auto* model { new IncomeStatementModel(header_info_.income_statement, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* dialog { new IncomeStatementDialog(sc_f_.tree_model, model, widget_id, this) };

    {
        auto* view { dialog->View() };
        InitTreeView(view, std::to_underlying(IncomeStatementEnum::kId), std::to_underlying(IncomeStatementEnum::kDescription));
        DelegateBalanceSheet(view);
    }

    utils::ManageDialog(sc_f_.widget_hash, dialog, widget_id);

    dialog->show();
}

void MainWindow::RIncomeStatementAck(const QUuid& widget_id, const QJsonArray& node_array, const QJsonArray& path_array, double net_profit)
{
    auto widget { sc_f_.widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* d_widget { static_cast<IncomeStatementDialog*>(widget.data()) };
    if (!d_widget)
        return;

    auto* model { d_widget->Model() };
    model->ResetModel(node_array, path_array, net_profit);
}