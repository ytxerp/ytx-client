#include "treemodel.h"

#include <QJsonArray>
#include <QQueue>

#include "global/nodepool.h"
#include "global/resourcepool.h"
#include "global/websocket.h"
#include "tree/excludeonefiltermodel.h"
#include "utils/jsongen.h"

TreeModel::TreeModel(CSectionInfo& info, CString& separator, int default_unit, QObject* parent)
    : QAbstractItemModel(parent)
    , separator_ { separator }
    , section_ { info.section }
    , section_str_ { info.section_str }
    , node_header_ { info.node_header }
{
    InitRoot(root_, default_unit);
}

TreeModel::~TreeModel() { FlushCaches(); }

void TreeModel::RRemoveNode(const QUuid& node_id)
{
    auto index { GetIndex(node_id) };
    removeRows(index.row(), 1, index.parent());
}

void TreeModel::RSyncDelta(
    const QUuid& node_id, double initial_delta, double final_delta, double /*first_delta*/, double /*second_delta*/, double /*discount_delta*/)
{
    auto* node = GetNode(node_id);
    if (!node)
        return;

    // Multiplier is used to adjust the sign (only meaningful for leaf nodes).
    const int multiplier { node->direction_rule ? 1 : -1 };

    // NOTE: Only leaf nodes apply the direction adjustment.
    // The adjusted deltas are treated as the final values
    // and will be propagated to ancestor nodes.
    const double adjust_initial_delta = multiplier * initial_delta;
    const double adjust_final_delta = multiplier * final_delta;

    // Accumulate into the current node totals
    node->initial_total += adjust_initial_delta;
    node->final_total += adjust_final_delta;

    // Propagate adjusted deltas to ancestor nodes
    UpdateAncestorValue(node, adjust_initial_delta, adjust_final_delta);

    const auto [start_col, end_col] = TotalColumnRange();
    EmitRowChanged(node_id, start_col, end_col);

    emit SSyncStatusValue();
}

void TreeModel::UpdateDelta(const QJsonObject& data)
{
    if (!data.contains(kId)) {
        qCritical() << "ApplyDelta: missing key 'kId' in data:" << data;
    }
    assert(data.contains(kId));

    if (!data.contains(kInitialDelta)) {
        qCritical() << "ApplyDelta: missing key 'kInitialDelta' in data:" << data;
    }
    assert(data.contains(kInitialDelta));

    if (!data.contains(kFinalDelta)) {
        qCritical() << "ApplyDelta: missing key 'kFinalDelta' in data:" << data;
    }
    assert(data.contains(kFinalDelta));

    const QUuid node_id { data.value(kId).toString() };
    const double initial_delta = data.value(kInitialDelta).toString().toDouble();
    const double final_delta = data.value(kFinalDelta).toString().toDouble();

    RSyncDelta(node_id, initial_delta, final_delta);
}

bool TreeModel::InsertNode(int row, const QModelIndex& parent, Node* node)
{
    if (row < 0 || row > rowCount(parent)) {
        qCritical() << "InsertNode: row out of range";
    }
    assert(row >= 0 && row <= rowCount(parent));

    const auto message { JsonGen::InsertNode(section_str_, node, node->parent->id) };
    WebSocket::Instance()->SendMessage(kNodeInsert, message);

    auto* parent_node { GetNodeByIndex(parent) };
    InsertImpl(parent_node, row, node);
    return true;
}

void TreeModel::InsertNode(const QUuid& ancestor, const QJsonObject& data)
{
    if (!node_hash_.contains(ancestor)) {
        qCritical() << "ApplyNodeInsert: ancestor not found in node_hash_, ancestor =" << ancestor;
    }
    assert(node_hash_.contains(ancestor));

    auto* node { NodePool::Instance().Allocate(section_) };
    node->ReadJson(data);

    Node* parent { node_hash_.value(ancestor) };
    const auto row { parent->children.size() };

    InsertImpl(parent, row, node);
}

void TreeModel::InsertImpl(Node* parent, int row, Node* node)
{
    auto parent_index { GetIndex(parent->id) };

    beginInsertRows(parent_index, row, row);
    parent->children.insert(row, node);
    node->parent = parent;
    endInsertRows();

    RegisterNode(node);
    RegisterPath(node);
    SortModel();
}

void TreeModel::InsertMeta(const QUuid& node_id, const QJsonObject& data)
{
    auto* node = GetNode(node_id);
    if (!node)
        return;

    InsertMeta(node, data);
}

