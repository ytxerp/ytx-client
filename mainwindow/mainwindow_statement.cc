#include "billing/statement/statemententrywidget.h"
#include "billing/statement/statementnodewidget.h"
#include "billing/statement/statementwidget.h"
#include "enum/statementenum.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::on_actionStatement_triggered()
{
    qInfo() << "[UI]" << "on_actionStatement_triggered";

    Q_ASSERT(IsOrderSection(start_));

    auto* model { new StatementModel(sc_->info, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new StatementWidget(model, widget_id, start_, this) };

    const int tab_index { ui->tabWidget->addTab(widget, tr("Statement")) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, widget_id }));

    auto* view { widget->View() };
    SetStatementView(view, std::to_underlying(StatementEnum::kPlaceholder));
    DelegateStatement(view, sc_->section_config);

    connect(widget, &StatementWidget::SStatementNode, this, &MainWindow::RStatementNode);

    RegisterWidget(widget_id, widget);
}

void MainWindow::RStatement(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id, nullptr) };
    if (!widget)
        return;

    auto* d_widget { static_cast<StatementWidget*>(widget.data()) };
    Q_ASSERT(d_widget != nullptr);

    auto* model { d_widget->Model() };
    model->ResetModel(array);
}

void MainWindow::RStatementNodeAcked(Section section, const QUuid& widget_id, const QJsonArray& array)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id, nullptr) };
    if (!widget)
        return;

    auto* d_widget { static_cast<StatementNodeWidget*>(widget.data()) };
    Q_ASSERT(d_widget != nullptr);

    auto* model { d_widget->Model() };
    model->ResetModel(array);
}

void MainWindow::RStatementEntryAcked(Section section, const QUuid& widget_id, const QJsonArray& array, const QJsonObject& total)
{
    auto* sc { GetSectionContex(section) };

    auto widget { sc->widget_hash.value(widget_id, nullptr) };
    if (!widget)
        return;

    auto* d_widget { static_cast<StatementEntryWidget*>(widget.data()) };
    Q_ASSERT(d_widget != nullptr);

    auto* model { d_widget->Model() };

    model->ResetModel(array);
    d_widget->ResetTotal(total);
}

void MainWindow::RStatementNode(const QUuid& partner_id, const QDateTime& start, const QDateTime& end, int unit)
{
    auto* model { new StatementNodeModel(sc_->info, partner_id, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new StatementNodeWidget(model, widget_id, partner_id, start, end, start_, unit, this) };

    const QString title { QString("%1-%2").arg(tr("Statement"), sc_p_.tree_model->Name(partner_id)) };

    const int tab_index { ui->tabWidget->addTab(widget, title) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, widget_id }));

    auto* view { widget->View() };
    SetStatementView(view, std::to_underlying(StatementNodeEnum::kDescription));
    DelegateStatementNode(view, sc_->section_config);

    connect(widget, &StatementNodeWidget::SStatementEntry, this, &MainWindow::RStatementEntry);

    RegisterWidget(widget_id, widget);
}

void MainWindow::RStatementEntry(const QUuid& partner_id, const QDateTime& start, const QDateTime& end, int unit)
{
    auto tree_model_p { sc_p_.tree_model };
    const QString partner_name { tree_model_p->Name(partner_id) };

    auto* entry_hub_p { static_cast<EntryHubP*>(sc_p_.entry_hub.data()) };
    auto* tree_model_i { static_cast<TreeModelI*>(sc_i_.tree_model.data()) };

    Q_ASSERT(entry_hub_p != nullptr);
    Q_ASSERT(tree_model_i != nullptr);

    auto* model { new StatementEntryModel(entry_hub_p, sc_->info, partner_id, this) };
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new StatementEntryWidget(
        model, entry_hub_p, tree_model_i, widget_id, partner_id, start, end, partner_name, app_config_.company_name, start_, unit, this) };

    const QString title { QString("%1-%2").arg(tr("StatementDetail"), partner_name) };

    const int tab_index { ui->tabWidget->addTab(widget, title) };
    auto* tab_bar { ui->tabWidget->tabBar() };

    tab_bar->setTabData(tab_index, QVariant::fromValue(TabInfo { start_, widget_id }));

    auto* view { widget->View() };
    SetStatementView(view, std::to_underlying(StatementEntryEnum::kDescription));
    DelegateStatementEntry(view, sc_->section_config);

    RegisterWidget(widget_id, widget);
}
