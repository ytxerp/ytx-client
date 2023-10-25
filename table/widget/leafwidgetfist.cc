#include "leafwidgetfist.h"

#include "ui_leafwidgetfist.h"

LeafWidgetFIST::LeafWidgetFIST(LeafModel* model, QWidget* parent)
    : LeafWidget(parent)
    , ui(new Ui::LeafWidgetFIST)
    , model_ { model }
{
    ui->setupUi(this);
    ui->tableViewFIST->setModel(model);
}

LeafWidgetFIST::~LeafWidgetFIST()
{
    delete model_;
    delete ui;
}

QTableView* LeafWidgetFIST::View() const { return ui->tableViewFIST; }
