#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

namespace Ui {

class RegisterDialog;

}

class RegisterDialog : public QDialog {
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget* parent = nullptr);
    ~RegisterDialog();

public slots:
    void RRegisterResult(bool result);

private slots:
    void on_pushButtonSubmit_clicked();

private:
    Ui::RegisterDialog* ui;
};

#endif // REGISTERDIALOG_H
