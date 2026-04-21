#ifndef PERIODCLOSEDIALOG_H
#define PERIODCLOSEDIALOG_H

#include <QDialog>

#include "search/entry/searchentrymodel.h"
#include "tree/model/treemodel.h"

namespace Ui {
class PeriodCloseDialog;
}

class PeriodCloseDialog : public QDialog {
    Q_OBJECT

public:
    explicit PeriodCloseDialog(CTreeModel* model, SearchEntryModel* table_model, QWidget* parent = nullptr);
    ~PeriodCloseDialog() override;

    QTableView* View();

private:
    void InitDialog();

private:
    Ui::PeriodCloseDialog* ui;

    CTreeModel* model_ {};
};

#endif // PERIODCLOSEDIALOG_H
