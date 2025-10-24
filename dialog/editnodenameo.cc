#include "editnodenameo.h"

#include "component/signalblocker.h"
#include "ui_editnodenameo.h"

EditNodeNameO::EditNodeNameO(CString& name, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::EditNodeNameO)
{
    ui->setupUi(this);
    SignalBlocker blocker(this);
    IniDialog(name);
    IniData(name);
}

EditNodeNameO::~EditNodeNameO() { delete ui; }

QString EditNodeNameO::GetName() const { return ui->lineName->text(); }

void EditNodeNameO::IniDialog(CString& name)
{
    ui->lineName->setFocus();
    ui->lineName->setValidator(&LineEdit::kInputValidator);

    this->setWindowTitle(name);
    this->setFixedSize(400, 300);
}

void EditNodeNameO::IniData(CString& name) { ui->lineName->setText(name); }
