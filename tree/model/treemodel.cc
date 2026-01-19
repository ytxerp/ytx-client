#include "treemodel.h"

#include <QJsonArray>
#include <QQueue>

#include "global/nodepool.h"
#include "global/resourcepool.h"
#include "tree/excludeidfiltermodel.h"
#include "tree/excludeidunitfiltermodel.h"
#include "tree/includeunitfiltermodel.h"
#include "tree/replaceselffiltermodel.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TreeModel::TreeModel(CSectionInfo& info, CString& separator, QObject* parent)
    : QAbstractItemModel(parent)
    , separator_ { separator }
    , section_ { info.section }
    , node_header_ { info.node_header }
{
    InitRoot(root_);
}

TreeModel::~TreeModel()
{
    FlushCaches();
    NodePool::Instance().Recycle(node_hash_, section_);
}

void TreeModel::RRemoveNode(const QUuid& node_id)
{
    if (!node_hash_.contains(node_id)) {
        return;
    }

    auto index { GetIndex(node_id) };
    removeRows(index.row(), 1, index.parent());
}

void TreeModel::SyncTotalArray(const QJsonArray& total_array)
{
    if (total_array.isEmpty())
        return;

    QSet<QUuid> affected_ids {};

    for (const auto& delta : total_array) {
        const QJsonObject obj { delta.toObject() };

        Q_ASSERT_X(obj.contains(kId), "TreeModel::UpdateDelta", "Missing kId in delta object");
        Q_ASSERT_X(obj.contains(kInitialTotal), "TreeModel::UpdateDelta", "Missing kInitialDelta in delta object");
        Q_ASSERT_X(obj.contains(kFinalTotal), "TreeModel::UpdateDelta", "Missing kFinalDelta in delta object");

        const QUuid node_id { QUuid(obj.value(kId).toString()) };
        const double initial_total { obj.value(kInitialTotal).toString().toDouble() };
        const double final_total { obj.value(kFinalTotal).toString().toDouble() };

        const auto ids { UpdateTotal(node_id, initial_total, final_total) };
        affected_ids.unite(ids);
    }

    RefreshAffectedTotal(affected_ids);
}

void TreeModel::InsertNode(const QUuid& ancestor, const QJsonObject& data)
{
    Node* parent { node_hash_.value(ancestor) };
    if (!parent)
        return;

    auto* node { NodePool::Instance().Allocate(section_) };
    node->ReadJson(data);

    const auto row { parent->children.size() };

    auto parent_index { GetIndex(parent->id) };

    beginInsertRows(parent_index, row, row);
    parent->children.insert(row, node);
    node->parent = parent;
    endInsertRows();

    node_hash_.insert(node->id, node);
    RegisterPath(node);
    SortModel();
}

QSet<QUuid> TreeModel::UpdateTotal(const QUuid& node_id, double initial_total, double final_total)
{
    auto* node = GetNode(node_id);
    if (!node)
        return {};

    const double initial_delta { initial_total - node->initial_total };
    const double final_delta { final_total - node->final_total };

    // Accumulate into the current node totals
    node->initial_total = initial_total;
    node->final_total = final_total;

    // Propagate adjusted deltas to ancestor nodes
    auto affected_ids { UpdateAncestorTotal(node, initial_delta, final_delta) };
    affected_ids.insert(node_id);

    emit SSyncValue();

    return affected_ids;
}

void TreeModel::InsertMeta(const QUuid& node_id, const QJsonObject& meta)
{
    auto* node = GetNode(node_id);
    if (!node)
        return;

    InsertMeta(node, meta);
}

void TreeModel::SyncNode(const QUuid& node_id, const QJsonObject& update)
{
    if (update.isEmpty()) {
        qInfo().noquote() << "SyncNode ignored: empty data for node" << node_id.toString(QUuid::WithoutBraces);
        return;
    }

    auto* node = GetNode(node_id);
    if (!node) {
        qInfo().noquote() << "SyncNode ignored: node not found in local node_hash_, id =" << node_id.toString(QUuid::WithoutBraces);
        return;
    }

    node->ReadJson(update);

    auto index { GetIndex(node_id) };
    if (index.isValid()) {
        const auto [start, end] = Utils::NodeCacheColumnRange(section_);
        if (end != -1)
            emit dataChanged(index.siblingAtColumn(start), index.siblingAtColumn(end));
    }
}

