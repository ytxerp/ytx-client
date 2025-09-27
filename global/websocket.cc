#include "websocket.h"

#include <QCoreApplication>
#include <QJsonArray>

#include "component/constant.h"
#include "component/using.h"
#include "utils/jsongen.h"

WebSocket::WebSocket(QObject* parent)
    : QObject(parent)
{
    InitHandler();
    InitConnect();
    InitTimer();
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
    const QString host = local_settings->value(kHost).toString();
    const QString port = local_settings->value(kPort).toString();
    local_settings->endGroup();

    const QString url_str = (!host.isEmpty() && !port.isEmpty()) ? QString("ws://%1:%2").arg(host, port) : QString("wss://ytxerp.cc");

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
    emit SActionLoginTriggered();

    if (ping_timer_) {
        ping_timer_->start();
    }
}

void WebSocket::RDisconnected()
{
    session_id_.clear();
    emit SConnectResult(false);

    if (ping_timer_) {
        ping_timer_->stop();
    }
}

void WebSocket::RErrorOccurred(QAbstractSocket::SocketError error) { qWarning() << "RErrorOccurred" << error << "-" << socket_.errorString(); }

void WebSocket::InitHandler()
{
    handler_obj_[kLoginSuccess] = [this](const QJsonObject& obj) { AckLoginSuccess(obj); };
    handler_obj_[kLoginFailed] = [this](const QJsonObject& obj) { AckLoginFailed(obj); };
    handler_obj_[kRegisterResult] = [this](const QJsonObject& obj) { AckRegisterResult(obj); };
    handler_obj_[kTreeAcked] = [this](const QJsonObject& obj) { AckTree(obj); };
    handler_obj_[kLeafAcked] = [this](const QJsonObject& obj) { AckLeaf(obj); };

    handler_obj_[kWorkspaceAccessPending] = [this](const QJsonObject& obj) { AckWorkspaceAccessPending(obj); };
    handler_obj_[kTreeApplied] = [this](const QJsonObject& obj) { ApplyTree(obj); };
    handler_obj_[kNodeInsert] = [this](const QJsonObject& obj) { ApplyNodeInsert(obj); };
    handler_obj_[kLeafRemove] = [this](const QJsonObject& obj) { ApplyLeafRemove(obj); };
    handler_obj_[kBranchRemove] = [this](const QJsonObject& obj) { ApplyBranchRemove(obj); };
    handler_obj_[kLeafReplace] = [this](const QJsonObject& obj) { ApplyLeafReplace(obj); };
    handler_obj_[kNodeUpdate] = [this](const QJsonObject& obj) { ApplyNodeUpdate(obj); };
    handler_obj_[kEntryInsert] = [this](const QJsonObject& obj) { ApplyEntryInsert(obj); };
    handler_obj_[kEntrySearch] = [this](const QJsonObject& obj) { AckEntrySearch(obj); };
    handler_obj_[kEntryUpdate] = [this](const QJsonObject& obj) { ApplyEntryUpdate(obj); };
    handler_obj_[kEntryRemove] = [this](const QJsonObject& obj) { ApplyEntryRemove(obj); };
    handler_obj_[kDirectionRule] = [this](const QJsonObject& obj) { ApplyDirectionRule(obj); };
    handler_obj_[kLeafReference] = [this](const QJsonObject& obj) { AckLeafReference(obj); };
    handler_obj_[kNodeDrag] = [this](const QJsonObject& obj) { ApplyNodeDrag(obj); };
    handler_obj_[kCheckAction] = [this](const QJsonObject& obj) { ApplyCheckAction(obj); };
    handler_obj_[kDefaultUnit] = [this](const QJsonObject& obj) { ApplyDefaultUnit(obj); };
    handler_obj_[kUpdateDefaultUnitFailed] = [this](const QJsonObject& obj) { AckUpdateDefaultUnitFailed(obj); };
    handler_obj_[kDocumentDir] = [this](const QJsonObject& obj) { ApplyDocumentDir(obj); };
    handler_obj_[kName] = [this](const QJsonObject& obj) { ApplyName(obj); };
    handler_obj_[kEntryRhsNode] = [this](const QJsonObject& obj) { ApplyEntryRhsNode(obj); };
    handler_obj_[kEntryRate] = [this](const QJsonObject& obj) { ApplyEntryRate(obj); };
    handler_obj_[kEntryNumeric] = [this](const QJsonObject& obj) { ApplyEntryNumeric(obj); };
    handler_obj_[kUnreferencedNodeRemove] = [this](const QJsonObject& obj) { ApplyUnreferencedNodeRemove(obj); };

    handler_arr_[kGlobalConfig] = [this](const QJsonArray& arr) { ApplyGlobalConfig(arr); };
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

    Q_ASSERT(ping_timer_ != nullptr);
}

