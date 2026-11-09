#include "treemodelr.h"

#include "component/constantwebsocket.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TreeModelR::TreeModelR(Section section, const QStringList& header, CString& separator, QObject* parent)
    : TreeModel(section, header, separator, parent)
{
    leaf_model_ = new ItemModel(this);
}

QVariant TreeModelR::data(const QModelIndex& index, int role) const { }

bool TreeModelR::setData(const QModelIndex& index, const QVariant& value, int role) { }

void TreeModelR::sort(int column, Qt::SortOrder order) { }

Qt::ItemFlags TreeModelR::flags(const QModelIndex& index) const { }

bool TreeModelR::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) { }

void TreeModelR::UpdateAmount(QUuid node_id, double initial_delta) { }

QSet<QUuid> TreeModelR::UpdateAncestorTotal(Node* node, const node::Delta& delta) const { }

void TreeModelR::InitAncestorTotal(Node* node, const node::Delta& delta) const { }