void TreeModel::UpdateMeta(const QUuid& node_id, const QJsonObject& meta)
{
    auto* node = GetNode(node_id);
    if (!node)
        return;

    UpdateMeta(node, meta);
}

void TreeModel::UpdateMeta(Node* node, const QJsonObject& meta)
{
    Q_ASSERT_X(meta.contains(kUpdatedBy), "TreeModel::UpdateMeta", "Missing 'updated_by' in meta");
    Q_ASSERT_X(meta.contains(kUpdatedTime), "TreeModel::UpdateMeta", "Missing 'updated_time' in meta");

    node->updated_time = QDateTime::fromString(meta[kUpdatedTime].toString(), Qt::ISODate);
    node->updated_by = QUuid(meta[kUpdatedBy].toString());
}

void TreeModel::InsertMeta(Node* node, const QJsonObject& meta)
{
    Q_ASSERT_X(meta.contains(kUserId), "TreeModel::InsertMeta", "Missing 'user_id' in meta");
    Q_ASSERT_X(meta.contains(kCreatedTime), "TreeModel::InsertMeta", "Missing 'created_time' in meta");
    Q_ASSERT_X(meta.contains(kCreatedBy), "TreeModel::InsertMeta", "Missing 'created_by' in meta");

    node->user_id = QUuid(meta[kUserId].toString());
    node->created_time = QDateTime::fromString(meta[kCreatedTime].toString(), Qt::ISODate);
    node->created_by = QUuid(meta[kCreatedBy].toString());
}

void TreeModel::UpdateDirectionRule(Node* node, bool value)
{
    if (node->direction_rule == value)
        return;

    QJsonObject message { JsonGen::NodeDirectionRule(section_, node->id, value) };
    WebSocket::Instance()->SendMessage(kDirectionRule, message);

    DirectionRuleImpl(node, value);
}

void TreeModel::SyncDirectionRule(const QUuid& node_id, bool direction_rule)
{
    auto* node = GetNode(node_id);
    if (!node)
        return;

    DirectionRuleImpl(node, direction_rule);
}

void TreeModel::DirectionRuleImpl(Node* node, bool value)
{
    node->InvertTotal();
    node->direction_rule = value;

    const QUuid node_id { node->id };

    if (node->kind == NodeKind::kLeaf) {
        emit SDirectionRule(node_id, node->direction_rule);
    }

    const int rule_column { Utils::DirectionRuleColumn(section_) };
    EmitRowChanged(node_id, rule_column, rule_column);

    const auto [start_col, end_col] = Utils::NodeNumericColumnRange(section_);
    EmitRowChanged(node_id, start_col, end_col);

    emit SSyncValue();
}

void TreeModel::ReplaceLeaf(const QUuid& old_node_id, const QUuid& new_node_id)
{
    auto* old_node { GetNode(old_node_id) };
    auto* new_node { GetNode(new_node_id) };

    if (!old_node || !new_node)
        return;

    const int multiplier { old_node->direction_rule == new_node->direction_rule ? 1 : -1 };
    const double initial_delta { multiplier * old_node->initial_total };
    const double final_delta { multiplier * old_node->final_total };

    new_node->initial_total += initial_delta;
    new_node->final_total += final_delta;

    const auto affected_ids { UpdateAncestorTotal(new_node, initial_delta, final_delta) };
    RefreshAffectedTotal(affected_ids);

    RRemoveNode(old_node_id);
}

