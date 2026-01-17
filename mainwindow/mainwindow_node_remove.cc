#include <QtWidgets/qmessagebox.h>

#include "dialog/removenode/leafremovedialog.h"
#include "mainwindow.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

void MainWindow::RemoveNode()
{
    const auto index { sc_->tree_view->currentIndex() };
    if (!index.isValid())
        return;

    auto* node { static_cast<Node*>(index.internalPointer()) };
    Q_ASSERT(node);

    auto model { sc_->tree_model };
    Q_ASSERT(model);

    const QUuid node_id { node->id };
    if (node_pending_removal_.contains(node_id))
        return;

    node_pending_removal_.insert(node_id);

    switch (NodeKind(node->kind)) {
    case NodeKind::kBranch: {
        BranchRemove(model, index, node_id);
        break;
    }
    case NodeKind::kLeaf: {
        const auto message { JsonGen::LeafRemoveCheck(sc_->info.section, node_id) };
        WebSocket::Instance()->SendMessage(kLeafRemoveCheck, message);
        break;
    }
    default:
        break;
    }
}

void MainWindow::RLeafRemoveDenied(const QJsonObject& obj)
{
    Section section { obj.value(kSection).toInt() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };

    auto* section_contex = GetSectionContex(section);

    auto model { section_contex->tree_model };
    const auto unit { model->Unit(node_id) };

    auto* dialog { new LeafRemoveDialog(model, section_contex->info, obj, node_id, unit, this) };
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(true);
    dialog->show();

    connect(dialog, &LeafRemoveDialog::SRemoveNode, model, &TreeModel::RRemoveNode, Qt::SingleShotConnection);
    connect(dialog, &QObject::destroyed, this, [this, node_id]() { node_pending_removal_.remove(node_id); });
}

void MainWindow::BranchRemove(TreeModel* tree_model, const QModelIndex& index, const QUuid& node_id)
{
    QMessageBox msg {};
    msg.setIcon(QMessageBox::Question);
    msg.setText(tr("Remove %1").arg(tree_model->Path(node_id)));
    msg.setInformativeText(tr("The branch will be removed, and its direct children will be promoted to the same level."));
    msg.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);

    if (msg.exec() == QMessageBox::Ok) {
        const QUuid parent_id { sc_->tree_model->GetNode(node_id)->parent->id };

        const auto message { JsonGen::BranchRemove(sc_->info.section, node_id, parent_id) };
        WebSocket::Instance()->SendMessage(kBranchRemove, message);
        tree_model->removeRows(index.row(), 1, index.parent());
    }

    node_pending_removal_.remove(node_id);
}
