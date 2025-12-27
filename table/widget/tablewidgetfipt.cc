#include "tablewidgetfipt.h"

#include "ui_tablewidgetfipt.h"

TableWidgetFIPT::TableWidgetFIPT(TableModel* model, QWidget* parent)
    : TableWidget(parent)
    , ui(new Ui::TableWidgetFIPT)
    , model_ { model }
{
    ui->setupUi(this);
    ui->tableViewFIPT->setModel(model);
    model_->setParent(ui->tableViewFIPT);
}

TableWidgetFIPT::~TableWidgetFIPT() { delete ui; }

QTableView* TableWidgetFIPT::View() const { return ui->tableViewFIPT; }
