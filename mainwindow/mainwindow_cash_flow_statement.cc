#include "charts/cash_flow_statement/cashflowcarriermodel.h"
#include "charts/cash_flow_statement/cashflowspecialmodel.h"
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

    auto* carrier_model { new CashFlowCarrierModel(header_info_.cash_flow_statement, this) };
    auto* special_model { new CashFlowSpecialModel(header_info_.cash_flow_statement, this) };

    auto* dialog { new CashFlowStatementDialog(model, carrier_model, special_model, widget_id, this) };

    {
        auto* view { dialog->View() };
        InitTreeView(view, std::to_underlying(CashFlowStatementEnum::kId), std::to_underlying(CashFlowStatementEnum::kDescription));
        DelegateCashFlowStatement(view);

        auto* carrier_view { dialog->CarrierView() };
        InitTreeView(carrier_view, std::to_underlying(CashFlowStatementEnum::kId), std::to_underlying(CashFlowStatementEnum::kDescription));
        DelegateCashFlowStatement(carrier_view);

        auto* special_view { dialog->SpecialView() };
        InitTreeView(special_view, std::to_underlying(CashFlowStatementEnum::kId), std::to_underlying(CashFlowStatementEnum::kDescription));
        DelegateCashFlowStatement(special_view);
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
    const QJsonArray counterpart_array { obj.value(cash_flow_statement::kCounterPartArray).toArray() };
    const QJsonArray special_array { obj.value(cash_flow_statement::kSpecialArray).toArray() };
    const QJsonArray wrong_entry_array { obj.value(cash_flow_statement::kWrongEntryArray).toArray() };

    auto* model { d_widget->Model() };
    model->ResetModel(node_array);

    auto* carrier_model { d_widget->CarrierModel() };
    carrier_model->ResetModel(carrier_array, counterpart_array);

    auto* special_model { d_widget->SpecialModel() };
    special_model->ResetModel(special_array);
}