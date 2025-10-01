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
    void Clear();

    void RegisterTreeModel(const QString& section, QPointer<TreeModel> node) { tree_model_hash_.insert(section, node); }
    void UnregisterTreeNode(const QString& section) { tree_model_hash_.remove(section); }

    void RegisterEntryHub(const QString& section, QPointer<EntryHub> entry_hub) { entry_hub_hash_.insert(section, entry_hub); }
    void UnregisterEntryHub(const QString& section) { entry_hub_hash_.remove(section); }

    WebSocket(const WebSocket&) = delete;
    WebSocket& operator=(const WebSocket&) = delete;
    WebSocket(WebSocket&&) = delete;
    WebSocket& operator=(WebSocket&&) = delete;

signals:
    void SActionLoginTriggered();
    void SLoginResult(bool result);
    void SWorkspaceAccessPending(const QString& email, const QString& workspace);
    void SRegisterResult(int result);
    void SConnectResult(bool result);
    void SInitializeContext(const QString& expire_date);
    void SCreateUserResult(bool result);
    void SCreateDatabaseResult(bool result);
    void SInitializeDatabaseResult(bool result);
    void SScrollToEntry(const QUuid& node_id, const QUuid& entry_id);

    void SLeafRemoveCheck(const QJsonObject& obj);
    void SGlobalConfig(const QJsonArray& arr);
    void SDocumentDir(const QString& section, const QString& document_dir);
    void SDefaultUnit(const QString& section, int unit);
    void SUpdateDefaultUnitFailed(const QString& section);

    void SReplaceResult(bool result);

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
    void ApplyUpdateTotal(const CString& section, const QJsonArray& node_delta) const;

    // ----------------------------
    // Naming convention:
    // Private messages (server responds to a client request individually) → Ack
    // Broadcast messages (server pushes to all clients) → Apply
    // ----------------------------

private:
    void AckLoginFailed(const QJsonObject& obj);
    void AckLoginSuccess(const QJsonObject& obj);
    void AckRegisterResult(const QJsonObject& obj);

    void AckTree(const QJsonObject& obj);
    void AckLeaf(const QJsonObject& obj);
    void AckNode(const QJsonObject& obj);

    void AckEntrySearch(const QJsonObject& obj);
    void AckLeafRemoveCheck(const QJsonObject& obj);
    void AckUpdateDefaultUnitFailed(const QJsonObject& obj);

    void AckWorkspaceAccessPending(const QJsonObject& obj);

private:
    void GlobalConfig(const QJsonArray& arr);
    void ApplyTree(const QJsonObject& obj);

    void InsertNode(const QJsonObject& obj);
    void ApplyNodeUpdate(const QJsonObject& obj);
    void DragNode(const QJsonObject& obj);
    void ApplyLeafRemove(const QJsonObject& obj);
    void ApplyLeafRemoveSafely(const QJsonObject& obj);
    void ApplyBranchRemove(const QJsonObject& obj);

    void ApplyLeafReplace(const QJsonObject& obj);
    void ApplyEntryUpdate(const QJsonObject& obj);
    void ApplyEntryInsert(const QJsonObject& obj);
    void ApplyEntryRemove(const QJsonObject& obj);
    void ApplyDirectionRule(const QJsonObject& obj);
    void ApplyName(const QJsonObject& obj);
    void ActionEntry(const QJsonObject& obj);
    void ApplyDocumentDir(const QJsonObject& obj);
    void ApplyDefaultUnit(const QJsonObject& obj);
    void ApplyEntryRhsNode(const QJsonObject& obj);
    void ApplyEntryRate(const QJsonObject& obj);
    void ApplyEntryNumeric(const QJsonObject& obj);

private:
    QWebSocket socket_ {};
    QString session_id_ {};

    QUrl server_url_ {};

    QTimer* ping_timer_ {};

    QHash<QString, std::function<void(const QJsonObject&)>> handler_obj_ {};
    QHash<QString, std::function<void(const QJsonArray&)>> handler_arr_ {};
    QHash<QString, QPointer<TreeModel>> tree_model_hash_ {};
    QHash<QString, QPointer<EntryHub>> entry_hub_hash_ {};
};

#endif // WEBSOCKET_H
