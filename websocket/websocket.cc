#include "websocket.h"

#include <QCoreApplication>
#include <QJsonArray>

#include "component/constant.h"
#include "component/using.h"
#include "websocket/jsongen.h"

WebSocket::WebSocket(QObject* parent)
    : QObject(parent)
{
    InitHandler();
    InitConnect();
}

WebSocket::~WebSocket()
{
    if (ping_timer_) {
        ping_timer_->stop();
        ping_timer_->deleteLater();
        ping_timer_ = nullptr;
    }

    if (socket_.state() == QAbstractSocket::ConnectedState) {
        socket_.close();
    }
}

WebSocket* WebSocket::Instance()
{
    static WebSocket* instance = new WebSocket(qApp);

    Q_ASSERT(instance != nullptr);
    return instance;
}

void WebSocket::ReadConfig(QSharedPointer<QSettings> local_settings)
{
    local_settings->beginGroup(kServer);
    const QString host { local_settings->value(kHost).toString() };
    const QString port { local_settings->value(kPort).toString() };
    local_settings->endGroup();

    const QString url_str { (!host.isEmpty() && !port.isEmpty()) ? QString("ws://%1:%2").arg(host, port) : QString("wss://ytxerp.cc") };

    server_url_ = QUrl(url_str);
    if (!server_url_.isValid()) {
        qWarning() << "Invalid WebSocket URL constructed:" << url_str;
    }
}

void WebSocket::Connect()
{
    if (!server_url_.isValid()) {
        qWarning() << "WebSocket: invalid URL, cannot connect.";
        return;
    }

    if (socket_.state() == QAbstractSocket::ConnectedState || socket_.state() == QAbstractSocket::ConnectingState) {
        qInfo() << "WebSocket: already connected or connecting.";
        return;
    }

    socket_.open(server_url_);
}

void WebSocket::RConnected()
{
    emit SConnectResult(true);
    InitTimer();
}

void WebSocket::RDisconnected()
{
    if (session_id_.isEmpty())
        return;

    emit SRemoteHostClosed();

    session_id_.clear();

    if (ping_timer_) {
        ping_timer_->stop();
    }

    Connect();
}

void WebSocket::RErrorOccurred(QAbstractSocket::SocketError error)
{
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        qWarning() << "WebSocket connection refused!";
        emit SConnectResult(false);
        break;
    case QAbstractSocket::NetworkError:
        qWarning() << "WebSocket network error, possibly no network connection!";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        qWarning() << "WebSocket remote host closed connection!";
        break;
    default:
        qWarning() << "WebSocket error:" << error;
        break;
    }
}