void WebSocket::SendMessage(const QString& type, const QJsonObject& value)
{
    if (socket_.state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send message: WebSocket is not connected";
        return;
    }

    const QJsonValue message = QJsonObject { { kMsgType, type }, { kValue, value } };
    const QByteArray json = message.toJson(QJsonDocument::Compact);

    socket_.sendTextMessage(QString::fromUtf8(json));
}

void WebSocket::Clear()
{
    if (socket_.state() == QAbstractSocket::ConnectedState || socket_.state() == QAbstractSocket::ConnectingState) {
        connect(&socket_, &QWebSocket::disconnected, this, [this]() { this->Connect(); }, Qt::SingleShotConnection);
        socket_.close();
    }

    tree_model_hash_.clear();
    entry_hub_hash_.clear();
}

void WebSocket::RReceiveMessage(const QString& message)
{
    QJsonParseError err {};
    const QJsonValue root = QJsonValue::fromJson(message.toUtf8(), &err);

    if (err.error != QJsonParseError::NoError || !root.isObject()) {
        qWarning() << "Invalid JSON message:" << message;
        return;
    }

    const QJsonObject obj = root.toObject();
    const QString msg_type = obj.value(kMsgType).toString();
    const QJsonValue value = obj.value(kValue);

    if (value.isObject()) {
        if (const auto it = handler_obj_.constFind(msg_type); it != handler_obj_.constEnd()) {
            it.value()(value.toObject());
            return;
        }
    }

    if (value.isArray()) {
        if (const auto it = handler_arr_.constFind(msg_type); it != handler_arr_.constEnd()) {
            it.value()(value.toArray());
            return;
        }
    }

    qWarning() << "Unsupported value type for message:" << msg_type;
}

void WebSocket::AckLoginFailed(const QJsonObject& /*obj*/) { emit SLoginResult(false); }

void WebSocket::AckLoginSuccess(const QJsonObject& obj)
{
    session_id_ = obj[kSessionId].toString();
    const auto expire_time = QDateTime::fromString(obj[kExpireTime].toString(), Qt::ISODate);
    const auto expire_date = expire_time.date().toString(kDateFST);

    emit SLoginResult(true);
    emit SInitializeContext(expire_date);
}

void WebSocket::AckWorkspaceAccessPending(const QJsonObject& obj)
{
    CString email = obj.value(kEmail).toString();
    CString workspace = obj.value(kWorkspace).toString();

    emit SWorkspaceAccessPending(email, workspace);
}

void WebSocket::AckRegisterResult(const QJsonObject& obj)
{
    const bool result = obj[kResult].toBool();
    emit SRegisterResult(result);
}

void WebSocket::ApplyNodeInsert(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();
    const QJsonObject node_obj = obj.value(kNode).toObject();
    const QJsonObject path_obj = obj.value(kPath).toObject();

    const auto descendant = QUuid(path_obj.value(kDescendant).toString());
    const auto ancestor = QUuid(path_obj.value(kAncestor).toString());

    auto tree_model = tree_model_hash_.value(section);

    if (session_id == session_id_)
        tree_model->ApplyMetaInsert(descendant, node_obj);
    else
        tree_model->ApplyNodeInsert(ancestor, node_obj);
}

void WebSocket::ApplyNodeUpdate(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();
    const auto id = QUuid(obj.value(kId).toString());

    const QJsonObject data = obj.value(kCache).toObject();

    auto tree_model = tree_model_hash_.value(section);

    if (session_id == session_id_)
        tree_model->ApplyMetaUpdate(id, data);
    else
        tree_model->ApplyNodeUpdate(id, data);
}

void WebSocket::ApplyNodeDrag(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();

    const QJsonObject path = obj.value(kPath).toObject();
    const QJsonObject node = obj.value(kNode).toObject();

    CString ancestor = path.value(kAncestor).toString();
    CString descendant = path.value(kDescendant).toString();

    if (section.isEmpty() || ancestor.isEmpty() || descendant.isEmpty() || node.isEmpty()) {
        qWarning() << "Invalid drag node obj";
        return;
    }

    auto tree_model = tree_model_hash_.value(section);

    if (session_id == session_id_)
        tree_model->ApplyNodeUpdate(QUuid(descendant), node);
    else
        tree_model->ApplyNodeDrag(QUuid(ancestor), QUuid(descendant), node);
}