void TreeModel::UpdateName(const QUuid& node_id, const QString& name)
{
    auto* node = GetNode(node_id);
    if (!node)
        return;

    if (node->name == name)
        return;

    node->name = name;

    Utils::UpdatePath(leaf_path_, branch_path_, root_, node, separator_);
    Utils::UpdateModel(leaf_path_, leaf_path_model_, node);

    const int name_column { std::to_underlying(NodeEnum::kName) };

    EmitRowChanged(node_id, name_column, name_column);
    emit SResizeColumnToContents(name_column);
    emit SUpdateName(node->id, node->name, node->kind == NodeKind::kBranch);
}

void TreeModel::DragNode(const QUuid& ancestor, const QUuid& descendant)
{
    // Get the node to be moved
    auto* node { GetNode(descendant) };
    if (!node) {
        qWarning() << "DragNode: descendant node not found, skip move";
        return;
    }

    // Get destination parent node
    auto* destination_node { GetNode(ancestor) };
    if (!destination_node) {
        qWarning() << "DragNode: destination node not found, skip move";
        return;
    }

    // Get source parent node
    Node* source_parent_node { node->parent };
    if (!source_parent_node) {
        qWarning() << "DragNode: node has no source parent (top-level or detached), skip move";
        return;
    }

    // Same parent, nothing to do
    if (source_parent_node == destination_node) {
        qWarning() << "DragNode: source and destination parent are the same, skip move";
        return;
    }

    // Check for circular dependency (destination is descendant of node)
    if (Utils::IsDescendant(destination_node, node)) {
        qWarning() << "DragNode: cannot move node to its descendant, skip move";
        return;
    }

    // Calculate the destination row (insert at the end)
    const qsizetype destination_child { destination_node->children.size() };
    const QModelIndex destination_parent_index { GetIndex(ancestor) };
    if (!destination_parent_index.isValid()) {
        qInfo() << "DragNode: moving node to top-level";
    }

    // Find the row of the node in its parent's children list
    const qsizetype source_row { source_parent_node->children.indexOf(node) };
    if (source_row == -1) {
        qWarning() << "DragNode: node not found in source parent's children, skip move";
        return;
    }

    // Get the source parent index
    QModelIndex source_parent_index { GetIndex(source_parent_node->id) };
    if (!source_parent_index.isValid()) {
        qInfo() << "DragNode: moving top-level node";
    }

    // Perform the row move
    if (!moveRows(source_parent_index, source_row, 1, destination_parent_index, destination_child)) {
        qWarning() << "DragNode: moveRows failed";
    }
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
    // root_'s index is QModelIndex(), root_'s id == -1
    if (!index.isValid())
        return QModelIndex();

    auto* node { static_cast<Node*>(index.internalPointer()) };
    if (!node) {
        qWarning() << "parent: invalid internal pointer in index";
        return QModelIndex();
    }

    // Node has no parent or parent is root
    auto* parent_node { node->parent };
    if (!parent_node || parent_node == root_)
        return QModelIndex();

    // Parent node should have a parent (grandparent)
    if (!parent_node->parent) {
        qWarning() << "parent: parent_node has no parent (should be root)";
        return QModelIndex();
    }

    // Find parent's row in grandparent's children
    const qsizetype row { parent_node->parent->children.indexOf(parent_node) };
    if (row == -1) {
        qWarning() << "parent: parent_node not found in grandparent's children";
        return QModelIndex();
    }

    return createIndex(row, 0, parent_node);
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    auto* parent_node { GetNodeByIndex(parent) };
    if (!parent_node) {
        qWarning() << "index: parent node not found";
        return QModelIndex();
    }

    // Check row is within bounds
    if (row < 0 || row >= parent_node->children.size()) {
        qWarning() << "index: row out of bounds"
                   << "row:" << row << "children size:" << parent_node->children.size();
        return QModelIndex();
    }

    auto* node { parent_node->children.at(row) };
    if (!node) {
        qWarning() << "index: child node at row" << row << "is null";
        return QModelIndex();
    }

    return createIndex(row, column, node);
}

int TreeModel::rowCount(const QModelIndex& parent) const { return GetNodeByIndex(parent)->children.size(); }