void WebSocket::InitHandler()
{
    handler_obj_[kLoginResult] = [this](const QJsonObject& obj) { NotifyLoginResult(obj); };
    handler_obj_[kRegisterResult] = [this](const QJsonObject& obj) { NotifyRegisterResult(obj); };
    handler_obj_[kTreeAcked] = [this](const QJsonObject& obj) { AckTree(obj); };
    handler_obj_[kLeafAcked] = [this](const QJsonObject& obj) { AckLeaf(obj); };
    handler_obj_[kWorkspaceAccessPending] = [this](const QJsonObject& obj) { NotifyWorkspaceAccessPending(obj); };
    handler_obj_[kTreeApplied] = [this](const QJsonObject& obj) { ApplyTree(obj); };
    handler_obj_[kNodeInsert] = [this](const QJsonObject& obj) { InsertNode(obj); };
    handler_obj_[kLeafRemove] = [this](const QJsonObject& obj) { RemoveLeaf(obj); };
    handler_obj_[kBranchRemove] = [this](const QJsonObject& obj) { RemoveBranch(obj); };
    handler_obj_[kLeafReplace] = [this](const QJsonObject& obj) { ReplaceLeaf(obj); };
    handler_obj_[kNodeUpdate] = [this](const QJsonObject& obj) { UpdateNode(obj); };
    handler_obj_[kEntryInsert] = [this](const QJsonObject& obj) { InsertEntry(obj); };
    handler_obj_[kEntrySearch] = [this](const QJsonObject& obj) { SearchEntry(obj); };
    handler_obj_[kEntryUpdate] = [this](const QJsonObject& obj) { UpdateEntry(obj); };
    handler_obj_[kEntryRemove] = [this](const QJsonObject& obj) { RemoveEntry(obj); };
    handler_obj_[kDirectionRule] = [this](const QJsonObject& obj) { UpdateDirectionRule(obj); };
    handler_obj_[kNodeStatus] = [this](const QJsonObject& obj) { UpdateNodeStatus(obj); };
    handler_obj_[kLeafRemoveDenied] = [this](const QJsonObject& obj) { NotifyLeafRemoveDenied(obj); };
    handler_obj_[kNodeDrag] = [this](const QJsonObject& obj) { DragNode(obj); };
    handler_obj_[kEntryAction] = [this](const QJsonObject& obj) { ActionEntry(obj); };
    handler_obj_[kDefaultUnit] = [this](const QJsonObject& obj) { UpdateDefaultUnit(obj); };
    handler_obj_[kUpdateDefaultUnitFailure] = [this](const QJsonObject& obj) { NotifyUpdateDefaultUnitFailure(obj); };
    handler_obj_[kDocumentDir] = [this](const QJsonObject& obj) { UpdateDocumentDir(obj); };
    handler_obj_[kNodeName] = [this](const QJsonObject& obj) { UpdateNodeName(obj); };
    handler_obj_[kEntryLinkedNode] = [this](const QJsonObject& obj) { UpdateEntryLinkedNode(obj); };
    handler_obj_[kEntryRate] = [this](const QJsonObject& obj) { UpdateEntryRate(obj); };
    handler_obj_[kEntryNumeric] = [this](const QJsonObject& obj) { UpdateEntryNumeric(obj); };
    handler_obj_[kLeafRemoveSafely] = [this](const QJsonObject& obj) { RemoveLeafSafely(obj); };

    handler_arr_[kSharedConfig] = [this](const QJsonArray& arr) { SharedConfig(arr); };
}

void WebSocket::InitConnect()
{
    connect(&socket_, &QWebSocket::connected, this, &WebSocket::RConnected);
    connect(&socket_, &QWebSocket::disconnected, this, &WebSocket::RDisconnected);
    connect(&socket_, &QWebSocket::errorOccurred, this, &WebSocket::RErrorOccurred);
    connect(&socket_, &QWebSocket::textMessageReceived, this, &WebSocket::RReceiveMessage);

    connect(&socket_, &QWebSocket::pong, []() { });
}

void WebSocket::InitTimer()
{
    if (!ping_timer_) {
        ping_timer_ = new QTimer(this);
        ping_timer_->setInterval(30000);

        connect(ping_timer_, &QTimer::timeout, this, [this]() {
            if (socket_.state() == QAbstractSocket::ConnectedState) {
                socket_.ping();
            }
        });
    }

    ping_timer_->start();
}

void WebSocket::SendMessage(const QString& type, const QJsonObject& value)
{
    if (socket_.state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send message: WebSocket is not connected";
        return;
    }

    const QJsonValue message { QJsonObject { { kKind, type }, { kValue, value } } };
    const QByteArray json { message.toJson(QJsonDocument::Compact) };

    socket_.sendTextMessage(QString::fromUtf8(json));
}

void WebSocket::Clear()
{
    if (socket_.state() == QAbstractSocket::ConnectedState || socket_.state() == QAbstractSocket::ConnectingState) {
        socket_.close();
    }

    tree_model_hash_.clear();
    entry_hub_hash_.clear();
}

void WebSocket::RReceiveMessage(const QString& message)
{
    QJsonParseError err {};
    const QJsonValue root { QJsonValue::fromJson(message.toUtf8(), &err) };

    if (err.error != QJsonParseError::NoError || !root.isObject()) {
        qWarning() << "Invalid JSON message:" << message;
        return;
    }

    const QJsonObject obj { root.toObject() };
    const QString msg_type { obj.value(kKind).toString() };
    const QJsonValue value { obj.value(kValue) };

    if (value.isObject()) {
        if (const auto it { handler_obj_.constFind(msg_type) }; it != handler_obj_.constEnd()) {
            it.value()(value.toObject());
            return;
        }
    }

    if (value.isArray()) {
        if (const auto it { handler_arr_.constFind(msg_type) }; it != handler_arr_.constEnd()) {
            it.value()(value.toArray());
            return;
        }
    }

    qWarning() << "Unsupported value type for message:" << msg_type;
}

