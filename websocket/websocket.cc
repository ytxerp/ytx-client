#include "websocket.h"

#include <QCoreApplication>
#include <QJsonArray>

#include "component/constant.h"
#include "component/using.h"
#include "tree/model/treemodelo.h"
#include "tree/model/treemodelp.h"
#include "tree/model/treemodelt.h"
#include "websocket/jsongen.h"

WebSocket::WebSocket(QObject* parent)
    : QObject(parent)
{
    InitHandler();
    InitConnect();
}

WebSocket::~WebSocket()
{
    if (heartbeat_) {
        heartbeat_->stop();
        heartbeat_->deleteLater();
        heartbeat_ = nullptr;
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
    const QString host { local_settings->value(kHost, "ytxerp.cc").toString() };
    const QString port { local_settings->value(kPort, "443").toString() };
    local_settings->endGroup();

    QString scheme {};

    if (host == "127.0.0.1" || host == "localhost") {
        scheme = "ws";
    } else {
        scheme = "wss";
    }

    const QString url_str { QString("%1://%2:%3").arg(scheme, host, port) };

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
    qInfo() << "Websocket connected";
    emit SConnectionAccepted(true);
    InitTimer();
}

void WebSocket::RDisconnected()
{
    qInfo() << "Websocket disconnected";

    if (session_id_.isEmpty()) {
        qInfo() << "Session ID is empty — disconnected before login.";
        return;
    }

    if (!manual_disconnect_)
        emit SRemoteHostClosed();

    manual_disconnect_ = false;

    session_id_.clear();
    tree_model_hash_.clear();
    entry_hub_hash_.clear();

    if (heartbeat_) {
        heartbeat_->stop();
    }
}

void WebSocket::Close()
{
    if (socket_.state() == QAbstractSocket::ConnectedState || socket_.state() == QAbstractSocket::ConnectingState) {
        manual_disconnect_ = true;
        socket_.close();
    }
}

void WebSocket::RErrorOccurred(QAbstractSocket::SocketError error)
{
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        emit SConnectionRefused();
        qWarning() << "WebSocket connection refused! (The peer refused or timed out)";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        qWarning() << "WebSocket remote host closed connection!";
        break;
    case QAbstractSocket::HostNotFoundError:
        qWarning() << "WebSocket host not found! (Invalid address or DNS failure)";
        break;
    case QAbstractSocket::SocketAccessError:
        qWarning() << "WebSocket socket access error! (Permission denied)";
        break;
    case QAbstractSocket::SocketResourceError:
        qWarning() << "WebSocket socket resource error! (Too many open sockets)";
        break;
    case QAbstractSocket::SocketTimeoutError:
        qWarning() << "WebSocket socket timeout error!";
        break;
    case QAbstractSocket::DatagramTooLargeError:
        qWarning() << "WebSocket datagram too large error! (Exceeded system limit)";
        break;
    case QAbstractSocket::NetworkError:
        qWarning() << "WebSocket network error! (Network cable unplugged or lost connection)";
        break;
    case QAbstractSocket::AddressInUseError:
        qWarning() << "WebSocket address already in use! (Port conflict)";
        break;
    case QAbstractSocket::SocketAddressNotAvailableError:
        qWarning() << "WebSocket socket address not available! (Address does not belong to host)";
        break;
    default:
        qWarning() << "WebSocket unknown error:" << error;
        break;
    }
}

