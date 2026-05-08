#include "periodclosedialog.h"

#include "component/signalblocker.h"
#include "global/entrypool.h"
#include "ui_periodclosedialog.h"
#include "websocket/websocket.h"

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
    EntryPool::Instance().Recycle(entry_list_, section_);
    delete ui;
}

QTableView* PeriodCloseDialog::View() { return ui->tableView; }

void PeriodCloseDialog::InitDialog()
{
    {
        auto* leaf_path_branch_path_model = new ItemModel(this);
        tree_model_->LeafPathBranchPathModel(leaf_path_branch_path_model);
        leaf_path_branch_path_model->sort(0);

        ui->comboBoxClosing->setModel(leaf_path_branch_path_model);
        ui->comboBoxClosing->setCurrentIndex(-1);
    }

    {
        auto* leaf_model { tree_model_->LeafModel() };
        leaf_model->sort(0);

        ui->comboBoxSummary->setModel(leaf_model);
        ui->comboBoxSummary->setCurrentIndex(-1);
    }
}

void PeriodCloseDialog::ConstructEntry(const QSet<Node*>& closing_leaf_node, const Node* summary_node)
{
    if (!summary_node || closing_leaf_node.isEmpty())
        return;

    entry_list_.reserve(closing_leaf_node.size());
    const auto date_time { QDateTime::currentDateTimeUtc() };

    for (const Node* node : closing_leaf_node) {
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
        entry->rhs_node = summary_node->id;
        entry->issued_time = date_time;
        entry->id = QUuid::createUuidV7();

        const double amount { node->final_total };

        if (node->direction_rule == Rule::kDICD) {
            entry->lhs_credit = amount;
            entry->rhs_debit = amount;
        } else {
            entry->lhs_debit = amount;
            entry->rhs_credit = amount;
        }

        entry_list_.emplaceBack(entry);
    }
}

void PeriodCloseDialog::ResetState()
{
    closing_leaf_node_.clear();
    EntryPool::Instance().Recycle(entry_list_, section_);
}

QJsonArray PeriodCloseDialog::BuildUuidArray(const QSet<Node*>& set)
{
    QJsonArray array {};

    for (const auto* node : set) {
        if (!node) {
            qWarning() << Q_FUNC_INFO << "Null node* encountered";
            continue;
        }

        if (node->id.isNull()) {
            qWarning() << Q_FUNC_INFO << "Node with null UUID encountered";
            continue;
        }

        array.append(node->id.toString(QUuid::WithoutBraces));
    }

    return array;
}

void PeriodCloseDialog::on_pushButtonPreview_clicked()
{
    if (ui->comboBoxClosing->currentIndex() == -1 || ui->comboBoxSummary->currentIndex() == -1)
        return;

    const auto closing_node_id { ui->comboBoxClosing->currentData().toUuid() };
    summary_node_id_ = ui->comboBoxSummary->currentData().toUuid();

    auto* closing_node { tree_model_->GetNode(closing_node_id) };
    auto* summary_node { tree_model_->GetNode(summary_node_id_) };

    if (!closing_node || !summary_node || closing_node == summary_node)
        return;

    ResetState();

    {
        QQueue<Node*> queue {};
        queue.enqueue(closing_node);

        while (!queue.isEmpty()) {
            Node* current { queue.dequeue() };

            if (!current) {
                qWarning() << "Null node current";
                continue;
            }

            if (current->kind == NodeKind::kLeaf) {
                if (!FloatEqual(current->final_total, 0.0)) {
                    closing_leaf_node_.insert(current);
                }

                continue;
            }

            for (Node* child : std::as_const(current->children)) {
                queue.enqueue(child);
            }
        }
    }

    {
        ConstructEntry(closing_leaf_node_, summary_node);
        table_model_->ResetModel(entry_list_);
    }
}

void PeriodCloseDialog::on_pushButtonCommit_clicked()
{
    QJsonObject message {};

    QJsonObject summary_total {};
    summary_total.insert(kId, summary_node_id_.toString(QUuid::WithoutBraces));

    message.insert(kSummaryTotal, summary_total);
    message.insert(kClosingLeafNode, BuildUuidArray(closing_leaf_node_));
    message.insert(kEntryArray, QJsonArray());
    message.insert(kTotalArray, QJsonArray());
    message.insert(kSection, std::to_underlying(section_));

    WebSocket::Instance()->SendMessage(WsKey::kPeriodClose, message);

    ResetState();
    table_model_->ResetModel({});
}