void WebSocket::NotifyLoginResult(const QJsonObject& obj)
{
    const bool result { obj[kResult].toBool() };

    emit SLoginResult(result);

    if (result) {
        session_id_ = obj[kSessionId].toString();
        const auto expire_time { QDateTime::fromString(obj[kExpireTime].toString(), Qt::ISODate) };
        const auto expire_date { expire_time.date().toString(kDateFST) };
        emit SInitializeContext(expire_date);
    }
}

void WebSocket::NotifyWorkspaceAccessPending(const QJsonObject& obj)
{
    CString email { obj.value(kEmail).toString() };
    CString workspace { obj.value(kWorkspace).toString() };

    emit SWorkspaceAccessPending(email, workspace);
}

void WebSocket::NotifyRegisterResult(const QJsonObject& obj)
{
    const bool result { obj[kResult].toBool() };
    emit SRegisterResult(result);
}

void WebSocket::InsertNode(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const QJsonObject node_obj { obj.value(kNode).toObject() };
    const QJsonObject path_obj { obj.value(kPath).toObject() };

    const auto descendant { QUuid(path_obj.value(kDescendant).toString()) };
    const auto ancestor { QUuid(path_obj.value(kAncestor).toString()) };

    auto tree_model { tree_model_hash_.value(section) };

    if (session_id == session_id_)
        tree_model->InsertMeta(descendant, node_obj);
    else
        tree_model->InsertNode(ancestor, node_obj);
}

void WebSocket::UpdateNode(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };
    const QJsonObject cache { obj.value(kCache).toObject() };

    auto tree_model { tree_model_hash_.value(section) };

    if (session_id == session_id_)
        tree_model->UpdateMeta(node_id, cache);
    else
        tree_model->SyncNode(node_id, cache);
}

void WebSocket::DragNode(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const QJsonObject path { obj.value(kPath).toObject() };
    const QJsonObject node { obj.value(kNode).toObject() };

    CString ancestor { path.value(kAncestor).toString() };
    CString descendant { path.value(kDescendant).toString() };

    if (ancestor.isEmpty() || descendant.isEmpty() || node.isEmpty()) {
        qWarning() << "Invalid drag node obj";
        return;
    }

    auto tree_model { tree_model_hash_.value(section) };

    if (session_id == session_id_)
        tree_model->UpdateMeta(QUuid(descendant), node);
    else
        tree_model->DragNode(QUuid(ancestor), QUuid(descendant), node);
}

void WebSocket::ApplyTree(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    auto tree_model { tree_model_hash_.value(section) };
    tree_model->ApplyTree(obj);
}

void WebSocket::AckTree(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    auto tree_model { tree_model_hash_.value(section) };
    tree_model->AckTree(obj);
}

void WebSocket::AckLeaf(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };
    const auto entry_id { QUuid(obj.value(kEntryId).toString()) };
    const QJsonArray array { obj.value(kEntryArray).toArray() };

    auto entry_hub { entry_hub_hash_.value(section) };
    entry_hub->AckLeafTable(node_id, array);

    if (!entry_id.isNull())
        emit SScrollToEntry(node_id, entry_id);
}

void WebSocket::AckNode(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };

    const QJsonObject leaf_obj { obj.value(kNode).toObject() };
    const QUuid ancestor { QUuid(obj.value(kAncestor).toString()) };

    auto tree_model { tree_model_hash_.value(section) };
    tree_model->AckNode(leaf_obj, ancestor);
}