void WebSocket::InitHandler()
{
    handler_obj_[kLoginResult] = [this](const QJsonObject& obj) { NotifyLoginResult(obj); };
    handler_obj_[kRegisterResult] = [this](const QJsonObject& obj) { NotifyRegisterResult(obj); };
    handler_obj_[kWorkspaceAccessPending] = [this](const QJsonObject& obj) { NotifyWorkspaceAccessPending(obj); };
    handler_arr_[kSharedConfig] = [this](const QJsonArray& arr) { ApplySharedConfig(arr); };
    handler_arr_[kPartnerEntry] = [this](const QJsonArray& arr) { ApplyPartnerEntry(arr); };

    handler_obj_[kDefaultUnit] = [this](const QJsonObject& obj) { UpdateDefaultUnit(obj); };
    handler_obj_[kUpdateDefaultUnitFailure] = [this](const QJsonObject& obj) { NotifyUpdateDefaultUnitFailure(obj); };
    handler_obj_[kDocumentDir] = [this](const QJsonObject& obj) { UpdateDocumentDir(obj); };

    handler_obj_[kTreeAcked] = [this](const QJsonObject& obj) { AckTree(obj); };
    handler_obj_[kTableAcked] = [this](const QJsonObject& obj) { AckTable(obj); };
    handler_obj_[kSaleReferenceAcked] = [this](const QJsonObject& obj) { AckSaleReference(obj); };
    handler_obj_[kStatementAcked] = [this](const QJsonObject& obj) { AckStatement(obj); };
    handler_obj_[kStatementNodeAcked] = [this](const QJsonObject& obj) { AckStatementNode(obj); };
    handler_obj_[kStatementEntryAcked] = [this](const QJsonObject& obj) { AckStatementEntry(obj); };
    handler_obj_[kSettlementAcked] = [this](const QJsonObject& obj) { AckSettlement(obj); };
    handler_obj_[kSettlementItemAcked] = [this](const QJsonObject& obj) { AckSettlementItem(obj); };
    handler_obj_[kSettlementInserted] = [this](const QJsonObject& obj) { InsertSettlement(obj); };
    handler_obj_[kSettlementUpdated] = [this](const QJsonObject& obj) { UpdateSettlement(obj); };
    handler_obj_[kSettlementRecalled] = [this](const QJsonObject& obj) { RecallSettlement(obj); };
    handler_obj_[kPartnerUpdated] = [this](const QJsonObject& obj) { UpdatePartner(obj); };
    handler_obj_[kSettlementInsertValidationFailed] = [this](const QJsonObject& obj) { SettlementValidationFailed(obj); };
    handler_obj_[kSettlementUpdateValidationFailed] = [this](const QJsonObject& obj) { SettlementValidationFailed(obj); };
    handler_obj_[kInvalidOperation] = [this](const QJsonObject& /*obj*/) { NotifyInvalidOperation(); };

    handler_obj_[kTreeApplied] = [this](const QJsonObject& obj) { ApplyTree(obj); };
    handler_obj_[kNodeInsert] = [this](const QJsonObject& obj) { InsertNode(obj); };
    handler_obj_[kNodeAcked] = [this](const QJsonObject& obj) { AckNode(obj); };
    handler_obj_[kLeafRemove] = [this](const QJsonObject& obj) { RemoveLeaf(obj); };
    handler_obj_[kBranchRemove] = [this](const QJsonObject& obj) { RemoveBranch(obj); };
    handler_obj_[kLeafReplace] = [this](const QJsonObject& obj) { ReplaceLeaf(obj); };
    handler_obj_[kNodeUpdate] = [this](const QJsonObject& obj) { UpdateNode(obj); };
    handler_obj_[kEntryInsert] = [this](const QJsonObject& obj) { InsertEntry(obj); };
    handler_obj_[kEntrySearch] = [this](const QJsonObject& obj) { SearchEntry(obj); };
    handler_obj_[kNodeSearch] = [this](const QJsonObject& obj) { SearchNode(obj); };
    handler_obj_[kEntryUpdate] = [this](const QJsonObject& obj) { UpdateEntry(obj); };
    handler_obj_[kEntryRemove] = [this](const QJsonObject& obj) { RemoveEntry(obj); };
    handler_obj_[kDirectionRule] = [this](const QJsonObject& obj) { UpdateDirectionRule(obj); };
    handler_obj_[kNodeStatus] = [this](const QJsonObject& obj) { UpdateNodeStatus(obj); };
    handler_obj_[kLeafRemoveDenied] = [this](const QJsonObject& obj) { NotifyLeafRemoveDenied(obj); };
    handler_obj_[kNodeDrag] = [this](const QJsonObject& obj) { DragNode(obj); };
    handler_obj_[kEntryAction] = [this](const QJsonObject& obj) { ActionEntry(obj); };

    handler_obj_[kNodeName] = [this](const QJsonObject& obj) { UpdateNodeName(obj); };
    handler_obj_[kEntryLinkedNode] = [this](const QJsonObject& obj) { UpdateEntryLinkedNode(obj); };
    handler_obj_[kEntryRate] = [this](const QJsonObject& obj) { UpdateEntryRate(obj); };
    handler_obj_[kEntryNumeric] = [this](const QJsonObject& obj) { UpdateEntryNumeric(obj); };
    handler_obj_[kLeafRemoveSafely] = [this](const QJsonObject& obj) { RemoveLeafSafely(obj); };
    handler_obj_[kOrderInsertSaved] = [this](const QJsonObject& obj) { InsertOrder(obj, false); };
    handler_obj_[kOrderUpdateSaved] = [this](const QJsonObject& obj) { UpdateOrder(obj, false); };
    handler_obj_[kOrderInsertReleased] = [this](const QJsonObject& obj) { InsertOrder(obj, true); };
    handler_obj_[kOrderUpdateReleased] = [this](const QJsonObject& obj) { UpdateOrder(obj, true); };
    handler_obj_[kOrderRecalled] = [this](const QJsonObject& obj) { RecallOrder(obj); };
}

