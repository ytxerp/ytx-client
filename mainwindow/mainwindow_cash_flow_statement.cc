#include "charts/cash_flow_statement/cashflowstatementdialog.h"
#include "charts/cash_flow_statement/cashflowstatementenum.h"
#include "mainwindow.h"
#include "utils/mainwindowutils.h"

void MainWindow::on_actionCashFlowStatement_triggered()
{
    qInfo() << Q_FUNC_INFO;

    auto* model { new CashFlowStatementModel(header_info_.cash_flow_statement, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* dialog { new CashFlowStatementDialog(model, widget_id, this) };

    {
        auto* view { dialog->View() };
        InitTreeView(view, std::to_underlying(CashFlowStatementEnum::kId), std::to_underlying(CashFlowStatementEnum::kDescription));
        DelegateCashFlowStatement(view);
    }

    utils::ManageDialog(sc_f_.widget_hash, dialog, widget_id);

    dialog->show();
}

void MainWindow::RCashFlowStatementAck(const QUuid& widget_id, const QJsonArray& o_node, const QJsonArray& o_path, const QJsonArray& i_node,
    const QJsonArray& i_path, const QJsonArray& f_node, const QJsonArray& f_path)
{
    auto widget { sc_f_.widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* d_widget { static_cast<CashFlowStatementDialog*>(widget.data()) };
    if (!d_widget)
        return;

    auto* model { d_widget->Model() };
    model->ResetModel(o_node, o_path, i_node, i_path, f_node, f_path);
}