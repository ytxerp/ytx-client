#include <QJsonArray>

#include "component/constantstring.h"
#include "dashboard/cash_flow_statement/cashflowstatementdialog.h"
#include "dashboard/cash_flow_statement/cashflowstatementenum.h"
#include "mainwindow.h"
#include "utils/mainwindowutils.h"

void MainWindow::on_actionCashFlowStatement_triggered()
{
    qInfo() << Q_FUNC_INFO;

    const QUuid widget_id { QUuid::createUuidV7() };
    auto* dialog { new CashFlowStatementDialog(header_info_, widget_id) };

    {
        auto* view { dialog->View() };
        InitTreeView(view, -1, std::to_underlying(cash_flow::RowField::kDescription));
        DelegateCashFlowStatement(view);

        auto* carrier_view { dialog->CarrierView() };
        InitTreeView(carrier_view, -1, std::to_underlying(cash_flow::RowField::kDescription));
        DelegateCashFlowStatementCarrier(carrier_view);
        carrier_view->setColumnHidden(std::to_underlying(cash_flow::RowField::kFinalTotal), kIsHidden);

        auto* special_view { dialog->SpecialView() };
        InitTreeView(special_view, -1, std::to_underlying(cash_flow::RowField::kDescription));
        DelegateCashFlowStatement(special_view);

        auto* wrong_view { dialog->WrongView() };
        InitTableView(wrong_view, std::to_underlying(cash_flow::WrongRowField::kDescription));
        DelegateCashFlowStatementWrong(wrong_view);
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
    const QJsonArray carrier_array { obj.value(cash_flow::kCarrierArray).toArray() };
    const QJsonArray counterpart_array { obj.value(cash_flow::kCounterPartArray).toArray() };
    const QJsonArray special_array { obj.value(cash_flow::kSpecialArray).toArray() };
    const QJsonArray wrong_entry_array { obj.value(cash_flow::kWrongEntryArray).toArray() };

    auto* model { d_widget->Model() };
    model->Rebuild(node_array);

    auto* carrier_model { d_widget->CarrierModel() };
    carrier_model->Rebuild(carrier_array, counterpart_array);

    auto* special_model { d_widget->SpecialModel() };
    special_model->Rebuild(special_array);

    auto* wrong_model { d_widget->WrongModel() };
    wrong_model->Rebuild(wrong_entry_array);
}