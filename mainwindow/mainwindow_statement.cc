#include "billing/statement/statementenum.h"
#include "billing/statement/statementprimarywidget.h"
#include "billing/statement/statementsecondarywidget.h"
#include "billing/statement/statementtertiarywidget.h"
#include "mainwindow.h"

void MainWindow::on_actionStatement_triggered()
{
    qInfo() << Q_FUNC_INFO;

    Q_ASSERT(IsOrderSection(start_));

    auto* model { new StatementPrimaryModel(header_info_.statement, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new StatementPrimaryWidget(model, widget_id, start_, this) };

    const int tab_index { sc_->tab_widget->addTab(widget, tr("Statement")) };
    auto* tab_bar { sc_->tab_widget->tabBar() };

    tab_bar->setTabData(tab_index, widget_id);

    auto* view { widget->View() };
    InitTableView(view, -1, -1, std::to_underlying(StatementPrimaryEnum::kPlaceholder));
    DelegateStatement(view, sc_->section_config);

    connect(widget, &StatementPrimaryWidget::SStatementNode, this, &MainWindow::RStatementNode);

    RegisterWidget(widget, widget_id, WidgetRole::kStatement);
}

void MainWindow::RAckStatement(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* d_widget { static_cast<StatementPrimaryWidget*>(widget.data()) };

    auto* model { d_widget->Model() };
    model->ResetModel(array);
}

void MainWindow::RAckStatementNode(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* d_widget { static_cast<StatementSecondaryWidget*>(widget.data()) };

    auto* model { d_widget->Model() };
    model->ResetModel(array);
}

void MainWindow::RAckStatementEntry(Section section, const QUuid& widget_id, const QJsonArray& array, const QJsonObject& total)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* d_widget { static_cast<StatementTertiaryWidget*>(widget.data()) };

    auto* model { d_widget->Model() };

    model->ResetModel(array);
    d_widget->ResetTotal(total);
}

void MainWindow::RStatementNode(const QUuid& partner_id, const QDateTime& start, const QDateTime& end, int unit)
{
    auto* model { new StatementSecondaryModel(header_info_.statement_node, partner_id, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new StatementSecondaryWidget(model, widget_id, partner_id, start, end, start_, unit, this) };

    const QString title { QString("%1-%2").arg(tr("Statement"), sc_p_.tree_model->Name(partner_id)) };

    const int tab_index { sc_->tab_widget->addTab(widget, title) };
    auto* tab_bar { sc_->tab_widget->tabBar() };

    tab_bar->setTabData(tab_index, widget_id);

    auto* view { widget->View() };
    InitTableView(view, -1, -1, std::to_underlying(StatementSecondaryEnum::kDescription));
    DelegateStatementNode(view, sc_->section_config);

    connect(widget, &StatementSecondaryWidget::SStatementEntry, this, &MainWindow::RStatementEntry);

    RegisterWidget(widget, widget_id, WidgetRole::kStatement);
}

void MainWindow::RStatementEntry(const QUuid& partner_id, const QDateTime& start, const QDateTime& end, int unit)
{
    auto tree_model_p { sc_p_.tree_model };
    const QString partner_name { tree_model_p->Name(partner_id) };

    auto* tree_model_i { static_cast<TreeModelI*>(sc_i_.tree_model.data()) };
    Q_ASSERT(tree_model_i != nullptr);

    auto* model { new StatementTertiaryModel(header_info_.statement_entry, partner_id, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new StatementTertiaryWidget(
        model, tree_model_i, widget_id, partner_id, start, end, partner_name, app_config_.company_name, start_, unit, this) };

    const QString title { QString("%1-%2").arg(tr("Statement Detail"), partner_name) };

    const int tab_index { sc_->tab_widget->addTab(widget, title) };
    auto* tab_bar { sc_->tab_widget->tabBar() };

    tab_bar->setTabData(tab_index, widget_id);

    auto* view { widget->View() };
    InitTableView(view, -1, -1, std::to_underlying(StatementTertiaryEnum::kDescription));
    DelegateStatementEntry(view, sc_->section_config);

    RegisterWidget(widget, widget_id, WidgetRole::kStatement);
}

void MainWindow::RStatementMarkBatch(Mark mark)
{
    auto* current_widget { sc_->tab_widget->currentWidget() };

    Q_ASSERT(qobject_cast<StatementTertiaryWidget*>(current_widget));
    auto* widget { static_cast<StatementTertiaryWidget*>(current_widget) };

    auto* model { widget->Model() };
    model->MarkBatch(mark);
}