void WebSocket::InitConnect()
{
    connect(&socket_, &QWebSocket::connected, this, &WebSocket::RConnected);
    connect(&socket_, &QWebSocket::disconnected, this, &WebSocket::RDisconnected);
    connect(&socket_, &QWebSocket::errorOccurred, this, &WebSocket::RErrorOccurred);
    connect(&socket_, &QWebSocket::textMessageReceived, this, &WebSocket::RReceiveMessage);

    connect(&socket_, &QWebSocket::pong, this, [this](quint64, const QByteArray&) { last_heartbeat_time_ = QDateTime::currentDateTime(); });
}

void WebSocket::InitTimer()
{
    if (!heartbeat_) {
        heartbeat_ = new QTimer(this);
        heartbeat_->setInterval(HEARTBEAT_INTERVAL);

        connect(heartbeat_, &QTimer::timeout, this, [this]() {
            if (socket_.state() != QAbstractSocket::ConnectedState) {
                return;
            }

            const auto elapsed_ms { last_heartbeat_time_.msecsTo(QDateTime::currentDateTime()) };
            if (elapsed_ms > TIMEOUT_THRESHOLD) {
                qWarning() << "Pong timeout:" << elapsed_ms << "ms, closing connection";
                socket_.abort();
                return;
            }

            socket_.ping();
        });
    }

    heartbeat_->start();
    last_heartbeat_time_ = QDateTime::currentDateTime();
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

void WebSocket::RReceiveMessage(const QString& message)
{
    QJsonParseError err {};
    const QJsonValue root { QJsonValue::fromJson(message.toUtf8(), &err) };

    if (err.error != QJsonParseError::NoError || !root.isObject()) {
        qWarning() << "Invalid Message:" << message;
        return;
    }

    last_heartbeat_time_ = QDateTime::currentDateTime();

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

    qWarning() << "Unsupported Message:" << msg_type;
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

void WebSocket::NotifyInvalidOperation() { emit SInvalidOperation(); }

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
    const QJsonObject meta { obj.value(kMeta).toObject() };

    const auto descendant { QUuid(path_obj.value(kDescendant).toString()) };
    const auto ancestor { QUuid(path_obj.value(kAncestor).toString()) };

    auto tree_model { tree_model_hash_.value(section) };

    if (session_id != session_id_)
        tree_model->InsertNode(ancestor, node_obj);

    tree_model->InsertMeta(descendant, meta);
}

void WebSocket::UpdateNode(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };
    const QJsonObject update { obj.value(kUpdate).toObject() };
    const QJsonObject meta { obj.value(kMeta).toObject() };

    auto tree_model { tree_model_hash_.value(section) };

    if (session_id != session_id_)
        tree_model->SyncNode(node_id, update);

    tree_model->UpdateMeta(node_id, meta);
}

void WebSocket::DragNode(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const QJsonObject path { obj.value(kPath).toObject() };

    CString ancestor { path.value(kAncestor).toString() };
    CString descendant { path.value(kDescendant).toString() };

    if (ancestor.isEmpty() || descendant.isEmpty()) {
        qWarning() << "Invalid drag node obj";
        return;
    }

    auto tree_model { tree_model_hash_.value(section) };

    if (session_id != session_id_)
        tree_model->DragNode(QUuid(ancestor), QUuid(descendant));
}

