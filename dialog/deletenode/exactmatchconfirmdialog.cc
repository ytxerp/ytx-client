#include "exactmatchconfirmdialog.h"

#include <QRandomGenerator>

#include "ui_exactmatchconfirmdialog.h"

ExactMatchConfirmDialog::ExactMatchConfirmDialog(CString& info, CString& accept_text, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ExactMatchConfirmDialog)
{
    ui->setupUi(this);

    const int num { QRandomGenerator::global()->bounded(1000, 10000) }; // [1000,9999]

    ui->label->setText(info);
    ui->lineEditMatchText->setText(QString::number(num));
    ui->pBtnApply->setText(accept_text);
    ui->lineEditInput->setFocus();

    ui->pBtnApply->setEnabled(false);
    InitConnect();
}

ExactMatchConfirmDialog::~ExactMatchConfirmDialog() { delete ui; }

void ExactMatchConfirmDialog::InitConnect()
{
    connect(ui->lineEditInput, &QLineEdit::textChanged, this,
        [this](const QString& text) { ui->pBtnApply->setEnabled(!text.isEmpty() && text == ui->lineEditMatchText->text()); });

    // Cancel
    connect(ui->pBtnCancel, &QPushButton::clicked, this, &QDialog::reject);
    // Apply
    connect(ui->pBtnApply, &QPushButton::clicked, this, &QDialog::accept);
}
