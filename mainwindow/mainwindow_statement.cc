#include "finance/statement/statementenum.h"
#include "finance/statement/statementprimarywidget.h"
#include "finance/statement/statementsecondarywidget.h"
#include "finance/statement/statementtertiarywidget.h"
#include "mainwindow.h"

void MainWindow::on_actionStatement_triggered()
{
    qInfo() << Q_FUNC_INFO;

    Q_ASSERT(IsOrderSection(start_));

    auto* model { new statement::PrimaryModel(header_info_.statement_primary, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new StatementPrimaryWidget(model, widget_id, start_, this) };

    const int tab_index { sc_->tab_widget->addTab(widget, tr("Statement")) };
    auto* tab_bar { sc_->tab_widget->tabBar() };

    tab_bar->setTabData(tab_index, widget_id);

    auto* view { widget->View() };
    InitTableView(view, std::to_underlying(statement::PrimaryField::kPlaceholder));
    DelegateStatementPrimary(view, sc_->section_config);

    connect(widget, &StatementPrimaryWidget::SShowSecondaryStatement, this, &MainWindow::RShowSecondaryStatement);

    RegisterWidget(widget, widget_id, WidgetRole::kStatement);
}

void MainWindow::RStatementPrimary(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* d_widget { static_cast<StatementPrimaryWidget*>(widget.data()) };

    auto* model { d_widget->Model() };
    model->Rebuild(array);
}

void MainWindow::RStatemetSecondary(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* d_widget { static_cast<StatementSecondaryWidget*>(widget.data()) };

    auto* model { d_widget->Model() };
    model->Rebuild(array);
}

void MainWindow::RStatementTertiary(Section section, const QUuid& widget_id, const QJsonArray& array, const QJsonObject& total)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* d_widget { static_cast<StatementTertiaryWidget*>(widget.data()) };

    auto* model { d_widget->Model() };

    model->Rebuild(array);
    d_widget->ResetTotal(total);
}

void MainWindow::RShowSecondaryStatement(const QUuid& partner_id, const utils::DateRange& range, int unit)
{
    auto* model { new statement::SecondaryModel(header_info_.statement_secondary, partner_id, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new StatementSecondaryWidget(model, widget_id, partner_id, range, start_, unit, this) };

    const QString title { QString("%1-%2").arg(tr("Statement"), sc_p_.tree_model->Name(partner_id)) };

    const int tab_index { sc_->tab_widget->addTab(widget, title) };
    auto* tab_bar { sc_->tab_widget->tabBar() };

    tab_bar->setTabData(tab_index, widget_id);

    auto* view { widget->View() };
    InitTableView(view, std::to_underlying(statement::SecondaryField::kDescription));
    DelegateStatementSecondary(view, sc_->section_config);

    connect(widget, &StatementSecondaryWidget::SShowTertiaryStatement, this, &MainWindow::RShowTertiaryStatement);

    RegisterWidget(widget, widget_id, WidgetRole::kStatement);
}

void MainWindow::RShowTertiaryStatement(const QUuid& partner_id, const utils::DateRange& range, int unit)
{
    auto tree_model_p { sc_p_.tree_model };
    const QString partner_name { tree_model_p->Name(partner_id) };

    auto* model { new statement::TertiaryModel(header_info_.statement_tertiary, partner_id, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new StatementTertiaryWidget(model, widget_id, partner_id, range, partner_name, app_config_.company_name, start_, unit, this) };

    const QString title { QString("%1-%2").arg(tr("Statement Detail"), partner_name) };

    const int tab_index { sc_->tab_widget->addTab(widget, title) };
    auto* tab_bar { sc_->tab_widget->tabBar() };

    tab_bar->setTabData(tab_index, widget_id);

    auto* view { widget->View() };
    InitTableView(view, std::to_underlying(statement::TertiaryField::kDescription));
    DelegateStatementTertiary(view, sc_->section_config);

    RegisterWidget(widget, widget_id, WidgetRole::kStatement);
}

void MainWindow::RStatementMarkEntries(MarkOperation operation)
{
    auto* current_widget { sc_->tab_widget->currentWidget() };

    Q_ASSERT(qobject_cast<StatementTertiaryWidget*>(current_widget));
    auto* widget { static_cast<StatementTertiaryWidget*>(current_widget) };

    auto* model { widget->Model() };
    model->MarkEntries(operation);
}
