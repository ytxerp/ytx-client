#include "editnodename.h"

#include "component/signalblocker.h"
#include "ui_editnodename.h"

EditNodeName::EditNodeName(CString& name, CString& parent_path, const QSet<QString>& name_set, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::EditNodeName)
    , parent_path_ { parent_path }
    , name_set_ { name_set }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    InitDialog(name);
    InitConnect();
    InitData(name);
}

EditNodeName::~EditNodeName() { delete ui; }

void EditNodeName::InitDialog(CString& name)
{
    ui->lineName->setFocus();
    ui->lineName->setValidator(&LineEdit::kInputValidator);

    this->setWindowTitle(parent_path_ + name);
    this->setFixedSize(400, 300);
}

void EditNodeName::InitConnect() { connect(ui->lineName, &QLineEdit::textEdited, this, &EditNodeName::RNameEdited); }

void EditNodeName::InitData(CString& name)
{
    ui->lineName->setText(name);
    ui->pBtnOk->setEnabled(false);
}

void EditNodeName::RNameEdited(const QString& arg1)
{
    const auto& simplified { arg1.simplified() };
    this->setWindowTitle(parent_path_ + simplified);
    ui->pBtnOk->setEnabled(!simplified.isEmpty() && !name_set_.contains(simplified));
}

QString EditNodeName::GetName() const { return ui->lineName->text(); }
