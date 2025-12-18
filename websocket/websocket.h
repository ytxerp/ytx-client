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
#include <QTimer>
#include <QWebSocket>

#include "entryhub/entryhub.h"
#include "tree/model/treemodel.h"

class WebSocket final : public QObject {
    Q_OBJECT

public:
    static WebSocket* Instance();

    void ReadConfig(QSharedPointer<QSettings> local_settings);
    void Connect();
    void SendMessage(const QString& msg_type, const QJsonObject& value);
    void Close();

    void RegisterTreeModel(Section section, QPointer<TreeModel> node) { tree_model_hash_.insert(section, node); }
    void DeregisterTreeNode(Section section) { tree_model_hash_.remove(section); }

    void RegisterEntryHub(Section section, QPointer<EntryHub> entry_hub) { entry_hub_hash_.insert(section, entry_hub); }
    void DeregisterEntryHub(Section section) { entry_hub_hash_.remove(section); }

    WebSocket(const WebSocket&) = delete;
    WebSocket& operator=(const WebSocket&) = delete;
    WebSocket(WebSocket&&) = delete;
    WebSocket& operator=(WebSocket&&) = delete;

signals:
    void SLoginResult(bool result);
    void SWorkspaceAccessPending(const QString& email, const QString& workspace);
    void SRegisterResult(int result);
    void SConnectionAccepted(bool result);
    void SRemoteHostClosed();
    void SInitializeContext(const QString& expire_date);
    void SSelectLeafEntry(const QUuid& node_id, const QUuid& entry_id);

    void SLeafRemoveDenied(const QJsonObject& obj);
    void SSharedConfig(const QJsonArray& arr);
    void SDocumentDir(Section section, const QString& document_dir);
    void SDefaultUnit(Section section, int unit);
    void SUpdateDefaultUnitFailed(const QString& section);
    void SNodeRemoveConfirmed(const QUuid& node_id);

    void SReplaceResult(bool result);
    void SSaleReference(Section section, const QUuid& widget_id, const QJsonArray& array);
    void SStatement(Section section, const QUuid& widget_id, const QJsonArray& array);
    void SStatementNodeAcked(Section section, const QUuid& widget_id, const QJsonArray& array);
    void SStatementEntryAcked(Section section, const QUuid& widget_id, const QJsonArray& array, const QJsonObject& total);
    void SSettlement(Section section, const QUuid& widget_id, const QJsonArray& array);
    void SSettlementItemAcked(Section section, const QUuid& widget_id, const QJsonArray& array);
    void SSettlementInserted(const QJsonObject& obj);
    void SSettlementRecalled(const QJsonObject& obj);
    void SSettlementUpdated(const QJsonObject& obj);

    void SConnectionRefused();

    // send to SearchNodeModel
    void SNodeSearch(const QJsonObject& obj);

private slots:
    void RConnected();
    void RDisconnected();
    void RReceiveMessage(const QString& message);
    void RErrorOccurred(QAbstractSocket::SocketError error);

private:
    explicit WebSocket(QObject* parent = nullptr);
    ~WebSocket();

    void InitHandler();
    void InitConnect();
    void InitTimer();

    QHash<QUuid, QSet<QUuid>> ParseNodeReference(const QJsonObject& obj);

private:
    void NotifyRegisterResult(const QJsonObject& obj);
    void NotifyLoginResult(const QJsonObject& obj);

    void AckTree(const QJsonObject& obj);
    void AckTable(const QJsonObject& obj);
    void AckNode(const QJsonObject& obj);
    void AckSaleReference(const QJsonObject& obj);
    void AckStatement(const QJsonObject& obj);
    void AckStatementNode(const QJsonObject& obj);
    void AckStatementEntry(const QJsonObject& obj);
    void AckSettlement(const QJsonObject& obj);
    void AckSettlementItem(const QJsonObject& obj);

    void SearchEntry(const QJsonObject& obj);
    void SearchNode(const QJsonObject& obj);

    void NotifyLeafRemoveDenied(const QJsonObject& obj);
    void NotifyUpdateDefaultUnitFailure(const QJsonObject& obj);
    void NotifyWorkspaceAccessPending(const QJsonObject& obj);

private:
    void ApplySharedConfig(const QJsonArray& arr);
    void ApplyTree(const QJsonObject& obj);
    void ApplyPartnerEntry(const QJsonArray& arr);

    void InsertNode(const QJsonObject& obj);
    void UpdateNode(const QJsonObject& obj);
    void DragNode(const QJsonObject& obj);
    void ReplaceLeaf(const QJsonObject& obj);
    void RemoveLeaf(const QJsonObject& obj);
    void RemoveLeafSafely(const QJsonObject& obj);
    void RemoveBranch(const QJsonObject& obj);
    void UpdateDirectionRule(const QJsonObject& obj);
    void UpdateNodeStatus(const QJsonObject& obj);
    void UpdateNodeName(const QJsonObject& obj);

    void UpdateOrder(const QJsonObject& obj, bool is_release);
    void InsertOrder(const QJsonObject& obj, bool is_release);
    void RecallOrder(const QJsonObject& obj);

    void InsertSettlement(const QJsonObject& obj);
    void UpdateSettlement(const QJsonObject& obj);
    void RecallSettlement(const QJsonObject& obj);
    void UpdatePartner(const QJsonObject& obj);

    void UpdateDocumentDir(const QJsonObject& obj);
    void UpdateDefaultUnit(const QJsonObject& obj);

    void UpdateEntry(const QJsonObject& obj);
    void InsertEntry(const QJsonObject& obj);
    void RemoveEntry(const QJsonObject& obj);
    void ActionEntry(const QJsonObject& obj);
    void UpdateEntryLinkedNode(const QJsonObject& obj);
    void UpdateEntryRate(const QJsonObject& obj);
    void UpdateEntryNumeric(const QJsonObject& obj);

private:
    QWebSocket socket_ {};
    QString session_id_ {};

    QUrl server_url_ {};
    bool manual_disconnect_ {};

    QTimer* heartbeat_ {};
    QDateTime last_heartbeat_time_ {};

    QHash<QString, std::function<void(const QJsonObject&)>> handler_obj_ {};
    QHash<QString, std::function<void(const QJsonArray&)>> handler_arr_ {};
    QHash<Section, QPointer<TreeModel>> tree_model_hash_ {};
    QHash<Section, QPointer<EntryHub>> entry_hub_hash_ {};
};

#endif // WEBSOCKET_H
