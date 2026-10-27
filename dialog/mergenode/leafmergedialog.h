#ifndef LEAFMERGEDIALOG_H
#define LEAFMERGEDIALOG_H

#include <QDialog>

#include "tree/model/treemodel.h"

namespace Ui {
class LeafMergeDialog;
}

class LeafMergeDialog : public QDialog {
    Q_OBJECT

public:
    explicit LeafMergeDialog(TreeModel* tree_model, QWidget* parent = nullptr);
    ~LeafMergeDialog() override;

private slots:
    void on_comboBoxSource_currentIndexChanged(int index);
    void on_comboBoxTarget_currentIndexChanged(int index);

    void on_pBtnOk_clicked();
    void RMergeResult(bool result);

private:
    void InitData();
    void UpdateState();
    void InitConnect();

private:
    Ui::LeafMergeDialog* ui;

    TreeModel* tree_model_ {};
};

#endif // LEAFMERGEDIALOG_H