void WebSocket::ApplyTree(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    auto tree_model { tree_model_hash_.value(section) };
    tree_model->ApplyTree(obj);
}

void WebSocket::ApplyPartnerEntry(const QJsonArray& arr)
{
    auto entry_hub { entry_hub_hash_.value(Section::kPartner) };
    entry_hub->ApplyPartnerEntry(arr);
}

void WebSocket::AckTree(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    auto tree_model { tree_model_hash_.value(section) };
    tree_model->AckTree(obj);
}

void WebSocket::AckTable(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };
    const auto entry_id { QUuid(obj.value(kEntryId).toString()) };
    const QJsonArray array { obj.value(kEntryArray).toArray() };

    auto entry_hub { entry_hub_hash_.value(section) };
    entry_hub->AckTable(node_id, array);

    if (!entry_id.isNull())
        emit SSelectLeafEntry(node_id, entry_id);
}

void WebSocket::AckNode(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };

    const QJsonObject leaf_obj { obj.value(kNode).toObject() };
    const QUuid ancestor { QUuid(obj.value(kAncestor).toString()) };

    auto tree_model { tree_model_hash_.value(section) };
    tree_model->AckNode(leaf_obj, ancestor);
}

void WebSocket::AckSaleReference(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QUuid widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const QJsonArray array { obj.value(kArray).toArray() };

    emit SSaleReference(section, widget_id, array);
}

void WebSocket::AckStatement(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QUuid widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const QJsonArray array { obj.value(kArray).toArray() };

    emit SStatement(section, widget_id, array);
}

void WebSocket::AckStatementNode(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QUuid widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const QJsonArray array { obj.value(kArray).toArray() };

    emit SStatementNodeAcked(section, widget_id, array);
}

void WebSocket::AckStatementEntry(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QUuid widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const QJsonArray array { obj.value(kArray).toArray() };
    const QJsonObject total { obj.value(kTotal).toObject() };

    emit SStatementEntryAcked(section, widget_id, array, total);
}

void WebSocket::AckSettlement(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QUuid widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const QJsonArray array { obj.value(kArray).toArray() };

    emit SSettlement(section, widget_id, array);
}

void WebSocket::AckSettlementItem(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QUuid widget_id { QUuid(obj.value(kWidgetId).toString()) };
    const QJsonArray array { obj.value(kArray).toArray() };

    emit SSettlementItemAcked(section, widget_id, array);
}

void WebSocket::RemoveLeaf(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };

    const QJsonObject linked_entry_obj { obj.value(kLinkedEntry).toObject() };
    const QJsonArray total_array { obj.value(kTotalArray).toArray() };

    const auto leaf_entry { ParseNodeReference(linked_entry_obj) };

    auto entry_hub { entry_hub_hash_.value(section) };
    auto tree_model { tree_model_hash_.value(section) };

    entry_hub->RemoveLeaf(leaf_entry);
    tree_model->SyncTotalArray(total_array);

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
    const bool inventory_outside_ref { obj.value(kInventoryOutsideRef).toBool() };
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

    if (section == Section::kInventory && inventory_outside_ref) {
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
    const QJsonObject update { obj.value(kUpdate).toObject() };
    const QJsonObject meta { obj.value(kMeta).toObject() };

    auto entry_hub { entry_hub_hash_.value(section) };

    assert(entry_hub);
    assert(!entry_id.isNull());

    if (session_id != session_id_) {
        entry_hub->UpdateEntry(entry_id, update);
    }

    entry_hub->UpdateMeta(entry_id, meta);
}

void WebSocket::UpdateEntryLinkedNode(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const QUuid entry_id { obj.value(kEntryId).toString() };
    const bool is_parallel { obj.value(kIsParallel).toBool() };
    const QJsonObject meta { obj.value(kMeta).toObject() };
    const QJsonObject update { obj.value(kUpdate).toObject() };

    auto entry_hub { entry_hub_hash_.value(section) };
    auto tree_model { tree_model_hash_.value(section) };

    QJsonObject lhs_total {};
    QJsonObject rhs_total {};

    const bool has_total { obj.contains(kLhsTotal) && obj.value(kLhsTotal).isObject() && obj.contains(kRhsTotal) && obj.value(kRhsTotal).isObject() };
    if (has_total) {
        lhs_total = obj.value(kLhsTotal).toObject();
        rhs_total = obj.value(kRhsTotal).toObject();

        tree_model->UpdateMeta(QUuid(lhs_total.value(kId).toString()), meta);
        tree_model->UpdateMeta(QUuid(rhs_total.value(kId).toString()), meta);

        const QJsonArray total_array { lhs_total, rhs_total };
        tree_model->SyncTotalArray(total_array);
    }

    if (session_id != session_id_) {
        entry_hub->UpdateEntryLinkedNode(entry_id, update, is_parallel);
    }

    entry_hub->UpdateMeta(entry_id, meta);
}

