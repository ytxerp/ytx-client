#include "charts/cash_flow_statement/cashflowstatementdialog.h"
#include "charts/cash_flow_statement/cashflowstatementenum.h"
#include "component/constantstring.h"
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

void MainWindow::RCashFlowStatementAck(const QUuid& widget_id, const QJsonObject& obj)
{
    auto widget { sc_f_.widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* d_widget { static_cast<CashFlowStatementDialog*>(widget.data()) };
    if (!d_widget)
        return;

    const QJsonArray node_array { obj.value(kNodeArray).toArray() };
    const QJsonArray carrier_array { obj.value(cash_flow_statement::kCarrierArray).toArray() };
    const QJsonArray special_array { obj.value(cash_flow_statement::kSpecialArray).toArray() };
    const QJsonArray wrong_entry_array { obj.value(cash_flow_statement::kWrongEntryArray).toArray() };

    auto* model { d_widget->Model() };
    model->ResetModel(node_array);
}