void TreeModel::UpdateNode(const QUuid& node_id, const QJsonObject& data)
{
    auto* node = GetNode(node_id);
    if (!node)
        return;

    node->ReadJson(data);

    auto index { GetIndex(node_id) };
    if (index.isValid()) {
        const int first_start = std::to_underlying(NodeEnum::kCode);
        const int first_end = std::to_underlying(NodeEnum::kNote);
        emit dataChanged(index.siblingAtColumn(first_start), index.siblingAtColumn(first_end));

        const auto [second_start, second_end] = CacheColumnRange();
        if (second_end != 0)
            emit dataChanged(index.siblingAtColumn(second_start), index.siblingAtColumn(second_end));
    }
}

void TreeModel::UpdateMeta(const QUuid& node_id, const QJsonObject& data)
{
    auto* node = GetNode(node_id);
    if (!node)
        return;

    UpdateMeta(node, data);
}

void TreeModel::UpdateMeta(Node* node, const QJsonObject& meta)
{
    if (!meta.contains(kUpdatedBy)) {
        qCritical() << "UpdateMeta: missing key 'updated_by' in meta:" << meta;
    }
    assert(meta.contains(kUpdatedBy));

    if (!meta.contains(kUpdatedTime)) {
        qCritical() << "UpdateMeta: missing key 'updated_time' in meta:" << meta;
    }
    assert(meta.contains(kUpdatedTime));

    node->updated_time = QDateTime::fromString(meta[kUpdatedTime].toString(), Qt::ISODate);
    node->updated_by = QUuid(meta[kUpdatedBy].toString());
}

void TreeModel::InsertMeta(Node* node, const QJsonObject& meta)
{
    if (!meta.contains(kUserId)) {
        qCritical() << "InsertMeta: missing 'user_id' in meta:" << meta;
    }
    assert(meta.contains(kUserId));

    if (!meta.contains(kCreatedTime)) {
        qCritical() << "InsertMeta: missing 'created_time' in meta:" << meta;
    }
    assert(meta.contains(kCreatedTime));

    if (!meta.contains(kCreatedBy)) {
        qCritical() << "InsertMeta: missing 'created_by' in meta:" << meta;
    }
    assert(meta.contains(kCreatedBy));

    node->user_id = QUuid(meta.value(kUserId).toString());
    node->created_time = QDateTime::fromString(meta.value(kCreatedTime).toString(), Qt::ISODate);
    node->created_by = QUuid(meta.value(kCreatedBy).toString());
}

void TreeModel::UpdateDirectionRule(Node* node, bool value)
{
    if (node->direction_rule == value)
        return;

    QJsonObject message { JsonGen::NodeDirectionRule(section_str_, node->id, value) };
    WebSocket::Instance()->SendMessage(kDirectionRule, message);

    DirectionRuleImpl(node, value);
}

void TreeModel::UpdateDirectionRule(const QUuid& node_id, bool direction_rule, const QJsonObject& meta)
{
    auto* node = GetNode(node_id);
    if (!node)
        return;

    UpdateMeta(node, meta);
    DirectionRuleImpl(node, direction_rule);
}

void TreeModel::DirectionRuleImpl(Node* node, bool value)
{
    node->InvertTotal();
    node->direction_rule = value;

    const QUuid node_id { node->id };

    if (node->kind == kLeaf) {
        emit SDirectionRule(node_id, node->direction_rule);
    }

    const int role_column = std::to_underlying(NodeEnum::kDirectionRule);
    EmitRowChanged(node_id, role_column, role_column);

    const auto [start_col, end_col] = TotalColumnRange();
    EmitRowChanged(node_id, start_col, end_col);

    emit SSyncStatusValue();
}

void TreeModel::ReplaceLeaf(const QUuid& old_node_id, const QUuid& new_node_id)
{
    auto* old_node { GetNode(old_node_id) };
    auto* new_node { GetNode(new_node_id) };

    if (!old_node || !new_node)
        return;

    const int multiplier { old_node->direction_rule == new_node->direction_rule ? 1 : -1 };
    const double initial_delta = multiplier * old_node->initial_total;
    const double final_delta = multiplier * old_node->final_total;

    new_node->initial_total += initial_delta;
    new_node->final_total += final_delta;

    UpdateAncestorValue(new_node, initial_delta, final_delta);
    RRemoveNode(old_node_id);
}

