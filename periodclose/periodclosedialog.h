#ifndef PERIODCLOSEDIALOG_H
#define PERIODCLOSEDIALOG_H

#include <QDialog>

namespace Ui {
class PeriodCloseDialog;
}

class PeriodCloseDialog : public QDialog {
    Q_OBJECT

public:
    explicit PeriodCloseDialog(QWidget* parent = nullptr);
    ~PeriodCloseDialog();

private:
    Ui::PeriodCloseDialog* ui;
};

#endif // PERIODCLOSEDIALOG_H
