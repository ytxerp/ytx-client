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
    void ConstructEntry(const QSet<Node*>& closing_leaf_node, const Node* summary_node);
    void ResetState();

    QJsonArray BuildUuidArray(const QSet<Node*>& set);

private:
    Ui::PeriodCloseDialog* ui;
    const Section section_;

    CTreeModel* tree_model_ {};
    PeriodCloseModel* table_model_ {};

    QUuid summary_node_id_ {};
    QSet<Node*> closing_leaf_node_ {};

    QList<Entry*> entry_list_ {};
};

#endif // PERIODCLOSEDIALOG_H
