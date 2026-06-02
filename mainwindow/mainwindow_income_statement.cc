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
        DelegateIncomeStatement(view);
    }

    utils::ManageDialog(sc_f_.widget_hash, dialog, widget_id);

    dialog->show();
}

void MainWindow::RIncomeStatementAck(const QUuid& widget_id, const QJsonObject& obj)
{
    auto widget { sc_f_.widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* d_widget { static_cast<IncomeStatementDialog*>(widget.data()) };
    if (!d_widget)
        return;

    const QJsonArray node_array { obj.value(kNodeArray).toArray() };
    const QJsonArray path_array { obj.value(kPathArray).toArray() };
    const double net_profit { obj.value(income_statement::kNetProfit).toString().toDouble() };
    const double yoy_net_profit { obj.value(income_statement::kYoyNetProfit).toString().toDouble() };
    const double mom_net_profit { obj.value(income_statement::kMomNetProfit).toString().toDouble() };

    const auto yoy_start { QDateTime::fromString(obj.value(income_statement::kYoyStart).toString(), Qt::ISODate).toLocalTime() };
    const auto yoy_end { QDateTime::fromString(obj.value(income_statement::kYoyEnd).toString(), Qt::ISODate).toLocalTime().addDays(-1) };
    const auto mom_start { QDateTime::fromString(obj.value(income_statement::kMomStart).toString(), Qt::ISODate).toLocalTime() };
    const auto mom_end { QDateTime::fromString(obj.value(income_statement::kMomEnd).toString(), Qt::ISODate).toLocalTime().addDays(-1) };

    const QString yoy_title { QString("YoY (%1~%2)").arg(yoy_start.toString(kDateFST), yoy_end.toString(kDateFST)) };
    const QString mom_title { QString("MoM (%1~%2)").arg(mom_start.toString(kDateFST), mom_end.toString(kDateFST)) };

    auto* model { d_widget->Model() };
    model->UpdateHeader(yoy_title, mom_title);
    model->ResetModel(node_array, path_array, net_profit, yoy_net_profit, mom_net_profit);
}