void WebSocket::RemoveLeaf(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };

    const QJsonObject leaf_obj { obj.value(kLeafEntry).toObject() };
    const QJsonArray delta_array { obj.value(kDeltaArray).toArray() };

    const auto leaf_entry { ParseNodeReference(leaf_obj) };

    auto entry_hub { entry_hub_hash_.value(section) };
    auto tree_model { tree_model_hash_.value(section) };

    entry_hub->RemoveLeaf(leaf_entry);
    tree_model->SyncDeltaArray(delta_array);

    if (session_id != session_id_)
        tree_model->RRemoveNode(QUuid(node_id));
}

void WebSocket::RemoveLeafSafely(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };

    auto tree_model { tree_model_hash_.value(section) };
    tree_model->RRemoveNode(node_id);

    if (session_id == session_id_)
        emit SNodeRemoveConfirmed(node_id);
}

void WebSocket::RemoveBranch(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };

    auto tree_model { tree_model_hash_.value(section) };

    if (session_id != session_id_)
        tree_model->RRemoveNode(QUuid(node_id));
}

void WebSocket::NotifyLeafRemoveDenied(const QJsonObject& obj) { emit SLeafRemoveDenied(obj); }

void WebSocket::ReplaceLeaf(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const bool result { obj.value(kResult).toBool() };
    const bool inventory_external_ref { obj.value(kInventoryExternalRef).toBool() };
    const QUuid old_node_id(obj.value(kOldNodeId).toString());
    const QUuid new_node_id(obj.value(kNewNodeId).toString());

    if (session_id == session_id_) {
        emit SReplaceResult(result);
    }

    if (!result) {
        qWarning() << "Replace Leaf Node failed";
        return;
    }

    auto entry_hub { entry_hub_hash_.value(section) };
    entry_hub->ReplaceLeaf(old_node_id, new_node_id);

    if (section == Section::kInventory && inventory_external_ref) {
        entry_hub_hash_.value(Section::kSale)->ApplyInventoryReplace(old_node_id, new_node_id);
        entry_hub_hash_.value(Section::kPurchase)->ApplyInventoryReplace(old_node_id, new_node_id);
        entry_hub_hash_.value(Section::kPartner)->ApplyInventoryReplace(old_node_id, new_node_id);
    }

    auto tree_model { tree_model_hash_.value(section) };
    tree_model->ReplaceLeaf(old_node_id, new_node_id);
}

void WebSocket::UpdateEntry(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const auto entry_id { QUuid(obj.value(kEntryId).toString()) };
    const QJsonObject cache { obj.value(kCache).toObject() };

    auto entry_hub { entry_hub_hash_.value(section) };

    assert(entry_hub);
    assert(!entry_id.isNull());

    if (session_id == session_id_) {
        entry_hub->UpdateMeta(entry_id, cache);
        return;
    }

    entry_hub->UpdateEntry(entry_id, cache);
}

void WebSocket::UpdateEntryLinkedNode(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };

    const QJsonObject entry { obj.value(kEntry).toObject() };
    const QJsonObject old_rhs_delta { obj.value(kOldNodeDelta).toObject() };
    const QJsonObject new_rhs_delta { obj.value(kNewNodeDelta).toObject() };

    const QUuid entry_id { obj.value(kEntryId).toString() };
    const QUuid old_node_id { obj.value(kOldNodeId).toString() };
    const QUuid new_node_id { obj.value(kNewNodeId).toString() };

    auto entry_hub { entry_hub_hash_.value(section) };
    auto tree_model { tree_model_hash_.value(section) };

    if (session_id == session_id_) {
        entry_hub->UpdateMeta(entry_id, entry);

        if (!old_rhs_delta.isEmpty())
            tree_model->UpdateMeta(old_node_id, old_rhs_delta);

        if (!new_rhs_delta.isEmpty())
            tree_model->UpdateMeta(new_node_id, new_rhs_delta);

        return;
    }

    entry_hub->UpdateEntryLinkedNode(entry_id, old_node_id, new_node_id, entry);

    if (!old_rhs_delta.isEmpty() && !new_rhs_delta.isEmpty()) {
        const QJsonArray delta_array { old_rhs_delta, new_rhs_delta };
        tree_model->SyncDeltaArray(delta_array);
    }
}