QMimeData* TreeModel::mimeData(const QModelIndexList& indexes) const
{
    auto* mime_data { new QMimeData() };
    if (indexes.isEmpty())
        return mime_data;

    auto first_index { indexes.first() };

    if (first_index.isValid()) {
        const QUuid id { first_index.sibling(first_index.row(), std::to_underlying(NodeEnum::kId)).data().toUuid() };
        mime_data->setData(kYTX, id.toRfc4122());
    }

    return mime_data;
}

bool TreeModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0 || row > rowCount(parent) - 1) {
        qCritical() << "removeRows: row out of range";
        return false;
    }

    if (count != 1) {
        qCritical() << "removeRows: Only support removing one row, count =" << count;
        return false;
    }

    auto* parent_node { GetNodeByIndex(parent) };
    auto* node { parent_node->children.at(row) };

    const auto node_id { node->id };

    beginRemoveRows(parent, row, row);
    parent_node->children.removeOne(node);
    endRemoveRows();

    RemovePath(node, parent_node);

    ResourcePool<Node>::Instance().Recycle(node);
    node_hash_.remove(node_id);

    emit SSyncValue();
    emit SResizeColumnToContents(std::to_underlying(NodeEnum::kName));
    emit SFreeWidget(section_, node_id);

    return true;
}

bool TreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    auto* destination_parent { GetNodeByIndex(parent) };
    if (!destination_parent) {
        qWarning() << "dropMimeData: destination parent not found";
        return false;
    }

    if (destination_parent->kind != NodeKind::kBranch)
        return false;

    const auto mime { data->data(kYTX) };
    if (mime.isEmpty()) {
        qWarning() << "dropMimeData: MIME data is empty";
        return false;
    }

    const QUuid node_id { QUuid::fromRfc4122(mime) };
    if (node_id.isNull()) {
        qWarning() << "dropMimeData: invalid UUID in MIME data";
        return false;
    }

    auto* node { node_hash_.value(node_id) };
    if (!node) {
        qWarning() << "dropMimeData: node not found (possibly deleted during drag)" << node_id;
        return false;
    }

    qInfo() << "[UI] dropMimeData";

    if (node->parent == destination_parent || Utils::IsDescendant(destination_parent, node))
        return false;

    int destination_child { row };
    if (row == -1) {
        destination_child = destination_parent->children.size();
    }
    destination_child = qBound(0, destination_child, destination_parent->children.size());

    auto* source_parent_node { node->parent };
    if (!source_parent_node) {
        qWarning() << "dropMimeData: source parent is null (node was orphaned during drag)";
        return false;
    }

    qsizetype source_row { source_parent_node->children.indexOf(node) };
    if (source_row == -1) {
        qCritical() << "dropMimeData: source row not found for node:" << node->id;
        return false;
    }

    QModelIndex source_parent_index { GetIndex(source_parent_node->id) };

    if (moveRows(source_parent_index, source_row, 1, parent, destination_child)) {
        const auto message { JsonGen::NodeDrag(section_, node_id, destination_parent->id) };
        WebSocket::Instance()->SendMessage(kNodeDrag, message);
        return true;
    }

    return false;
}

bool TreeModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild)
{
    if (sourceParent == destinationParent) {
        qWarning() << "moveRows: same parent move is not supported";
        return false;
    }

    auto* source_parent { GetNodeByIndex(sourceParent) };
    auto* destination_parent { GetNodeByIndex(destinationParent) };

    Q_ASSERT_X(source_parent, "TreeModel::moveRows", "Source parent is null");
    Q_ASSERT_X(destination_parent, "TreeModel::moveRows", "Destination parent is null");
    Q_ASSERT_X(count == 1, "TreeModel::moveRows", "Only single-row move is supported");
    Q_ASSERT_X(sourceRow >= 0 && sourceRow < source_parent->children.size(), "TreeModel::moveRows", "Source row is out of bounds");
    Q_ASSERT_X(destinationChild >= 0 && destinationChild <= destination_parent->children.size(), "TreeModel::moveRows", "Destination child is out of bounds");

    if (!beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent, destinationChild)) {
        qWarning() << "moveRows: beginMoveRows failed - invalid move operation";
        return false;
    }

    auto* node { source_parent->children.takeAt(sourceRow) };
    if (!node) {
        qCritical() << "moveRows: Node extraction failed!";
        return false;
    }

    const auto affected_ids_source { UpdateAncestorTotal(node, -node->initial_total, -node->final_total) };

    destination_parent->children.insert(destinationChild, node);
    node->parent = destination_parent;

    auto affected_ids_destination { UpdateAncestorTotal(node, node->initial_total, node->final_total) };
    endMoveRows();

    RefreshAffectedTotal(affected_ids_destination.unite(affected_ids_source));

    Utils::UpdatePath(leaf_path_, branch_path_, root_, node, separator_);
    Utils::UpdateModel(leaf_path_, leaf_path_model_, node);

    emit SUpdateName(node->id, node->name, node->kind == NodeKind::kBranch);
    emit SResizeColumnToContents(std::to_underlying(NodeEnum::kName));

    return true;
}

