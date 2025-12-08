#include "settlementnodewidget.h"

#include "ui_settlementnodewidget.h"

SettlementNodeWidget::SettlementNodeWidget(CUuid& widget_id, Settlement* settlement, std::shared_ptr<SettlementNodeList>& list, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettlementNodeWidget)
    , settlement_ { settlement }
    , widget_id_ { widget_id }
    , list_ { list }
{
    ui->setupUi(this);
}

SettlementNodeWidget::~SettlementNodeWidget() { delete ui; }