void WebSocket::UpdateEntryRate(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };

    const auto entry_id { QUuid(obj.value(kEntryId).toString()) };
    const QJsonObject cache { obj.value(kCache).toObject() };
    const QJsonObject lhs_delta { obj.value(kLhsDelta).toObject() };
    const QJsonObject rhs_delta { obj.value(kRhsDelta).toObject() };
    const bool is_parallel { obj.value(kIsParallel).toBool() };

    auto entry_hub { entry_hub_hash_.value(section) };
    auto tree_model { tree_model_hash_.value(section) };

    assert(entry_hub);
    assert(tree_model);

    if (session_id == session_id_) {
        entry_hub->UpdateMeta(entry_id, cache);

        if (!lhs_delta.isEmpty())
            tree_model->UpdateMeta(QUuid(lhs_delta.value(kId).toString()), lhs_delta);

        if (!rhs_delta.isEmpty())
            tree_model->UpdateMeta(QUuid(rhs_delta.value(kId).toString()), rhs_delta);

        return;
    }

    entry_hub->UpdateEntryRate(entry_id, cache, is_parallel);

    if (!lhs_delta.isEmpty() && !rhs_delta.isEmpty()) {
        const QJsonArray delta_array { lhs_delta, rhs_delta };
        tree_model->SyncDeltaArray(delta_array);
    }
}

void WebSocket::UpdateEntryNumeric(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };

    const auto entry_id { QUuid(obj.value(kEntryId).toString()) };
    const QJsonObject cache { obj.value(kCache).toObject() };
    const QJsonObject lhs_delta { obj.value(kLhsDelta).toObject() };
    const QJsonObject rhs_delta { obj.value(kRhsDelta).toObject() };
    const bool is_parallel { obj.value(kIsParallel).toBool() };

    auto entry_hub { entry_hub_hash_.value(section) };
    auto tree_model { tree_model_hash_.value(section) };

    assert(entry_hub);
    assert(tree_model);

    if (session_id == session_id_) {
        entry_hub->UpdateMeta(entry_id, cache);

        if (!lhs_delta.isEmpty())
            tree_model->UpdateMeta(QUuid(lhs_delta.value(kId).toString()), lhs_delta);

        if (!rhs_delta.isEmpty())
            tree_model->UpdateMeta(QUuid(rhs_delta.value(kId).toString()), rhs_delta);

        return;
    }

    entry_hub->UpdateEntryNumeric(entry_id, cache, is_parallel);

    if (!lhs_delta.isEmpty() && !rhs_delta.isEmpty()) {
        const QJsonArray delta_array { lhs_delta, rhs_delta };
        tree_model->SyncDeltaArray(delta_array);
    }
}

void WebSocket::SearchEntry(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QJsonArray array { obj.value(kEntryArray).toArray() };

    auto entry_hub { entry_hub_hash_.value(section) };
    entry_hub->SearchEntry(array);
}

void WebSocket::InsertEntry(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const auto entry_id { QUuid(obj.value(kEntryId).toString()) };

    const QJsonObject entry { obj.value(kEntry).toObject() };
    const QJsonObject lhs_delta { obj.value(kLhsDelta).toObject() };
    const QJsonObject rhs_delta { obj.value(kRhsDelta).toObject() };

    auto entry_hub { entry_hub_hash_.value(section) };
    auto tree_model { tree_model_hash_.value(section) };

    assert(entry_hub);
    assert(tree_model);

    if (session_id == session_id_) {
        entry_hub->InsertMeta(entry_id, entry);

        if (!lhs_delta.isEmpty())
            tree_model->UpdateMeta(QUuid(lhs_delta.value(kId).toString()), lhs_delta);

        if (!rhs_delta.isEmpty())
            tree_model->UpdateMeta(QUuid(rhs_delta.value(kId).toString()), rhs_delta);

        return;
    }

    entry_hub->InsertEntry(entry);

    if (!lhs_delta.isEmpty() && !rhs_delta.isEmpty()) {
        const QJsonArray delta_array { lhs_delta, rhs_delta };
        tree_model->SyncDeltaArray(delta_array);
    }
}

