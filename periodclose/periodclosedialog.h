#ifndef PERIODCLOSEDIALOG_H
#define PERIODCLOSEDIALOG_H

#include <QDialog>

#include "periodclosemodel.h"
#include "tree/model/treemodel.h"

namespace Ui {
class PeriodCloseDialog;
}

class PeriodCloseDialog : public QDialog {
    Q_OBJECT

public:
    explicit PeriodCloseDialog(Section section, CTreeModel* tree_model, PeriodCloseModel* table_model, QWidget* parent = nullptr);
    ~PeriodCloseDialog() override;

    QTableView* View();

private slots:
    void on_pushButtonPreview_clicked();

    void on_pushButtonCommit_clicked();

private:
    void InitDialog();
    void ConstructEntry(const QSet<Node*>& leaf_node, const Node* to_node);

private:
    Ui::PeriodCloseDialog* ui;
    const Section section_;

    CTreeModel* tree_model_ {};
    PeriodCloseModel* table_model_ {};

    QSet<Node*> leaf_node_ {};
    QSet<Node*> branch_node_ {};
    QList<Entry*> list_ {};
};

#endif // PERIODCLOSEDIALOG_H