void TreeModel::LeafPathBranchPathModel(ItemModel* model) const { Utils::LeafPathBranchPathModel(leaf_path_, branch_path_, model); }

void TreeModel::UpdateSeparator(CString& old_separator, CString& new_separator)
{
    Q_ASSERT(!new_separator.isEmpty());
    Q_ASSERT(!old_separator.isEmpty());

    if (old_separator == new_separator)
        return;

    Utils::UpdatePathSeparator(old_separator, new_separator, leaf_path_);
    Utils::UpdatePathSeparator(old_separator, new_separator, branch_path_);

    leaf_path_model_->UpdateSeparator(old_separator, new_separator);
}

void TreeModel::SearchNode(QList<Node*>& node_list, CString& name) const
{
    if (node_hash_.size() >= 50) {
        node_list.reserve(node_hash_.size() / 2);
    }

    for (const auto& [id, node_ptr] : node_hash_.asKeyValueRange()) {
        if (!node_ptr)
            continue;

        if (node_ptr->name.contains(name, Qt::CaseInsensitive)) {
            node_list.emplaceBack(node_ptr);
        }
    }
}

void TreeModel::Reset()
{
    FlushCaches();

    {
        leaf_path_.clear();
        branch_path_.clear();

        if (leaf_path_model_)
            leaf_path_model_->Reset();
    }

    {
        beginResetModel();

        NodePool::Instance().Recycle(node_hash_, section_);

        root_ = nullptr;
        InitRoot(root_);

        endResetModel();
    }
}

QModelIndex TreeModel::GetIndex(const QUuid& node_id) const
{
    // Return an invalid index if the node_id is null
    if (node_id.isNull())
        return QModelIndex();

    // Look up the node in the hash table
    Node* node { node_hash_.value(node_id, nullptr) };
    if (!node) {
        qCritical() << "GetIndex: node_id not found in node_hash_";
        return QModelIndex(); // Node not found → return invalid index
    }

    // If the node has no parent, it is a root node → return invalid index
    if (!node->parent)
        return QModelIndex();

    // Find the row of this node in its parent's children list
    auto row { node->parent->children.indexOf(node) };
    if (row == -1) {
        qCritical() << "GetIndex: node not found in parent's children list";
        return QModelIndex(); // Data inconsistency → return invalid index
    }

    // Create and return the QModelIndex for this node (single column model)
    return createIndex(row, 0, node);
}

QString TreeModel::Path(const QUuid& node_id) const
{
    if (auto it = leaf_path_.constFind(node_id); it != leaf_path_.constEnd())
        return it.value();

    if (auto it = branch_path_.constFind(node_id); it != branch_path_.constEnd())
        return it.value();

    return {};
}

QSortFilterProxyModel* TreeModel::ExcludeId(const QUuid& node_id, QObject* parent)
{
    auto* model { new ExcludeIdFilterModel(node_id, parent) };
    model->setSourceModel(leaf_path_model_);
    return model;
}

QSortFilterProxyModel* TreeModel::IncludeUnit(NodeUnit unit, QObject* parent)
{
    auto* set { UnitSet(unit) };
    auto* model { new IncludeUnitFilterModel(set, parent) };
    model->setSourceModel(leaf_path_model_);
    return model;
}

