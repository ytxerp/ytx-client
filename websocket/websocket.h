/*
 * Copyright (C) 2023 YTX
 *
 * This file is part of YTX.
 *
 * YTX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * YTX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YTX. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QWebSocket>

#include "component/constantwebsocket.h"
#include "entryhub/entryhub.h"
#include "tree/model/treemodel.h"

class WebSocket final : public QObject {
    Q_OBJECT

public:
    static WebSocket* Instance();

    void ReadConfig(const QSharedPointer<QSettings>& local_settings);
    void Connect();
    void SendMessage(WsKey key, const QJsonObject& value);
    void Reset();

    void RegisterTreeModel(Section section, QPointer<TreeModel> node) { tree_model_hash_.insert(section, node); }
    void RegisterEntryHub(Section section, QPointer<EntryHub> entry_hub) { entry_hub_hash_.insert(section, entry_hub); }

    WebSocket(const WebSocket&) = delete;
    WebSocket& operator=(const WebSocket&) = delete;
    WebSocket(WebSocket&&) = delete;
    WebSocket& operator=(WebSocket&&) = delete;

signals:
    void SConnectionAllow();
    void SConnectionDeny();

    void SLoginAllow(const QString& name, const QString& expire_date);
    void SLoginDeny(int code);

    void SRegisterResult(bool result, int code);
    void SRemoteHostClosed();
    void SEntrySelect(const QUuid& node_id, const QUuid& entry_id);

    void SLeafDeleteDeny(const QJsonObject& obj);
    void SSharedConfigApply(const QJsonArray& arr);
    void SDocumentDirUpdate(Section section, const QString& document_dir);
    void SDefaultUnitUpdate(Section section, int unit);
    void SDefaultUnitDeny(const QString& section);

    void STagApply(const QJsonObject& obj);
    void STagInsert(const QJsonObject& obj, bool is_same_session);
    void STagUpdate(const QJsonObject& obj);
    void STagDelete(const QJsonObject& obj);

    void SReplaceResult(bool result);
    void SOrderReferenceAck(Section section, const QUuid& widget_id, const QJsonArray& array);
    void SStatementAck(Section section, const QUuid& widget_id, const QJsonArray& array);
    void SStatementNodeAck(Section section, const QUuid& widget_id, const QJsonArray& array);
    void SStatementEntryAck(Section section, const QUuid& widget_id, const QJsonArray& array, const QJsonObject& total);
    void SSettlementAck(Section section, const QUuid& widget_id, const QJsonArray& array);
    void SSettlementItemAck(Section section, const QUuid& widget_id, const QJsonArray& array);
    void SSettlementInsert(const QJsonObject& obj);
    void SSettlementRecall(const QJsonObject& obj);
    void SSettlementUpdate(const QJsonObject& obj);
    void SOrderRelease(Section section, const QUuid& node_id, int version);
    void SOrderRecall(Section section, const QUuid& node_id, int version);
    void SOrderSave(Section section, const QUuid& node_id, int version);
    void SOperationDeny();
    void SNodeSelect(Section section, const QUuid& node_id);
    void SNodeLocate(Section section, const QUuid& node_id);
    void STreeSyncFinish();
    void SAccountName(const QString& name);
    void SAccountUsername(const QJsonObject& obj);
    void SWorkspaceMemberAck(const QUuid& widget_id, const QJsonArray& array);
    void SAuditLogAck(const QUuid& widget_id, const QJsonArray& array, const QJsonArray& user_array);
    void SAccountRoleUpdate();

    // send to SearchNodeModel
    void SNodeSearch(const QJsonObject& obj);

private slots:
    void RConnected();
    void RDisconnected();
    void RBinaryMessageReceived(const QByteArray& data);
    void RErrorOccurred(QAbstractSocket::SocketError error);
    void RPong()
    {
        qDebug() << "← Pong received, reset timeout_timer_";
        timeout_timer_->start(TimeConst::kTimeoutThresholdMs);
    }

    void RSendPing()
    {
        if (socket_.state() == QAbstractSocket::ConnectedState) {
            qDebug() << "→ Ping sent";
            socket_.ping();
        }
    }

    void RTimeout()
    {
        qWarning() << "Heartbeat timeout, closing connection";
        socket_.close(QWebSocketProtocol::CloseCodeGoingAway, "Heartbeat timeout");
        ping_timer_->stop();
    }

private:
    explicit WebSocket(QObject* parent = nullptr);
    ~WebSocket() override;

    void InitHandler();
    void InitConnect();
    void InitTimer();

    void HandleMessage(WsKey key, const QByteArray& payload);

    void StopTimer()
    {
        if (ping_timer_)
            ping_timer_->stop();

        if (timeout_timer_)
            timeout_timer_->stop();
    }

    QHash<QUuid, QSet<QUuid>> ParseLinkedEntry(const QJsonObject& obj) const;
    QSet<QUuid> ParseLinkedEntryP(const QJsonArray& arr) const;

private:
    void NotifyRegister(const QJsonObject& obj);
    void NotifyLogin(const QJsonObject& obj);

    void AckTree(const QJsonObject& obj);
    void AckTable(const QJsonObject& obj);
    void AckNode(const QJsonObject& obj);
    void AckOrderReference(const QJsonObject& obj);
    void AckStatement(const QJsonObject& obj);
    void AckStatementNode(const QJsonObject& obj);
    void AckStatementEntry(const QJsonObject& obj);
    void AckSettlement(const QJsonObject& obj);
    void AckSettlementItem(const QJsonObject& obj);
    void AckWorkspaceMember(const QJsonObject& obj);
    void AckAuditLog(const QJsonObject& obj);

    void SearchEntry(const QJsonObject& obj);
    void SearchNode(const QJsonObject& obj);

    void DenyLeafDelete(const QJsonObject& obj);
    void DenyDefaultUnit(const QJsonObject& obj);
    void DenyOperation();
    void FinishTreeSync();

private:
    void ApplySharedConfig(const QJsonArray& arr);
    void ApplyTree(const QJsonObject& obj);
    void ApplyTag(const QJsonObject& obj);
    void ApplyPartnerEntry(const QJsonArray& arr);

    void InsertNode(const QJsonObject& obj);
    void UpdateNode(const QJsonObject& obj);
    void DragNode(const QJsonObject& obj);
    void ReplaceLeaf(const QJsonObject& obj);
    void DeleteLeaf(const QJsonObject& obj);
    void DeleteLeafO(const QJsonObject& obj);
    void DeleteLeafP(const QJsonObject& obj);
    void AllowLeafDelete(const QJsonObject& obj);
    void DeleteBranch(const QJsonObject& obj);
    void UpdateNodeDirectionRule(const QJsonObject& obj);
    void UpdateNodeName(const QJsonObject& obj);
    void UpdateAccountName(const QJsonObject& obj);
    void UpdateAccountUsername(const QJsonObject& obj);
    void UpdateAccountRole(const QJsonObject& obj);

    void InsertTag(const QJsonObject& obj);
    void UpdateTag(const QJsonObject& obj);
    void DeleteTag(const QJsonObject& obj);

    void UpdateOrder(const QJsonObject& obj, bool is_released);
    void InsertOrder(const QJsonObject& obj, bool is_released);
    void RecallOrder(const QJsonObject& obj);

    void InsertSettlement(const QJsonObject& obj);
    void UpdateSettlement(const QJsonObject& obj);
    void RecallSettlement(const QJsonObject& obj);

    void UpdateDocumentDir(const QJsonObject& obj);
    void UpdateDefaultUnit(const QJsonObject& obj);

    void UpdateEntry(const QJsonObject& obj);
    void InsertEntry(const QJsonObject& obj);
    void DeleteEntry(const QJsonObject& obj);
    void MarkBatch(const QJsonObject& obj);
    void UpdateEntryLinkedNode(const QJsonObject& obj);
    void UpdateEntryRate(const QJsonObject& obj);
    void UpdateEntryNumeric(const QJsonObject& obj);
    void UpdateEntryIssuedTime(const QJsonObject& obj);

private:
    QWebSocket socket_ {};
    QUuid session_id_ {};

    QUrl server_url_ {};
    bool manual_disconnect_ {};

    QTimer* ping_timer_ {};
    QTimer* timeout_timer_ {};

    QHash<WsKey, std::function<void(const QJsonObject&)>> handler_obj_ {};
    QHash<WsKey, std::function<void(const QJsonArray&)>> handler_arr_ {};
    QHash<Section, QPointer<TreeModel>> tree_model_hash_ {};
    QHash<Section, QPointer<EntryHub>> entry_hub_hash_ {};
};

#endif // WEBSOCKET_H
