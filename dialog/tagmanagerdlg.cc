#include "tagmanagerdlg.h"

#include "ui_tagmanagerdlg.h"

TagManagerDlg::TagManagerDlg(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::TagManagerDlg)
{
    ui->setupUi(this);
}

TagManagerDlg::~TagManagerDlg() { delete ui; }
