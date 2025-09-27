#include "treewidgetp.h"

#include "ui_treewidgetp.h"

TreeWidgetP::TreeWidgetP(TreeModel* model, QWidget* parent)
    : TreeWidget(parent)
    , ui(new Ui::TreeWidgetP)
    , model_ { model }
{
    ui->setupUi(this);
    ui->treeViewP->setModel(model);
}

TreeWidgetP::~TreeWidgetP() { delete ui; }

QTreeView* TreeWidgetP::View() const { return ui->treeViewP; }
