#include "finance/reimbursement/tablemodelr.h"
#include "finance/reimbursement/tablewidgetr.h"
#include "finance/reimbursement/treewidgetr.h"
#include "global/userprofile.h"
#include "mainwindow.h"

void MainWindow::on_actionReimbursement_triggered()
{
    qInfo() << Q_FUNC_INFO;

    const QUuid widget_id { QUuid::createUuidV7() };

    auto* widget { new TreeWidgetR(widget_id, header_info_.reimbursement_primary, this) };

    {
        const int tab_index { sc_->tab_widget->addTab(widget, tr("Reimbursement")) };
        auto* tab_bar { sc_->tab_widget->tabBar() };

        tab_bar->setTabData(tab_index, widget_id);
    }

    {
        auto* view { widget->View() };

        connect(view, &QTableView::doubleClicked, this, &MainWindow::ROpenReimbursementSecondaryWidget);
        connect(widget, &TreeWidgetR::SCreateReimbursementSecondaryWidget, this, &MainWindow::RCreateReimbursementSecondaryWidget);

        InitTableView(view, std::to_underlying(reimbursement::PrimaryField::kPlaceholder));
        DelegateReimbursementPrimary(view, sc_f_.section_config);
    }

    RegisterWidget(sc_, widget, widget_id, WidgetRole::kReimbursement);
}

void MainWindow::RCreateReimbursementSecondaryWidget(reimbursement::TreeModelR* model)
{
    reimbursement::PrimaryRow primary_row {};

    primary_row.issued_time = QDateTime::currentDateTime();
    primary_row.id = QUuid::createUuidV7();
    primary_row.sync_state = SyncState::kCreating;

    CreateReimbursementSecondary(primary_row, model);
}

void MainWindow::ROpenReimbursementSecondaryWidget(const QModelIndex& index)
{
    qInfo() << Q_FUNC_INFO;

    auto* current_widget { sc_->tab_widget->currentWidget() };

    Q_ASSERT(qobject_cast<TreeWidgetR*>(current_widget));
    auto* widget { static_cast<TreeWidgetR*>(current_widget) };

    if (index.column() != std::to_underlying(reimbursement::PrimaryField::kUsername))
        return;

    auto* primary_row { static_cast<reimbursement::PrimaryRow*>(index.internalPointer()) };
    auto* primary_model { widget->Model() };

    CreateReimbursementSecondary(*primary_row, primary_model);
}

void MainWindow::CreateReimbursementSecondary(const reimbursement::PrimaryRow& primary_row, reimbursement::TreeModelR* primary_model)
{
    const QUuid widget_id { QUuid::createUuidV7() };
    const int finance_permission { UserProfile::Instance().SectionPermissions(Section::kFinance) };

    auto* model { new reimbursement::TableModelR(header_info_.reimbursement_secondary, primary_row.status, primary_row.id, finance_permission, this) };
    auto* widget { new TableWidgetR(model, sc_f_.section_config, primary_row, widget_id, finance_permission, this) };

    connect(widget, &TableWidgetR::SInsertPrimaryRow, primary_model, &reimbursement::TreeModelR::RInsertePrimaryRow);
    connect(widget, &TableWidgetR::SUpdatePrimaryRow, primary_model, &reimbursement::TreeModelR::RUpdatePrimaryRow);

    {
        const QString username { primary_row.sync_state == SyncState::kSynced ? primary_row.username : UserProfile::Instance().Username() };
        const QString label { tr("Reimbursement") + "-" + username };

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