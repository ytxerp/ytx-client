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

    switch (NodeKind(node->kind)) {
    case NodeKind::kBranch:
        DeleteBranch(node);
        break;
    case NodeKind::kLeaf: {
        const auto message { JsonGen::LeafDeleteCheck(sc_->info.section, node->id, node->version) };
        WebSocket::Instance()->SendMessage(WsKey::kLeafDeleteCheck, message);
        break;
    }
    }
}

void MainWindow::RDenyLeafDelete(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QUuid node_id { obj.value(kNodeId).toString() };

    auto* section_contex { GetSectionContex(section) };
    auto model { section_contex->tree_model };
    auto* node { model->GetNode(node_id) };

    if (!node || !node->IsValid())
        return;

    auto* dialog { new LeafDeleteDialog(model, section_contex->info, obj, node, this) };

    utils::ManageDialog(sc_->widget_hash, dialog);
    dialog->setWindowModality(Qt::WindowModal);

    connect(dialog, &QDialog::rejected, this, [=] { node->sync_state = SyncState::kSynced; });
    dialog->show();
}

void MainWindow::DeleteBranch(Node* node)
{
    auto tree_model { sc_->tree_model };
    Q_ASSERT(tree_model != nullptr);

    if (!node || !node->IsValid())
        return;

    auto* dlg = utils::CreateMessage(QMessageBox::Question, tr("Confirm Delete"),
        tr("The branch \"%1\" will be permanently deleted. Its direct children will be promoted to the same level.").arg(tree_model->Path(node->id)), true,
        QMessageBox::Yes | QMessageBox::Cancel, this);

    dlg->setDefaultButton(QMessageBox::Cancel);

    QObject::connect(dlg, &QMessageBox::finished, this, [this, node](int ret) {
        if (ret == QMessageBox::Yes) {
            if (!node || !node->IsValid())
                return;

            const auto message { JsonGen::BranchDelete(sc_->info.section, node->id, node->parent->id, node->version) };
            WebSocket::Instance()->SendMessage(WsKey::kBranchDelete, message);
        }
    });

    connect(dlg, &QDialog::rejected, this, [=] {
        if (!node || !node->IsValid())
            return;

        node->sync_state = SyncState::kSynced;
    });

    dlg->show();
}