void WebSocket::ApplyTree(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    auto tree_model = tree_model_hash_.value(section);
    tree_model->ApplyTree(obj);
}

void WebSocket::AckTree(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    auto tree_model = tree_model_hash_.value(section);
    tree_model->AckTree(obj);
}

void WebSocket::AckLeaf(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    const auto id = QUuid(obj.value(kNodeId).toString());
    const QJsonArray array { obj.value(kEntryArray).toArray() };

    auto entry_hub = entry_hub_hash_.value(section);
    entry_hub->AckLeafTable(id, array);
}

void WebSocket::AckOneNode(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();

    const QJsonObject leaf_obj = obj.value(kNode).toObject();
    const QUuid ancestor = QUuid(obj.value(kAncestor).toString());

    auto tree_model = tree_model_hash_.value(section);
    tree_model->AckOneNode(leaf_obj, ancestor);
}

void WebSocket::ApplyLeafRemove(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();
    const auto id = QUuid(obj.value(kId).toString());

    const QJsonObject leaf_obj = obj.value(kLeafEntry).toObject();
    const QJsonArray node_delta = obj.value(kNodeDelta).toArray();

    const auto leaf_entry = ParseNodeReference(leaf_obj);

    auto entry_hub = entry_hub_hash_.value(section);
    auto tree_model = tree_model_hash_.value(section);

    entry_hub->ApplyLeafRemove(leaf_entry);

    ApplyUpdateTotal(section, node_delta);

    if (session_id != session_id_)
        tree_model->RRemoveNode(QUuid(id));
}

void WebSocket::ApplyUnreferencedNodeRemove(const QJsonObject& obj)
{
    assert(obj.contains(kSection));
    assert(obj.contains(kId));

    CString section = obj.value(kSection).toString();
    const auto id = QUuid(obj.value(kId).toString());

    auto tree_model = tree_model_hash_.value(section);
    tree_model->RRemoveNode(id);
}

void WebSocket::ApplyBranchRemove(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();
    const auto id = QUuid(obj.value(kId).toString());

    auto tree_model = tree_model_hash_.value(section);

    if (session_id != session_id_)
        tree_model->RRemoveNode(QUuid(id));
}

void WebSocket::AckLeafReference(const QJsonObject& obj) { emit SRemoveLeafNode(obj); }

void WebSocket::ApplyLeafReplace(const QJsonObject& obj)
{
    const CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();
    const bool status = obj.value(kStatus).toBool();
    const bool external_reference = obj.value(kExternalReference).toBool();
    const QUuid old_id(obj.value(kOldId).toString());
    const QUuid new_id(obj.value(kNewId).toString());

    if (session_id == session_id_) {
        emit SReplaceResult(status);
    }

    if (!status) {
        qWarning() << "ReplaceLeafNode failed for section" << section << "from" << old_id << "to" << new_id;
        return;
    }

    auto entry_hub = entry_hub_hash_.value(section);
    entry_hub->ApplyLeafReplace(old_id, new_id);

    if (section == kItem && external_reference) {
        entry_hub_hash_.value(kSale)->ApplyItemReplace(old_id, new_id);
        entry_hub_hash_.value(kPurchase)->ApplyItemReplace(old_id, new_id);
        entry_hub_hash_.value(kPartner)->ApplyItemReplace(old_id, new_id);
    }

    auto tree_model = tree_model_hash_.value(section);
    tree_model->ApplyLeafReplace(old_id, new_id);
}

void WebSocket::ApplyEntryUpdate(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();
    const auto id = QUuid(obj.value(kId).toString());
    const QJsonObject cache = obj.value(kCache).toObject();

    auto entry_hub = entry_hub_hash_.value(section);
    auto tree_model = tree_model_hash_.value(section);

    assert(entry_hub);
    assert(tree_model);
    assert(!id.isNull());

    if (session_id == session_id_) {
        entry_hub->ApplyMetaUpdate(id, cache);
        return;
    }

    entry_hub->ApplyEntryUpdate(id, cache);
}

