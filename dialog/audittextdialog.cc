#include "audittextdialog.h"

#include "ui_audittextdialog.h"

AuditTextDialog::AuditTextDialog(const QString& text, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AuditTextDialog)
{
    ui->setupUi(this);
    InitDialog(text);
}

AuditTextDialog::~AuditTextDialog() { delete ui; }

void AuditTextDialog::InitDialog(const QString& text)
{
    setSizeGripEnabled(true); // works perfectly on top-level window
    resize(400, 300);

    ui->plainTextEdit->setReadOnly(true);
    ui->plainTextEdit->setFrameStyle(QFrame::NoFrame);
    ui->plainTextEdit->setPlainText(text);
}
