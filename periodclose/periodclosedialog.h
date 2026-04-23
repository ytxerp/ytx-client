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

private slots:
    void on_pushButtonPriview_clicked();

private:
    void InitDialog();
    QVector<Entry*> ConstructEntry(const QSet<Node*>& leaf_node, Node* to_node);

private:
    Ui::PeriodCloseDialog* ui;

    CTreeModel* model_ {};
    QSet<Node*> leaf_node_ {};
    QSet<Node*> branch_node_ {};
    QVector<Entry*> entries;
};

#endif // PERIODCLOSEDIALOG_H