void WebSocket::RemoveEntry(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const auto entry_id { QUuid(obj.value(kEntryId).toString()) };
    const QJsonObject lhs_delta { obj.value(kLhsDelta).toObject() };
    const QJsonObject rhs_delta { obj.value(kRhsDelta).toObject() };

    auto entry_hub { entry_hub_hash_.value(section) };
    auto tree_model { tree_model_hash_.value(section) };

    assert(entry_hub);
    assert(tree_model);

    if (session_id == session_id_) {
        if (!lhs_delta.isEmpty())
            tree_model->UpdateMeta(QUuid(lhs_delta.value(kId).toString()), lhs_delta);

        if (!rhs_delta.isEmpty())
            tree_model->UpdateMeta(QUuid(rhs_delta.value(kId).toString()), rhs_delta);

        return;
    }

    entry_hub->RemoveEntry(entry_id);

    if (!lhs_delta.isEmpty() && !rhs_delta.isEmpty()) {
        const QJsonArray delta_array { lhs_delta, rhs_delta };
        tree_model->SyncDeltaArray(delta_array);
    }
}

void WebSocket::UpdateDirectionRule(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const bool direction_rule { obj.value(kDirectionRule).toBool() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };
    const QJsonObject meta { obj.value(kMeta).toObject() };

    auto tree_model { tree_model_hash_.value(section) };

    if (session_id == session_id_) {
        tree_model->UpdateMeta(node_id, meta);
        return;
    }

    tree_model->SyncDirectionRule(node_id, direction_rule, meta);
}

void WebSocket::UpdateNodeStatus(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const int status { obj.value(kStatus).toInt() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };
    const QJsonObject meta { obj.value(kMeta).toObject() };

    auto tree_model { tree_model_hash_.value(section) };

    if (session_id == session_id_) {
        tree_model->UpdateMeta(node_id, meta);
        return;
    }

    tree_model->SyncNodeStatus(node_id, status, meta);
}

void WebSocket::UpdateNodeName(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };

    const auto node_id { QUuid(obj.value(kNodeId).toString()) };
    const auto name { obj.value(kName).toString() };
    const QJsonObject meta { obj.value(kMeta).toObject() };

    auto tree_model { tree_model_hash_.value(section) };

    if (session_id == session_id_) {
        tree_model->UpdateMeta(node_id, meta);
        return;
    }

    tree_model->SyncNodeName(node_id, name, meta);
}

void WebSocket::ActionEntry(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };

    const auto node_id { QUuid(obj.value(kNodeId).toString()) };
    const EntryAction action { EntryAction(obj.value(kAction).toInt()) };

    auto entry_hub { entry_hub_hash_.value(section) };

    if (session_id == session_id_) {
        return;
    }

    entry_hub->ActionEntry(node_id, action);
}

void WebSocket::UpdateDocumentDir(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    CString document_dir { obj.value(kDocumentDir).toString() };

    if (session_id != session_id_) {
        emit SDocumentDir(section, document_dir);
    }
}

void WebSocket::UpdateDefaultUnit(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const int default_unit { obj.value(kDefaultUnit).toInt() };

    auto tree_model { tree_model_hash_.value(section) };
    if (!tree_model)
        return;

    tree_model->UpdateDefaultUnit(default_unit);
    emit SDefaultUnit(section, default_unit);
}

void WebSocket::NotifyUpdateDefaultUnitFailure(const QJsonObject& /*obj*/) { emit SUpdateDefaultUnitFailed(kFinance); }

void WebSocket::SharedConfig(const QJsonArray& arr) { emit SSharedConfig(arr); }

QHash<QUuid, QSet<QUuid>> WebSocket::ParseNodeReference(const QJsonObject& obj)
{
    QHash<QUuid, QSet<QUuid>> map {};

    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        const QUuid key { QUuid(it.key()) };
        QSet<QUuid>& set = map[key];

        const QJsonArray arr { it.value().toArray() };
        for (const auto& val : arr) {
            set.insert(QUuid(val.toString()));
        }
    }

    return map;
}