void WebSocket::ApplyEntryRhsNode(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();

    const QJsonObject entry = obj.value(kEntry).toObject();
    const QJsonObject old_rhs_delta = obj.value(kOldRhsDelta).toObject();
    const QJsonObject new_rhs_delta = obj.value(kNewRhsDelta).toObject();

    const QUuid entry_id { obj.value(kEntryId).toString() };
    const QUuid old_rhs_id { obj.value(kOldRhsId).toString() };
    const QUuid new_rhs_id { obj.value(kNewRhsId).toString() };

    auto entry_hub = entry_hub_hash_.value(section);
    auto tree_model = tree_model_hash_.value(section);

    if (session_id == session_id_) {
        entry_hub->ApplyMetaUpdate(entry_id, entry);

        if (!old_rhs_delta.isEmpty())
            tree_model->ApplyMetaUpdate(old_rhs_id, old_rhs_delta);

        if (!new_rhs_delta.isEmpty())
            tree_model->ApplyMetaUpdate(new_rhs_id, new_rhs_delta);

        return;
    }

    entry_hub->ApplyEntryRhsNode(entry_id, old_rhs_id, new_rhs_id, entry);

    if (!old_rhs_delta.isEmpty())
        tree_model->ApplyDelta(old_rhs_delta);

    if (!new_rhs_delta.isEmpty())
        tree_model->ApplyDelta(new_rhs_delta);
}

void WebSocket::ApplyEntryRate(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();

    const auto entry_id = QUuid(obj.value(kEntryId).toString());
    const QJsonObject cache = obj.value(kCache).toObject();
    const QJsonObject lhs_node = obj.value(kLhsDelta).toObject();
    const QJsonObject rhs_node = obj.value(kRhsDelta).toObject();
    const bool is_parallel = obj.value(kIsParallel).toBool();

    auto entry_hub = entry_hub_hash_.value(section);
    auto tree_model = tree_model_hash_.value(section);

    assert(entry_hub);
    assert(tree_model);

    if (session_id == session_id_) {
        entry_hub->ApplyMetaUpdate(entry_id, cache);

        if (!lhs_node.isEmpty())
            tree_model->ApplyMetaUpdate(QUuid(lhs_node.value(kId).toString()), lhs_node);

        if (!rhs_node.isEmpty())
            tree_model->ApplyMetaUpdate(QUuid(rhs_node.value(kId).toString()), rhs_node);

        return;
    }

    entry_hub->ApplyEntryRate(entry_id, cache, is_parallel);

    if (!lhs_node.isEmpty())
        tree_model->ApplyDelta(lhs_node);

    if (!rhs_node.isEmpty())
        tree_model->ApplyDelta(rhs_node);
}

void WebSocket::ApplyEntryNumeric(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();

    const auto entry_id = QUuid(obj.value(kEntryId).toString());
    const QJsonObject cache = obj.value(kCache).toObject();
    const QJsonObject lhs_node = obj.value(kLhsDelta).toObject();
    const QJsonObject rhs_node = obj.value(kRhsDelta).toObject();
    const bool is_parallel = obj.value(kIsParallel).toBool();

    auto entry_hub = entry_hub_hash_.value(section);
    auto tree_model = tree_model_hash_.value(section);

    assert(entry_hub);
    assert(tree_model);

    if (session_id == session_id_) {
        entry_hub->ApplyMetaUpdate(entry_id, cache);

        if (!lhs_node.isEmpty())
            tree_model->ApplyMetaUpdate(QUuid(lhs_node.value(kId).toString()), lhs_node);

        if (!rhs_node.isEmpty())
            tree_model->ApplyMetaUpdate(QUuid(rhs_node.value(kId).toString()), rhs_node);

        return;
    }

    entry_hub->ApplyEntryNumeric(entry_id, cache, is_parallel);

    if (!lhs_node.isEmpty())
        tree_model->ApplyDelta(lhs_node);

    if (!rhs_node.isEmpty())
        tree_model->ApplyDelta(rhs_node);
}

void WebSocket::AckEntrySearch(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    const QJsonArray array { obj.value(kEntryArray).toArray() };

    auto entry_hub = entry_hub_hash_.value(section);
    entry_hub->AckEntrySearch(array);
}

void WebSocket::ApplyEntryInsert(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();
    const auto entry_id = QUuid(obj.value(kEntryId).toString());

    const QJsonObject entry = obj.value(kEntry).toObject();
    const QJsonObject lhs_node = obj.value(kLhsDelta).toObject();
    const QJsonObject rhs_node = obj.value(kRhsDelta).toObject();

    auto entry_hub = entry_hub_hash_.value(section);
    auto tree_model = tree_model_hash_.value(section);

    assert(entry_hub);
    assert(tree_model);

    if (session_id == session_id_) {
        entry_hub->ApplyMetaInsert(entry_id, entry);

        if (!lhs_node.isEmpty())
            tree_model->ApplyMetaUpdate(QUuid(lhs_node.value(kId).toString()), lhs_node);

        if (!rhs_node.isEmpty())
            tree_model->ApplyMetaUpdate(QUuid(rhs_node.value(kId).toString()), rhs_node);

        return;
    }

    entry_hub->ApplyEntryInsert(entry);

    if (!lhs_node.isEmpty())
        tree_model->ApplyDelta(lhs_node);

    if (!rhs_node.isEmpty())
        tree_model->ApplyDelta(rhs_node);
}

