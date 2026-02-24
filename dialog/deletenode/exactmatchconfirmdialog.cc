#include "exactmatchconfirmdialog.h"

#include <QRandomGenerator>

#include "ui_exactmatchconfirmdialog.h"

ExactMatchConfirmDialog::ExactMatchConfirmDialog(CString& info, CString& match_text, CString& accept_text, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ExactMatchConfirmDialog)
    , match_text_ { match_text }
{
    ui->setupUi(this);

    if (match_text_.isEmpty()) {
        int num = QRandomGenerator::global()->bounded(1000, 10000); // [1000,9999]
        match_text_ = QString::number(num);
    }

    ui->label->setText(info);
    ui->lineEditMatchText->setText(match_text_);
    ui->pBtnApply->setText(accept_text);
    ui->lineEditInput->setFocus();

    ui->pBtnApply->setEnabled(false);
    InitConnect();
}

ExactMatchConfirmDialog::~ExactMatchConfirmDialog() { delete ui; }

void ExactMatchConfirmDialog::InitConnect()
{
    connect(ui->lineEditInput, &QLineEdit::textChanged, this,
        [=, this](const QString& text) { ui->pBtnApply->setEnabled(!text.isEmpty() && text == ui->lineEditMatchText->text()); });

    // Cancel
    connect(ui->pBtnCancel, &QPushButton::clicked, this, &QDialog::reject);
    // Apply
    connect(ui->pBtnApply, &QPushButton::clicked, this, &QDialog::accept);
}
