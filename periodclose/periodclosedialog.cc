#include "periodclosedialog.h"

#include "ui_periodclosedialog.h"

PeriodCloseDialog::PeriodCloseDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PeriodCloseDialog)
{
    ui->setupUi(this);
}

PeriodCloseDialog::~PeriodCloseDialog() { delete ui; }
