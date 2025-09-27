#include "leafwidgetfipt.h"

#include "ui_leafwidgetfipt.h"

LeafWidgetFIPT::LeafWidgetFIPT(LeafModel* model, QWidget* parent)
    : LeafWidget(parent)
    , ui(new Ui::LeafWidgetFIPT)
    , model_ { model }
{
    ui->setupUi(this);
    ui->tableViewFIPT->setModel(model);
}

LeafWidgetFIPT::~LeafWidgetFIPT()
{
    delete model_;
    delete ui;
}

QTableView* LeafWidgetFIPT::View() const { return ui->tableViewFIPT; }
