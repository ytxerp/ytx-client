#include "settlementnodewidget.h"

#include "ui_settlementnodewidget.h"

SettlementNodeWidget::SettlementNodeWidget(CUuid& partner_id, CUuid& settlement_id, std::shared_ptr<SettlementNodeList>& list, int status, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettlementNodeWidget)
    , list_ { list }
{
    ui->setupUi(this);
}

SettlementNodeWidget::~SettlementNodeWidget() { delete ui; }