void WebSocket::ApplyEntryRemove(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();
    const auto id = QUuid(obj.value(kId).toString());
    const QJsonObject lhs_node = obj.value(kLhsNode).toObject();
    const QJsonObject rhs_node = obj.value(kRhsNode).toObject();

    auto entry_hub = entry_hub_hash_.value(section);
    auto tree_model = tree_model_hash_.value(section);

    assert(entry_hub);
    assert(tree_model);

    if (session_id == session_id_) {
        if (!lhs_node.isEmpty())
            tree_model->ApplyMetaUpdate(QUuid(lhs_node.value(kId).toString()), lhs_node);

        if (!rhs_node.isEmpty())
            tree_model->ApplyMetaUpdate(QUuid(rhs_node.value(kId).toString()), rhs_node);

        return;
    }

    entry_hub->ApplyEntryRemove(id);

    if (!lhs_node.isEmpty())
        tree_model->ApplyDelta(lhs_node);

    if (!rhs_node.isEmpty())
        tree_model->ApplyDelta(rhs_node);
}

void WebSocket::ApplyDirectionRule(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();
    const bool direction_rule = obj.value(kDirectionRule).toBool();
    const auto id = QUuid(obj.value(kId).toString());
    const QJsonObject meta = obj.value(kMeta).toObject();

    auto tree_model = tree_model_hash_.value(section);

    if (session_id == session_id_) {
        tree_model->ApplyMetaUpdate(id, meta);
        return;
    }

    tree_model->ApplyDirectionRule(id, direction_rule, meta);
}

void WebSocket::ApplyName(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();

    const auto id = QUuid(obj.value(kId).toString());
    const QJsonObject data = obj.value(kCache).toObject();

    auto tree_model = tree_model_hash_.value(section);

    if (session_id == session_id_) {
        tree_model->ApplyMetaUpdate(id, data);
        return;
    }

    tree_model->ApplyName(id, data);
}

void WebSocket::ApplyCheckAction(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();

    const auto node_id = QUuid(obj.value(kNodeId).toString());
    const Check check = Check(obj.value(kCheck).toInt());
    const QJsonObject meta = obj.value(kMeta).toObject();

    auto entry_hub = entry_hub_hash_.value(section);

    if (session_id == session_id_) {
        entry_hub->ApplyCheckActionMeta(node_id, meta);
        return;
    }

    entry_hub->ApplyCheckAction(node_id, check, meta);
}

void WebSocket::ApplyDocumentDir(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    CString session_id = obj.value(kSessionId).toString();
    CString document_dir = obj.value(kDocumentDir).toString();

    if (session_id != session_id_) {
        emit SDocumentDir(section, document_dir);
    }
}

void WebSocket::ApplyDefaultUnit(const QJsonObject& obj)
{
    CString section = obj.value(kSection).toString();
    const int default_unit = obj.value(kDefaultUnit).toInt();

    auto tree_model = tree_model_hash_.value(section);
    if (!tree_model)
        return;

    tree_model->UpdateDefaultUnit(default_unit);
    emit SDefaultUnit(section, default_unit);
}

void WebSocket::AckUpdateDefaultUnitFailed(const QJsonObject& /*obj*/) { emit SUpdateDefaultUnitFailed(kFinance); }

void WebSocket::ApplyGlobalConfig(const QJsonArray& arr) { emit SGlobalConfig(arr); }

QHash<QUuid, QSet<QUuid>> WebSocket::ParseNodeReference(const QJsonObject& obj)
{
    QHash<QUuid, QSet<QUuid>> map {};

    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        const QUuid key = QUuid(it.key());
        QSet<QUuid>& set = map[key];

        const QJsonArray arr = it.value().toArray();
        for (const auto& val : arr) {
            set.insert(QUuid(val.toString()));
        }
    }

    return map;
}

void WebSocket::ApplyUpdateTotal(const CString& section, const QJsonArray& node_delta) const
{
    if (node_delta.isEmpty())
        return;

    auto tree_model = tree_model_hash_.value(section);
    if (!tree_model)
        return;

    for (const auto& value : node_delta) {
        const QJsonObject obj = value.toObject();
        const QUuid node_id = QUuid(obj.value(kId).toString());
        const double initial_delta = obj.value(kInitialDelta).toString().toDouble();
        const double final_delta = obj.value(kFinalDelta).toString().toDouble();

        tree_model->RSyncDelta(node_id, initial_delta, final_delta);
    }
}
