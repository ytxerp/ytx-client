#include "periodclosedialog.h"

#include "component/signalblocker.h"
#include "ui_periodclosedialog.h"

PeriodCloseDialog::PeriodCloseDialog(CTreeModel* model, SearchEntryModel* table_model, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PeriodCloseDialog)
    , model_ { model }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(table_model);
    table_model->setParent(ui->tableView);

    InitDialog();
}

PeriodCloseDialog::~PeriodCloseDialog() { delete ui; }

QTableView* PeriodCloseDialog::View() { return ui->tableView; }

void PeriodCloseDialog::InitDialog()
{
    {
        auto* leaf_path_branch_path_model = new ItemModel(this);
        model_->LeafPathBranchPathModel(leaf_path_branch_path_model);
        leaf_path_branch_path_model->sort(0);

        ui->comboBoxFrom->setModel(leaf_path_branch_path_model);
        ui->comboBoxFrom->setCurrentIndex(-1);
    }

    {
        auto* leaf_model { model_->LeafModel() };
        leaf_model->sort(0);

        ui->comboBoxTo->setModel(leaf_model);
        ui->comboBoxTo->setCurrentIndex(-1);
    }
}

QVector<Entry*> PeriodCloseDialog::ConstructEntry(const QSet<Node*>& leaf_node, Node* to_node)
{
    QVector<Entry*> entries;

    if (!to_node || leaf_node.isEmpty())
        return entries;

    for (Node* node : leaf_node) {
        if (!node || FloatEqual(node->final_total, 0.0))
            continue;
    }
}

void PeriodCloseDialog::on_pushButtonPriview_clicked()
{
    if (ui->comboBoxFrom->currentIndex() == -1 || ui->comboBoxTo->currentIndex() == -1)
        return;

    const auto from_id { ui->comboBoxFrom->currentData().toUuid() };
    const auto to_id { ui->comboBoxTo->currentData().toUuid() };

    auto* from_node { model_->GetNode(from_id) };
    auto* to_node { model_->GetNode(to_id) };

    if (!from_node || !to_node)
        return;

    leaf_node_.clear();
    branch_node_.clear();

    {
        QQueue<Node*> queue {};
        queue.enqueue(from_node);

        while (!queue.isEmpty()) {
            Node* current = queue.dequeue();

            if (current->kind == NodeKind::kLeaf) {
                leaf_node_.insert(current);
                continue;
            }

            branch_node_.insert(current);

            for (Node* child : std::as_const(current->children)) {
                queue.enqueue(child);
            }
        }
    }
}