void WebSocket::UpdateEntryRate(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };

    const auto entry_id { QUuid(obj.value(kEntryId).toString()) };
    const QJsonObject update { obj.value(kUpdate).toObject() };
    const bool is_parallel { obj.value(kIsParallel).toBool() };
    const QJsonObject meta { obj.value(kMeta).toObject() };

    auto entry_hub { entry_hub_hash_.value(section) };
    auto tree_model { tree_model_hash_.value(section) };

    assert(entry_hub);
    assert(tree_model);

    QJsonObject lhs_total {};
    QJsonObject rhs_total {};

    const bool has_total { obj.contains(kLhsTotal) && obj.value(kLhsTotal).isObject() && obj.contains(kRhsTotal) && obj.value(kRhsTotal).isObject() };
    if (has_total) {
        lhs_total = obj.value(kLhsTotal).toObject();
        rhs_total = obj.value(kRhsTotal).toObject();

        tree_model->UpdateMeta(QUuid(lhs_total.value(kId).toString()), meta);
        tree_model->UpdateMeta(QUuid(rhs_total.value(kId).toString()), meta);

        const QJsonArray total_array { lhs_total, rhs_total };
        tree_model->SyncTotalArray(total_array);
    }

    if (session_id != session_id_) {
        entry_hub->UpdateEntryRate(entry_id, update, is_parallel);
    }

    entry_hub->UpdateMeta(entry_id, update);
}

void WebSocket::UpdateEntryNumeric(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };

    const auto entry_id { QUuid(obj.value(kEntryId).toString()) };
    const QJsonObject update { obj.value(kUpdate).toObject() };
    const QJsonObject meta { obj.value(kMeta).toObject() };

    auto entry_hub { entry_hub_hash_.value(section) };
    auto tree_model { tree_model_hash_.value(section) };

    assert(entry_hub);
    assert(tree_model);

    QJsonObject lhs_total {};
    QJsonObject rhs_total {};

    const bool has_total { obj.contains(kLhsTotal) && obj.value(kLhsTotal).isObject() && obj.contains(kRhsTotal) && obj.value(kRhsTotal).isObject() };
    if (has_total) {
        lhs_total = obj.value(kLhsTotal).toObject();
        rhs_total = obj.value(kRhsTotal).toObject();

        tree_model->UpdateMeta(QUuid(lhs_total.value(kId).toString()), meta);
        tree_model->UpdateMeta(QUuid(rhs_total.value(kId).toString()), meta);

        const QJsonArray total_array { lhs_total, rhs_total };
        tree_model->SyncTotalArray(total_array);
    }

    if (session_id != session_id_) {
        entry_hub->UpdateEntryNumeric(entry_id, update);
    }

    entry_hub->UpdateMeta(entry_id, meta);
}

void WebSocket::SearchEntry(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    const QJsonArray array { obj.value(kEntryArray).toArray() };

    auto entry_hub { entry_hub_hash_.value(section) };
    entry_hub->SearchEntry(array);
}

void WebSocket::SearchNode(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    auto tree_model { tree_model_hash_.value(section) };
    emit SNodeSearch(obj);
}

