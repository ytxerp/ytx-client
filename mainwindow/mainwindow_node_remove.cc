#include "component/constantwebsocket.h"
#include "dialog/deletenode/leafdeletedialog.h"
#include "mainwindow.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

void MainWindow::DeleteNode()
{
    const auto index { sc_->tree_view->currentIndex() };
    if (!index.isValid())
        return;

    auto* node { static_cast<Node*>(index.internalPointer()) };
    if (!node) {
        return;
    }

    if (node->sync_state == SyncState::kDeleting)
        return;

    if (start_ == Section::kSale || start_ == Section::kPurchase) {
        auto* d_node { static_cast<NodeO*>(node) };

        if (d_node->status == NodeStatus::kReleased) {
            utils::ShowMessage(QMessageBox::Information, tr("Operation Rejected"),
                tr("This order has been released and cannot be deleted.\n"
                   "Please recall it before deleting."),
                time_const::kAutoCloseMs);

            return;
        }
    }

    node->sync_state = SyncState::kDeleting;

    const QUuid node_id { node->id };

    switch (NodeKind(node->kind)) {
    case NodeKind::kBranch:
        DeleteBranch(node_id);
        break;
    case NodeKind::kLeaf: {
        const auto message { JsonGen::LeafDeleteCheck(sc_->info.section, node_id) };
        WebSocket::Instance()->SendMessage(WsKey::kLeafDeleteCheck, message);
        break;
    }
    }
}

void MainWindow::RDenyLeafDelete(const QJsonObject& obj)
{
    Section section { obj.value(kSection).toInt() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };

    auto* section_contex = GetSectionContex(section);

    auto model { section_contex->tree_model };
    const auto unit { model->Unit(node_id) };

    auto* dialog { new LeafDeleteDialog(model, section_contex->info, obj, node_id, unit, this) };

    utils::ManageDialog(sc_->widget_hash, dialog);
    dialog->setWindowModality(Qt::WindowModal);

    connect(dialog, &QDialog::rejected, this, [=] {
        if (auto* node = model->GetNode(node_id))
            node->sync_state = SyncState::kSynced;
    });

    dialog->show();
}

void MainWindow::DeleteBranch(const QUuid& node_id)
{
    auto tree_model { sc_->tree_model };
    Q_ASSERT(tree_model != nullptr);

    auto* dlg = utils::CreateMessage(QMessageBox::Question, tr("Confirm Delete"),
        tr("The branch \"%1\" will be permanently deleted. Its direct children will be promoted to the same level.").arg(tree_model->Path(node_id)), true,
        QMessageBox::Yes | QMessageBox::Cancel, this);

    dlg->setDefaultButton(QMessageBox::Cancel);

    QObject::connect(dlg, &QMessageBox::finished, this, [this, node_id, tree_model](int ret) {
        if (ret == QMessageBox::Yes) {
            const QUuid parent_id { tree_model->GetNode(node_id)->parent->id };
            const auto message { JsonGen::BranchDelete(sc_->info.section, node_id, parent_id) };
            WebSocket::Instance()->SendMessage(WsKey::kBranchDelete, message);
        }
    });

    connect(dlg, &QDialog::rejected, this, [=] {
        if (auto* node = tree_model->GetNode(node_id))
            node->sync_state = SyncState::kSynced;
    });

    dlg->show();
}
