#include "finance/reimbursement/reimbursementprimarywidget.h"
#include "finance/reimbursement/reimbursementsecondarymodel.h"
#include "finance/reimbursement/reimbursementsecondarywidget.h"
#include "global/userprofile.h"
#include "mainwindow.h"

void MainWindow::on_actionReimbursement_triggered()
{
    qInfo() << Q_FUNC_INFO;

    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new ReimbursementPrimaryWidget(widget_id, header_info_.reimbursement_primary, this) };

    {
        const int tab_index { sc_->tab_widget->addTab(widget, tr("Reimbursement")) };
        auto* tab_bar { sc_->tab_widget->tabBar() };

        tab_bar->setTabData(tab_index, widget_id);
    }

    {
        auto* view { widget->View() };

        connect(view, &QTableView::doubleClicked, this, &MainWindow::ROpenReimbursementSecondaryWidget);
        connect(widget, &ReimbursementPrimaryWidget::SCreateReimbursementSecondaryWidget, this, &MainWindow::RCreateReimbursementSecondaryWidget);

        InitTableView(view, std::to_underlying(reimbursement::PrimaryField::kPlaceholder));
        DelegateReimbursementPrimary(view, sc_f_.section_config);
    }

    RegisterWidget(sc_, widget, widget_id, WidgetRole::kReimbursement);
}

void MainWindow::RCreateReimbursementSecondaryWidget(reimbursement::PrimaryModel* model)
{
    reimbursement::PrimaryRow primary_row {};

    primary_row.issued_time = QDateTime::currentDateTime();
    primary_row.id = QUuid::createUuidV7();

    CreateReimbursementSecondary(primary_row, model);
}

void MainWindow::ROpenReimbursementSecondaryWidget(const QModelIndex& index)
{
    qInfo() << Q_FUNC_INFO;

    auto* current_widget { sc_->tab_widget->currentWidget() };

    Q_ASSERT(qobject_cast<ReimbursementPrimaryWidget*>(current_widget));
    auto* widget { static_cast<ReimbursementPrimaryWidget*>(current_widget) };

    if (index.column() != std::to_underlying(reimbursement::PrimaryField::kUsername))
        return;

    auto* primary_row { static_cast<reimbursement::PrimaryRow*>(index.internalPointer()) };
    auto* primary_model { widget->Model() };

    CreateReimbursementSecondary(*primary_row, primary_model);
}

void MainWindow::CreateReimbursementSecondary(const reimbursement::PrimaryRow& primary_row, reimbursement::PrimaryModel* primary_model)
{
    const QUuid widget_id { QUuid::createUuidV7() };

    auto* model { new reimbursement::SecondaryModel(
        header_info_.reimbursement_secondary, primary_row.status, UserProfile::Instance().SectionPermissions(Section::kFinance), this) };

    auto* widget { new ReimbursementSecondaryWidget(model, sc_f_.section_config, primary_row, widget_id, this) };
    connect(widget, &ReimbursementSecondaryWidget::SInsertPrimaryRow, primary_model, &reimbursement::PrimaryModel::RInsertePrimaryRow);
    connect(widget, &ReimbursementSecondaryWidget::SUpdatePrimaryRow, primary_model, &reimbursement::PrimaryModel::RUpdatePrimaryRow);

    {
        const QString name { UserProfile::Instance().Username() };
        const QString label { QString("%1-%2").arg(tr("Reimbursement"), name) };

        const int tab_index { sc_->tab_widget->addTab(widget, label) };
        auto* tab_bar { sc_->tab_widget->tabBar() };

        tab_bar->setTabData(tab_index, widget_id);
    }

    {
        auto* view { widget->View() };
        InitTableView(view, std::to_underlying(reimbursement::SecondaryField::kPlaceholder));
        DelegateReimbursementSecondary(view, sc_f_.section_config);
    }

    RegisterWidget(sc_, widget, widget_id, WidgetRole::kReimbursement);
}