void TreeModel::UpdateName(const QUuid& node_id, const QJsonObject& data)
{
    if (!data.contains(kName)) {
        qCritical() << "ApplyName: missing key 'name' in data:" << data;
    }
    assert(data.contains(kName));

    if (!data.contains(kUpdatedBy)) {
        qCritical() << "ApplyName: missing key 'updated_by' in data:" << data;
    }
    assert(data.contains(kUpdatedBy));

    if (!data.contains(kUpdatedTime)) {
        qCritical() << "ApplyName: missing key 'updated_time' in data:" << data;
    }
    assert(data.contains(kUpdatedTime));

    auto* node = GetNode(node_id);
    if (!node)
        return;

    node->name = data.value(kName).toString();
    node->updated_by = QUuid(data[kUpdatedBy].toString());
    node->updated_time = QDateTime::fromString(data[kUpdatedTime].toString(), Qt::ISODate);

    NodeUtils::UpdatePath(leaf_path_, branch_path_, root_, node, separator_);
    NodeUtils::UpdateModel(leaf_path_, leaf_model_, node);

    emit SResizeColumnToContents(std::to_underlying(NodeEnum::kName));
    emit SUpdateName(node->id, node->name, node->kind == kBranch);

    auto index { GetIndex(node_id) };
    if (index.isValid()) {
        emit dataChanged(index.siblingAtColumn(std::to_underlying(NodeEnum::kName)), index.siblingAtColumn(std::to_underlying(NodeEnum::kName)));
    }
}

void TreeModel::DragNode(const QUuid& ancestor, const QUuid& descendant, const QJsonObject& data)
{
    if (!data.contains(kUpdatedBy)) {
        qCritical() << "ApplyNodeDrag: missing key 'updated_by' in data:" << data;
    }
    assert(data.contains(kUpdatedBy));

    if (!data.contains(kUpdatedTime)) {
        qCritical() << "ApplyNodeDrag: missing key 'updated_time' in data:" << data;
    }
    assert(data.contains(kUpdatedTime));

    auto* node = GetNode(descendant);
    if (!node)
        return;

    auto* new_parent = GetNode(ancestor);
    if (!new_parent) {
        qCritical() << "ApplyNodeDrag: Ancestor node must exist";
    }
    assert(new_parent);

    const int destination_row = new_parent->children.size();
    const auto destination_parent { GetIndex(ancestor) };

    auto source_row { node->parent->children.indexOf(node) };
    auto source_parent { createIndex(source_row, 0, node).parent() };

    if (moveRows(source_parent, source_row, 1, destination_parent, destination_row)) {
        node->updated_time = QDateTime::fromString(data.value(kUpdatedTime).toString(), Qt::ISODate);
        node->updated_by = QUuid(data.value(kUpdatedBy).toString());
    }

    auto index { createIndex(destination_row, 0, node) };
    if (index.isValid())
        emit dataChanged(index.siblingAtColumn(std::to_underlying(NodeEnum::kUpdateTime)), index.siblingAtColumn(std::to_underlying(NodeEnum::kUpdateBy)));
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
    // root_'s index is QModelIndex(), root_'s id == -1
    if (!index.isValid())
        return QModelIndex();

    auto* node { GetNodeByIndex(index) };
    if (node->id.isNull())
        return QModelIndex();

    auto* parent_node { node->parent };
    if (parent_node->id.isNull())
        return QModelIndex();

    return createIndex(parent_node->parent->children.indexOf(parent_node), 0, parent_node);
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    auto* parent_node { GetNodeByIndex(parent) };
    auto* node { parent_node->children.at(row) };

    return node ? createIndex(row, column, node) : QModelIndex();
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
    }
    assert(row >= 0 && row <= rowCount(parent) - 1);

    if (count != 1) {
        qCritical() << "removeRows: Only support removing one row, count =" << count;
    }
    assert(count == 1);

    auto* parent_node { GetNodeByIndex(parent) };
    auto* node { parent_node->children.at(row) };

    const auto node_id { node->id };

    beginRemoveRows(parent, row, row);
    parent_node->children.removeOne(node);
    endRemoveRows();

    RemovePath(node, parent_node);

    ResourcePool<Node>::Instance().Recycle(node);
    node_hash_.remove(node_id);

    emit SSyncStatusValue();
    emit SResizeColumnToContents(std::to_underlying(NodeEnum::kName));
    emit SFreeWidget(node_id);

    return true;
}

