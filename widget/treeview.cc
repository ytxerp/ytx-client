#include "treeview.h"

#include <QDropEvent>

TreeView::TreeView(QWidget* parent)
    : QTreeView { parent }
{
    setDragDropMode(QAbstractItemView::DragDrop);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::DoubleClicked);
    setContextMenuPolicy(Qt::CustomContextMenu);

    setDropIndicatorShown(true);
    setUniformRowHeights(true);
    setSortingEnabled(true);
    setExpandsOnDoubleClick(false);

    setDragDropOverwriteMode(false);
    setAutoExpandDelay(800);
}

void TreeView::dropEvent(QDropEvent* event)
{
    const QModelIndex target { indexAt(event->position().toPoint()) };
    const DropIndicatorPosition indicator { dropIndicatorPosition() };
    const QPersistentModelIndex persistent_target { target };

    QTreeView::dropEvent(event);

    if (persistent_target.isValid() && indicator == QAbstractItemView::OnItem)
        expand(persistent_target);
}
