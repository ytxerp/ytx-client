#include "audittextdialog.h"

#include "ui_audittextdialog.h"

AuditTextDialog::AuditTextDialog(const QString& text, const QString& title, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AuditTextDialog)
{
    ui->setupUi(this);
    InitDialog(text);
    this->setWindowTitle(title);
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