bool TreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    auto* destination_parent { GetNodeByIndex(parent) };
    if (destination_parent->kind != kBranch)
        return false;

    QUuid node_id {};

    if (auto mime { data->data(kYTX) }; !mime.isEmpty())
        node_id = QUuid::fromRfc4122(mime);

    auto* node { node_hash_.value(node_id) };
    if (!node) {
        qCritical() << "dropMimeData: Node not found for UUID:" << node_id;
        return false;
    }

    if (node->parent == destination_parent || NodeUtils::IsDescendant(destination_parent, node))
        return false;

    auto destination_row { destination_parent->children.size() };
    auto source_row { node->parent->children.indexOf(node) };
    auto source_parent { createIndex(source_row, 0, node).parent() };

    if (moveRows(source_parent, source_row, 1, parent, destination_row)) {
        const auto message { JsonGen::DragNode(section_str_, node_id, destination_parent->id) };
        WebSocket::Instance()->SendMessage(kNodeDrag, message);
    }

    return true;
}

bool TreeModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int /*count*/, const QModelIndex& destinationParent, int destinationChild)
{
    auto* source_parent { GetNodeByIndex(sourceParent) };
    auto* destination_parent { GetNodeByIndex(destinationParent) };

    if (!source_parent) {
        qCritical() << "moveRows: Source parent is null!";
    }
    assert(source_parent);

    if (!destination_parent) {
        qCritical() << "moveRows: Destination parent is null!";
    }
    assert(destination_parent);

    if (sourceRow < 0 || sourceRow >= source_parent->children.size()) {
        qCritical() << "moveRows: Source row is out of bounds!";
    }
    assert(sourceRow >= 0 && sourceRow < source_parent->children.size());

    beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent, destinationChild);
    auto* node { source_parent->children.takeAt(sourceRow) };
    if (!node) {
        qCritical() << "moveRows: Node extraction failed!";
    }
    assert(node);

    UpdateAncestorValue(node, -node->initial_total, -node->final_total);

    destination_parent->children.insert(destinationChild, node);
    node->parent = destination_parent;
    UpdateAncestorValue(node, node->initial_total, node->final_total);
    endMoveRows();

    NodeUtils::UpdatePath(leaf_path_, branch_path_, root_, node, separator_);
    NodeUtils::UpdateModel(leaf_path_, leaf_model_, node);

    emit SUpdateName(node->id, node->name, node->kind == kBranch);
    emit SResizeColumnToContents(std::to_underlying(NodeEnum::kName));

    return true;
}

QStringList TreeModel::ChildrenName(const QUuid& parent_id) const
{
    auto* node { node_hash_.value(parent_id) };
    if (!node) {
        qCritical() << "ChildrenName: parent_id not found in node_hash_, parent_id =" << parent_id;
    }
    assert(node);

    QStringList list {};
    list.reserve(node->children.size());

    for (const auto* child : std::as_const(node->children)) {
        list.emplaceBack(child->name);
    }

    return list;
}

void TreeModel::LeafPathBranchPathModel(ItemModel* model) const { NodeUtils::LeafPathBranchPathModel(leaf_path_, branch_path_, model); }

void TreeModel::UpdateSeparator(CString& old_separator, CString& new_separator)
{
    if (old_separator == new_separator || old_separator.isEmpty() || new_separator.isEmpty())
        return;

    NodeUtils::UpdatePathSeparator(old_separator, new_separator, leaf_path_);
    NodeUtils::UpdatePathSeparator(old_separator, new_separator, branch_path_);

    leaf_model_->UpdateSeparator(old_separator, new_separator);
}

void TreeModel::UpdateDefaultUnit(int default_unit)
{
    root_->unit = default_unit;

    const int row_count = rowCount();
    if (row_count == 0)
        return;

    const int col_count = node_header_.size();
    const int first_col = col_count - 2;
    const int last_col = col_count - 1;

    const QModelIndex top_left = index(0, first_col);
    const QModelIndex bottom_right = index(row_count - 1, last_col);

    emit dataChanged(top_left, bottom_right);
}

void TreeModel::SearchNode(QList<const Node*>& node_list, CString& name) const
{
    node_list.reserve(node_hash_.size() / 2);

    for (const auto& [id, node_ptr] : node_hash_.asKeyValueRange()) {
        if (!node_ptr)
            continue;

        if (node_ptr->name.contains(name, Qt::CaseInsensitive)) {
            node_list.emplaceBack(node_ptr);
        }
    }
}

