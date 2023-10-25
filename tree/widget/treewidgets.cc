#include "treewidgets.h"

#include "ui_treewidgets.h"

TreeWidgetS::TreeWidgetS(TreeModel* model, QWidget* parent)
    : TreeWidget(parent)
    , ui(new Ui::TreeWidgetS)
    , model_ { model }
{
    ui->setupUi(this);
    ui->treeViewS->setModel(model);
}

TreeWidgetS::~TreeWidgetS() { delete ui; }

QTreeView* TreeWidgetS::View() const { return ui->treeViewS; }
