#include "settlementnodewidget.h"

#include "component/signalblocker.h"
#include "ui_settlementnodewidget.h"

SettlementNodeWidget::SettlementNodeWidget(
    SettlementNodeModel* model, const std::shared_ptr<Settlement>& settlement, bool is_persisted, Section section, CUuid& widget_id, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettlementNodeWidget)
    , settlement_ { settlement }
    , model_ { model }
    , widget_id_ { widget_id }
    , section_ { section }
    , is_persisted_ { is_persisted }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);
    InitWidget();
}

SettlementNodeWidget::~SettlementNodeWidget() { delete ui; }

QTableView* SettlementNodeWidget::View() const { return ui->tableView; }

void SettlementNodeWidget::InitWidget() { }