QModelIndex TreeModel::GetIndex(const QUuid& node_id) const
{
    if (!node_hash_.contains(node_id)) {
        qCritical() << "GetIndex: node_id not found in node_hash_, node_id =" << node_id;
    }
    assert(node_hash_.contains(node_id));

    if (node_id.isNull())
        return QModelIndex();

    const Node* node { node_hash_.value(node_id) };

    if (!node->parent)
        return QModelIndex();

    auto row { node->parent->children.indexOf(node) };
    if (row == -1)
        return QModelIndex();

    return createIndex(row, 0, node);
}

void TreeModel::UpdateName(const QUuid& node_id, CString& new_name)
{
    auto* node { node_hash_.value(node_id) };
    if (!node) {
        qCritical() << "UpdateName: node_id not found in node_hash_, node_id =" << node_id;
    }
    assert(node);

    node->name = new_name;

    QJsonObject cache {};
    cache.insert(kName, new_name);

    const auto message { JsonGen::Update(section_str_, node_id, cache) };
    WebSocket::Instance()->SendMessage(kNameUpdate, message);

    NodeUtils::UpdatePath(leaf_path_, branch_path_, root_, node, separator_);
    NodeUtils::UpdateModel(leaf_path_, leaf_model_, node);

    emit SResizeColumnToContents(std::to_underlying(NodeEnum::kName));
    emit SUpdateName(node->id, node->name, node->kind == kBranch);
}

QString TreeModel::Path(const QUuid& node_id) const
{
    if (auto it = leaf_path_.constFind(node_id); it != leaf_path_.constEnd())
        return it.value();

    if (auto it = branch_path_.constFind(node_id); it != branch_path_.constEnd())
        return it.value();

    return {};
}

QSortFilterProxyModel* TreeModel::ExcludeOneModel(const QUuid& node_id)
{
    auto* model { new ExcludeOneFilterModel(node_id, this) };
    model->setSourceModel(leaf_model_);
    return model;
}

void TreeModel::FetchOneNode(const QUuid& node_id)
{
    const auto message { JsonGen::NodeAcked(section_str_, node_id) };
    WebSocket::Instance()->SendMessage(kNodeAcked, message);
}

void TreeModel::RemovePath(Node* node, Node* parent_node)
{
    const auto node_id { node->id };

    switch (node->kind) {
    case kBranch: {
        for (auto* child : std::as_const(node->children)) {
            child->parent = parent_node;
            parent_node->children.emplace_back(child);
        }

        NodeUtils::UpdatePath(leaf_path_, branch_path_, root_, node, separator_);
        NodeUtils::UpdateModel(leaf_path_, leaf_model_, node);

        branch_path_.remove(node_id);
        emit SUpdateName(node_id, node->name, true);

    } break;
    case kLeaf: {
        leaf_path_.remove(node_id);
        NodeUtils::RemoveItem(leaf_model_, node_id);
        UpdateAncestorValue(node, -node->initial_total, -node->final_total);
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

        if (node->kind == kLeaf)
            UpdateAncestorValue(node, node->initial_total, node->final_total);
    }

    SortModel();
}

Node* TreeModel::GetNodeByIndex(const QModelIndex& index) const
{
    if (index.isValid() && index.internalPointer())
        return static_cast<Node*>(index.internalPointer());

    return root_;
}

bool TreeModel::UpdateAncestorValue(
    Node* node, double initial_delta, double final_delta, double /*first_delta*/, double /*second_delta*/, double /*discount_delta*/)
{
    if (!node || !node->parent || node->parent == root_)
        return false;

    if (initial_delta == 0.0 && final_delta == 0.0)
        return false;

    const int unit { node->unit };
    const bool direction_rule { node->direction_rule };

    QModelIndexList ancestor {};
    const auto [start_col, end_col] = TotalColumnRange();

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

        ancestor.emplaceBack(GetIndex(current->id));
    }

    if (!ancestor.isEmpty())
        emit dataChanged(index(ancestor.first().row(), start_col), index(ancestor.last().row(), end_col), { Qt::DisplayRole });

    return true;
}

void TreeModel::RestartTimer(const QUuid& id)
{
    QTimer* timer = timers_.value(id, nullptr);

    if (!timer) {
        timer = new QTimer(this);
        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout, this, [this, id]() {
            auto* expired_timer = timers_.take(id);
            const auto cache = caches_.take(id);

            if (!cache.isEmpty()) {
                const auto message = JsonGen::Update(section_str_, id, cache);
                WebSocket::Instance()->SendMessage(kNodeUpdate, message);
            }

            expired_timer->deleteLater();
        });

        timers_[id] = timer;
    }

    timer->start(kThreeThousand);
}

