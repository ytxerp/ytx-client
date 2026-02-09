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

    auto model { sc_->tree_model };
    Q_ASSERT(model != nullptr);

    const QUuid node_id { node->id };
    if (node_pending_deletion_.contains(node_id))
        return;

    node_pending_deletion_.insert(node_id);

    switch (NodeKind(node->kind)) {
    case NodeKind::kBranch: {
        DeleteBranch(model, index, node_id);
        break;
    }
    case NodeKind::kLeaf: {
        const auto message { JsonGen::LeafDeleteCheck(sc_->info.section, node_id) };
        WebSocket::Instance()->SendMessage(kLeafDeleteCheck, message);
        break;
    }
    default:
        break;
    }
}

void MainWindow::RLeafDeleteDenied(const QJsonObject& obj)
{
    Section section { obj.value(kSection).toInt() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };

    auto* section_contex = GetSectionContex(section);

    auto model { section_contex->tree_model };
    const auto unit { model->Unit(node_id) };

    auto* dialog { new LeafDeleteDialog(model, section_contex->info, obj, node_id, unit, this) };

    Utils::ManageDialog(sc_->dialog_hash, dialog);
    dialog->setModal(true);

    connect(dialog, &QDialog::destroyed, this, [this, node_id]() { node_pending_deletion_.remove(node_id); });
    dialog->show();
}

void MainWindow::DeleteBranch(TreeModel* tree_model, const QModelIndex& index, const QUuid& node_id)
{
    auto* dlg = Utils::CreateMessageBox(QMessageBox::Question, tr("Delete %1").arg(tree_model->Path(node_id)),
        tr("The branch will be deleted, and its direct children will be promoted to the same level."), true, QMessageBox::Ok | QMessageBox::Cancel, this);

    dlg->setDefaultButton(QMessageBox::Cancel);

    QObject::connect(dlg, &QMessageBox::finished, this, [=, this](int ret) {
        if (ret == QMessageBox::Ok) {
            const QUuid parent_id { tree_model->GetNode(node_id)->parent->id };
            const auto message { JsonGen::BranchDelete(sc_->info.section, node_id, parent_id) };
            WebSocket::Instance()->SendMessage(kBranchDelete, message);
            tree_model->removeRows(index.row(), 1, index.parent());
        }
    });

    // ✅ Resource cleanup - ensure pending state is always cleared
    QObject::connect(dlg, &QMessageBox::destroyed, this, [this, node_id]() { node_pending_deletion_.remove(node_id); });

    dlg->show();
}