void WebSocket::InsertEntry(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const auto entry_id { QUuid(obj.value(kEntryId).toString()) };

    const QJsonObject entry { obj.value(kEntry).toObject() };
    const QJsonObject meta { obj.value(kMeta).toObject() };

    auto entry_hub { entry_hub_hash_.value(section) };
    auto tree_model { tree_model_hash_.value(section) };

    assert(entry_hub);
    assert(tree_model);

    QJsonObject lhs_total {};
    QJsonObject rhs_total {};

    const bool has_total { obj.contains(kLhsTotal) && obj.value(kLhsTotal).isObject() && obj.contains(kRhsTotal) && obj.value(kRhsTotal).isObject() };
    if (has_total) {
        lhs_total = obj.value(kLhsTotal).toObject();
        rhs_total = obj.value(kRhsTotal).toObject();

        tree_model->UpdateMeta(QUuid(lhs_total.value(kId).toString()), meta);
        tree_model->UpdateMeta(QUuid(rhs_total.value(kId).toString()), meta);

        const QJsonArray total_array { lhs_total, rhs_total };
        tree_model->SyncTotalArray(total_array);
    }

    if (session_id != session_id_) {
        entry_hub->InsertEntry(entry);
    }

    entry_hub->InsertMeta(entry_id, meta);
}

void WebSocket::RemoveEntry(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const auto entry_id { QUuid(obj.value(kEntryId).toString()) };
    const QJsonObject meta { obj.value(kMeta).toObject() };

    auto entry_hub { entry_hub_hash_.value(section) };
    auto tree_model { tree_model_hash_.value(section) };

    assert(entry_hub);
    assert(tree_model);

    QJsonObject lhs_total {};
    QJsonObject rhs_total {};

    const bool has_total { obj.contains(kLhsTotal) && obj.value(kLhsTotal).isObject() && obj.contains(kRhsTotal) && obj.value(kRhsTotal).isObject() };
    if (has_total) {
        lhs_total = obj.value(kLhsTotal).toObject();
        rhs_total = obj.value(kRhsTotal).toObject();

        tree_model->UpdateMeta(QUuid(lhs_total.value(kId).toString()), meta);
        tree_model->UpdateMeta(QUuid(rhs_total.value(kId).toString()), meta);

        const QJsonArray total_array { lhs_total, rhs_total };
        tree_model->SyncTotalArray(total_array);
    }

    if (session_id != session_id_) {
        entry_hub->RemoveEntry(entry_id);
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

    if (session_id != session_id_) {
        tree_model->SyncDirectionRule(node_id, direction_rule);
    }

    tree_model->UpdateMeta(node_id, meta);
}

void WebSocket::UpdateNodeStatus(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const int status { obj.value(kStatus).toInt() };
    const auto node_id { QUuid(obj.value(kNodeId).toString()) };
    const QJsonObject meta { obj.value(kMeta).toObject() };

    auto* base_model { tree_model_hash_.value(section).data() };

    if (session_id != session_id_) {
        auto* task_model { static_cast<TreeModelT*>(base_model) };

        task_model->UpdateNodeStatus(node_id, status);
    }

    base_model->UpdateMeta(node_id, meta);
}

void WebSocket::UpdateNodeName(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };

    const auto node_id { QUuid(obj.value(kNodeId).toString()) };
    const auto name { obj.value(kName).toString() };
    const QJsonObject meta { obj.value(kMeta).toObject() };

    auto tree_model { tree_model_hash_.value(section) };

    if (session_id != session_id_) {
        tree_model->SyncNodeName(node_id, name);
    }

    tree_model->UpdateMeta(node_id, meta);
}

void WebSocket::UpdateOrder(const QJsonObject& obj, bool is_release)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };

    const auto node_id { QUuid(obj.value(kNodeId).toString()) };

    auto* base_model { tree_model_hash_.value(section).data() };
    auto* order_model = static_cast<TreeModelO*>(base_model);
    assert(order_model != nullptr);

    if (!order_model->Contains(node_id))
        return;

    const auto node_update { obj.value(kNodeUpdate).toObject() };
    const QJsonObject meta { obj.value(kMeta).toObject() };

    order_model->SyncNode(node_id, node_update);
    order_model->UpdateMeta(node_id, meta);

    if (is_release)
        order_model->RNodeStatus(node_id, NodeStatus::kReleased);
}