void TreeModel::FlushCaches()
{
    for (auto* timer : std::as_const(timers_)) {
        timer->stop();
        timer->deleteLater();
    }

    timers_.clear();

    for (auto it = caches_.cbegin(); it != caches_.cend(); ++it) {
        if (!it.value().isEmpty()) {
            const auto message = JsonGen::Update(section_str_, it.key(), it.value());
            WebSocket::Instance()->SendMessage(kNodeUpdate, message);
        }
    }

    caches_.clear();
}

void TreeModel::EmitRowChanged(const QUuid& node_id, int start_column, int end_column)
{
    auto index = GetIndex(node_id);
    if (!index.isValid())
        return;

    emit dataChanged(index.siblingAtColumn(start_column), index.siblingAtColumn(end_column));
}

void TreeModel::ApplyTree(const QJsonObject& data)
{
    const QJsonArray node_array { data.value(kNode).toArray() };
    const QJsonArray path_array { data.value(kPath).toArray() };

    beginResetModel();

    for (const QJsonValue& val : node_array) {
        const QJsonObject obj = val.toObject();
        auto* node = NodePool::Instance().Allocate(section_);
        node->ReadJson(obj);
        RegisterNode(node);
    }

    BuildHierarchy(path_array);
    HandleNode();

    endResetModel();
}

void TreeModel::AckNode(const QJsonObject& leaf_obj, const QUuid& ancestor_id)
{
    if (!node_hash_.contains(ancestor_id)) {
        qCritical() << "AckOneNode: ancestor_id not found in node_hash_:" << ancestor_id;
    }
    assert(node_hash_.contains(ancestor_id));

    auto* node = NodePool::Instance().Allocate(section_);
    node->ReadJson(leaf_obj);

    Node* ancestor { node_hash_.value(ancestor_id) };

    const int row = ancestor->children.size();
    const auto parent { GetIndex(ancestor_id) };

    beginInsertRows(parent, row, row);
    ancestor->children.insert(row, node);
    node->parent = ancestor;
    endInsertRows();

    RegisterNode(node);
    RegisterPath(node);
    SortModel();
}

void TreeModel::SortModel()
{
    if (leaf_model_ != nullptr) {
        leaf_model_->sort(0);
    }
}

// Initialize the root node.
// Root is always represented by an empty QUuid as its ID.
// Root always has direction_rule = true by definition.
void TreeModel::InitRoot(Node*& root, int default_unit)
{
    if (root == nullptr) {
        root = NodePool::Instance().Allocate(section_);
        root->kind = kBranch;
        root->unit = default_unit;
        root->direction_rule = false;
        root->name = QString();
        root->id = QUuid();
        node_hash_.insert(QUuid(), root);
    }

    if (!root) {
        qCritical() << "InitRoot: root node allocation failed!";
    }
    assert(root);
}

void TreeModel::BuildHierarchy(const QJsonArray& path_array)
{
    for (const QJsonValue& val : path_array) {
        const QJsonObject obj = val.toObject();

        const QUuid ancestor_id = QUuid(obj.value(kAncestor).toString());
        const QUuid descendant_id = QUuid(obj.value(kDescendant).toString());

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
    CString path { NodeUtils::ConstructPath(root_, node, separator_) };

    switch (node->kind) {
    case kBranch:
        branch_path_.insert(node->id, path);
        break;
    case kLeaf:
        leaf_path_.insert(node->id, path);
        leaf_model_->AppendItem(path, node->id);
        InsertUnitSet(node->id, node->unit);
        break;
    default:
        break;
    }
}

QSet<QUuid> TreeModel::ChildrenId(const QUuid& parent_id) const
{
    auto* node { node_hash_.value(parent_id) };
    if (!node) {
        qCritical() << "ChildrenId: parent_id not found in node_hash_:" << parent_id;
    }
    assert(node);

    if (node->kind != kBranch || node->children.isEmpty())
        return {};

    QQueue<const Node*> queue {};
    queue.enqueue(node);

    QSet<QUuid> set {};
    while (!queue.isEmpty()) {
        auto* queue_node = queue.dequeue();

        switch (queue_node->kind) {
        case kBranch:
            for (const auto* child : queue_node->children)
                queue.enqueue(child);
            break;
        case kLeaf:
            set.insert(queue_node->id);
            break;
        default:
            break;
        }
    }

    return set;
}