QSortFilterProxyModel* TreeModel::ExcludeIdUnit(const QUuid& node_id, NodeUnit unit, QObject* parent)
{
    auto* set { UnitSet(unit) };
    auto* model { new ExcludeIdUnitFilterModel(node_id, set, parent) };
    model->setSourceModel(leaf_path_model_);
    return model;
}

QSortFilterProxyModel* TreeModel::ReplaceSelf(const QUuid& node_id, NodeUnit unit, QObject* parent)
{
    auto* set { UnitSet(unit) };
    auto* model { new ReplaceSelfFilterModel(node_id, set, parent) };
    model->setSourceModel(leaf_path_model_);
    return model;
}

void TreeModel::AckNode(const QUuid& node_id) const
{
    if (node_hash_.contains(node_id))
        return;

    const auto message { JsonGen::NodeAcked(section_, node_id) };
    WebSocket::Instance()->SendMessage(kNodeAcked, message);
}

void TreeModel::RemovePath(Node* node, Node* parent_node)
{
    const auto node_id { node->id };
    const NodeKind kind { node->kind };

    switch (kind) {
    case NodeKind::kBranch: {
        for (auto* child : std::as_const(node->children)) {
            child->parent = parent_node;
            parent_node->children.emplace_back(child);
        }

        Utils::UpdatePath(leaf_path_, branch_path_, root_, node, separator_);
        Utils::UpdateModel(leaf_path_, leaf_path_model_, node);

        branch_path_.remove(node_id);
        emit SUpdateName(node_id, node->name, true);

    } break;
    case NodeKind::kLeaf: {
        leaf_path_.remove(node_id);
        Utils::RemoveItem(leaf_path_model_, node_id);

        const auto affected_ids { UpdateAncestorTotal(node, -node->initial_total, -node->final_total) };
        RefreshAffectedTotal(affected_ids);

        RemoveUnitSet(node_id, node->unit);
    } break;
    default:
        break;
    }
}

void TreeModel::HandleNode()
{
    for (auto* node : std::as_const(node_hash_)) {
        RegisterPath(node);

        if (node->kind == NodeKind::kLeaf)
            UpdateAncestorTotal(node, node->initial_total, node->final_total);
    }

    SortModel();
}

Node* TreeModel::GetNodeByIndex(const QModelIndex& index) const
{
    if (index.isValid() && index.internalPointer())
        return static_cast<Node*>(index.internalPointer());

    return root_;
}

QSet<QUuid> TreeModel::UpdateAncestorTotal(Node* node, double initial_delta, double final_delta)
{
    QSet<QUuid> affected_ids {};

    if (!node || !node->parent || node->parent == root_)
        return affected_ids;

    if (initial_delta == 0.0 && final_delta == 0.0)
        return affected_ids;

    const auto unit { node->unit };
    const bool direction_rule { node->direction_rule };

    // NOTE: When ancestor nodes receive deltas from a leaf node,
    // the adjustment rule is different from leaf calculation:
    // - If the ancestor has the same direction rule as the leaf, add the delta.
    // - If the ancestor has the opposite direction rule, subtract the delta.
    for (Node* current = node->parent; current && current != root_; current = current->parent) {
        if (current->unit != unit)
            continue;

        const int multiplier { current->direction_rule == direction_rule ? 1 : -1 };

        current->final_total += multiplier * final_delta;
        current->initial_total += multiplier * initial_delta;

        affected_ids.insert(current->id);
    }

    return affected_ids;
}

void TreeModel::RefreshAffectedTotal(const QSet<QUuid>& affected_ids)
{
    for (const QUuid& id : affected_ids) {
        QModelIndex idx = GetIndex(id);
        if (!idx.isValid())
            continue;

        const auto [start_col, end_col] = Utils::NodeNumericColumnRange(section_);
        emit dataChanged(index(idx.row(), start_col, idx.parent()), index(idx.row(), end_col, idx.parent()), { Qt::DisplayRole });
    }
}