void WebSocket::InsertOrder(const QJsonObject& obj, bool is_release)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const QJsonObject node_obj { obj.value(kNode).toObject() };
    const QJsonObject path_obj { obj.value(kPath).toObject() };

    const auto descendant { QUuid(path_obj.value(kDescendant).toString()) };
    const auto ancestor { QUuid(path_obj.value(kAncestor).toString()) };
    const auto node_id { QUuid(node_obj.value(kId).toString()) };
    const QJsonObject meta { obj.value(kMeta).toObject() };

    auto* base_model { tree_model_hash_.value(section).data() };

    auto* order_model = static_cast<TreeModelO*>(base_model);
    assert(order_model != nullptr);

    base_model->InsertNode(ancestor, node_obj);

    if (is_release)
        order_model->RNodeStatus(node_id, NodeStatus::kReleased);

    if (session_id == session_id_ && is_release) {
        emit SOrderReleased(section, node_id);
    }

    order_model->InsertMeta(descendant, meta);
}

void WebSocket::RecallOrder(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };

    const auto node_id { QUuid(obj.value(kNodeId).toString()) };
    auto* base_model { tree_model_hash_.value(section).data() };

    auto* order_model = static_cast<TreeModelO*>(base_model);
    assert(order_model != nullptr);

    if (!order_model->Contains(node_id))
        return;

    const auto node_update { obj.value(kNodeUpdate).toObject() };
    const QJsonObject meta { obj.value(kMeta).toObject() };

    order_model->SyncNode(node_id, node_update);
    order_model->RNodeStatus(node_id, NodeStatus::kRecalled);

    if (session_id == session_id_) {
        emit SOrderRecalled(section, node_id);
    }

    order_model->UpdateMeta(node_id, meta);
}

void WebSocket::InsertSettlement(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const QUuid settlement_id { QUuid(obj.value(kSettlementId).toString()) };
    const QJsonArray array { obj.value(kSettlementItemInserted).toArray() };

    auto* base_model { tree_model_hash_.value(section).data() };

    auto* order_model = static_cast<TreeModelO*>(base_model);
    assert(order_model != nullptr);

    for (const auto& v : array) {
        const QUuid node_id { v.toString() };
        order_model->InsertSettlement(node_id, settlement_id);
    }

    if (session_id == session_id_) {
        emit SSettlementInserted(obj);
    }
}

void WebSocket::UpdateSettlement(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const QUuid settlement_id { QUuid(obj.value(kSettlementId).toString()) };
    const QJsonArray insert_array { obj.value(kSettlementItemInserted).toArray() };
    const QJsonArray delete_array { obj.value(kSettlementItemDeleted).toArray() };

    auto* base_model { tree_model_hash_.value(section).data() };

    auto* order_model = static_cast<TreeModelO*>(base_model);
    assert(order_model != nullptr);

    for (const auto& v : insert_array) {
        const QUuid node_id { v.toString() };
        order_model->InsertSettlement(node_id, settlement_id);
    }

    for (const auto& v : delete_array) {
        const QUuid node_id { v.toString() };
        order_model->DeleteSettlement(settlement_id);
    }

    if (session_id == session_id_) {
        emit SSettlementUpdated(obj);
    }
}

void WebSocket::RecallSettlement(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };
    const QUuid settlement_id { QUuid(obj.value(kSettlementId).toString()) };

    auto* base_model { tree_model_hash_.value(section).data() };

    auto* order_model = static_cast<TreeModelO*>(base_model);
    assert(order_model != nullptr);

    order_model->RecallSettlement(settlement_id);

    if (session_id == session_id_) {
        emit SSettlementRecalled(obj);
    }
}

void WebSocket::UpdatePartner(const QJsonObject& obj)
{
    auto* base_model { tree_model_hash_.value(Section::kPartner).data() };
    auto* partner_model = static_cast<TreeModelP*>(base_model);

    const auto partner_id { QUuid(obj.value(kId).toString()) };
    const double initial_delta { obj.value(kInitialDelta).toString().toDouble() };

    partner_model->RUpdateAmount(partner_id, initial_delta);
}

void WebSocket::SettlementValidationFailed(const QJsonObject& /*obj*/) { emit SSettlementValidationFailed(); }

void WebSocket::ActionEntry(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    CString session_id { obj.value(kSessionId).toString() };

    const auto node_id { QUuid(obj.value(kNodeId).toString()) };
    const EntryAction action { EntryAction(obj.value(kAction).toInt()) };

    auto entry_hub { entry_hub_hash_.value(section) };

    if (session_id != session_id_) {
        entry_hub->ActionEntry(node_id, action);
    }
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

void WebSocket::ApplySharedConfig(const QJsonArray& arr) { emit SSharedConfig(arr); }

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
