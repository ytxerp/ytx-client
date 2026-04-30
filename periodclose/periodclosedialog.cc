#include "periodclosedialog.h"

#include "component/signalblocker.h"
#include "global/entrypool.h"
#include "ui_periodclosedialog.h"

PeriodCloseDialog::PeriodCloseDialog(Section section, CTreeModel* tree_model, PeriodCloseModel* table_model, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PeriodCloseDialog)
    , section_ { section }
    , tree_model_ { tree_model }
    , table_model_ { table_model }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(table_model);
    table_model->setParent(ui->tableView);

    InitDialog();
}

PeriodCloseDialog::~PeriodCloseDialog()
{
    EntryPool::Instance().Recycle(list_, section_);
    delete ui;
}

QTableView* PeriodCloseDialog::View() { return ui->tableView; }

void PeriodCloseDialog::InitDialog()
{
    {
        auto* leaf_path_branch_path_model = new ItemModel(this);
        tree_model_->LeafPathBranchPathModel(leaf_path_branch_path_model);
        leaf_path_branch_path_model->sort(0);

        ui->comboBoxFrom->setModel(leaf_path_branch_path_model);
        ui->comboBoxFrom->setCurrentIndex(-1);
    }

    {
        auto* leaf_model { tree_model_->LeafModel() };
        leaf_model->sort(0);

        ui->comboBoxTo->setModel(leaf_model);
        ui->comboBoxTo->setCurrentIndex(-1);
    }
}

void PeriodCloseDialog::ConstructEntry(const QSet<Node*>& leaf_node, const Node* to_node)
{
    if (!to_node || leaf_node.isEmpty())
        return;

    list_.reserve(leaf_node.size());
    const auto date_time { QDateTime::currentDateTimeUtc() };

    for (const Node* node : leaf_node) {
        if (!node) {
            qWarning() << "Null node encountered";
            continue;
        }

        if (FloatEqual(node->final_total, 0.0))
            continue;

        auto* entry { EntryPool::Instance().Allocate(section_) };
        entry->lhs_rate = 1;
        entry->rhs_rate = 1;
        entry->lhs_node = node->id;
        entry->rhs_node = to_node->id;
        entry->issued_time = date_time;

        const double amount { node->final_total };

        if (node->direction_rule == Rule::kDICD) {
            entry->lhs_credit = amount;
            entry->rhs_debit = amount;
        } else {
            entry->lhs_debit = amount;
            entry->rhs_credit = amount;
        }

        list_.emplaceBack(entry);
    }
}

void PeriodCloseDialog::on_pushButtonPreview_clicked()
{
    if (ui->comboBoxFrom->currentIndex() == -1 || ui->comboBoxTo->currentIndex() == -1)
        return;

    const auto from_id { ui->comboBoxFrom->currentData().toUuid() };
    const auto to_id { ui->comboBoxTo->currentData().toUuid() };

    auto* from_node { tree_model_->GetNode(from_id) };
    auto* to_node { tree_model_->GetNode(to_id) };

    if (!from_node || !to_node || from_node == to_node)
        return;

    // Reset state
    {
        leaf_node_.clear();
        branch_node_.clear();
        EntryPool::Instance().Recycle(list_, section_);
    }

    {
        QQueue<Node*> queue {};
        queue.enqueue(from_node);

        while (!queue.isEmpty()) {
            Node* current { queue.dequeue() };

            if (!current) {
                qWarning() << "Null node current";
                continue;
            }

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

    {
        ConstructEntry(leaf_node_, to_node);
        table_model_->ResetModel(list_);
    }
}

void PeriodCloseDialog::on_pushButtonCommit_clicked() { }