void TreeModel::RestartTimer(const QUuid& id)
{
    QTimer* timer { pending_timers_.value(id, nullptr) };

    if (!timer) {
        timer = new QTimer(this);
        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout, this, [this, id]() {
            auto* expired_timer { pending_timers_.take(id) };
            const auto update { pending_updates_.take(id) };

            if (!update.isEmpty()) {
                const auto message { JsonGen::NodeUpdate(section_, id, update) };
                WebSocket::Instance()->SendMessage(kNodeUpdate, message);
            }

            expired_timer->deleteLater();
        });

        pending_timers_[id] = timer;
    }

    timer->start(kThreeThousand);
}

void TreeModel::FlushCaches()
{
    for (auto* timer : std::as_const(pending_timers_)) {
        timer->stop();
        timer->deleteLater();
    }

    pending_timers_.clear();

    for (auto it = pending_updates_.cbegin(); it != pending_updates_.cend(); ++it) {
        if (!it.value().isEmpty()) {
            const auto message { JsonGen::NodeUpdate(section_, it.key(), it.value()) };
            WebSocket::Instance()->SendMessage(kNodeUpdate, message);
        }
    }

    pending_updates_.clear();
}

void TreeModel::EmitRowChanged(const QUuid& node_id, int start_column, int end_column)
{
    auto index { GetIndex(node_id) };
    if (!index.isValid())
        return;

    emit dataChanged(index.siblingAtColumn(start_column), index.siblingAtColumn(end_column));
}

void TreeModel::ApplyTree(const QJsonObject& data)
{
    const QJsonArray node_array { data.value(kNodeArray).toArray() };
    const QJsonArray path_array { data.value(kPathArray).toArray() };

    beginResetModel();

    for (const QJsonValue& val : node_array) {
        const QJsonObject obj { val.toObject() };
        auto* node = NodePool::Instance().Allocate(section_);
        node->ReadJson(obj);
        node_hash_.insert(node->id, node);
    }

    BuildHierarchy(path_array);
    HandleNode();

    sort(std::to_underlying(NodeEnum::kName), Qt::AscendingOrder);
    endResetModel();

    emit SInitStatus();
}

void TreeModel::SortModel()
{
    if (leaf_path_model_ != nullptr) {
        leaf_path_model_->sort(0);
    }
}

// Initialize the root node.
// Root is always represented by an empty QUuid as its ID.
// Root always has direction_rule = true by definition.
void TreeModel::InitRoot(Node*& root)
{
    if (root == nullptr) {
        root = NodePool::Instance().Allocate(section_);
        root->kind = NodeKind::kBranch;
        root->direction_rule = false;
        root->name = QString();
        root->id = QUuid();
        node_hash_.insert(QUuid(), root);
    }

    if (!root) {
        qCritical() << "InitRoot: root node allocation failed!";
    }

    Q_ASSERT(root != nullptr);
}

void TreeModel::BuildHierarchy(const QJsonArray& path_array)
{
    for (const QJsonValue& val : path_array) {
        const QJsonObject obj { val.toObject() };

        const QUuid ancestor_id { QUuid(obj.value(kAncestor).toString()) };
        const QUuid descendant_id { QUuid(obj.value(kDescendant).toString()) };

        Node* ancestor { node_hash_.value(ancestor_id, nullptr) };
        Node* descendant { node_hash_.value(descendant_id, nullptr) };

        if (ancestor && descendant) {
            ancestor->children.emplaceBack(descendant);
            descendant->parent = ancestor;
        }
    }
}

void TreeModel::RegisterPath(Node* node)
{
    CString path { Utils::ConstructPath(root_, node, separator_) };
    const NodeKind kind { node->kind };

    switch (kind) {
    case NodeKind::kBranch:
        branch_path_.insert(node->id, path);
        break;
    case NodeKind::kLeaf:
        leaf_path_.insert(node->id, path);
        leaf_path_model_->AppendItem(path, node->id);
        InsertUnitSet(node->id, node->unit);
        break;
    default:
        break;
    }
}
