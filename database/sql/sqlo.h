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

#ifndef SQLO_H
#define SQLO_H

#include "database/sql/prices.h"
#include "sql.h"

class SqlO final : public Sql {
    Q_OBJECT

public:
    SqlO(CInfo& info, QObject* parent = nullptr);
    ~SqlO();

signals:
    // send to sqlite stakeholder
    void SSyncPrice(const QList<PriceS>& list);

public slots:
    void RRemoveNode(const QUuid& node_id, int node_type) override;

    void RSyncProduct(const QUuid& old_node_id, const QUuid& new_node_id) const override;
    void RSyncStakeholder(const QUuid& old_node_id, const QUuid& new_node_id) const override;

public:
    bool ReadNode(NodeHash& node_hash, const QDateTime& start, const QDateTime& end);
    bool SearchNode(QList<const Node*>& node_list, const QList<QUuid>& party_id_list);
    Node* ReadNode(QUuid node_id);

    bool SettlementReference(const QUuid& settlement_id) const;
    int SettlementID(const QUuid& node_id) const;

    bool ReadSettlement(NodeList& node_list, const QDateTime& start, const QDateTime& end) const;
    bool WriteSettlement(Node* node) const;
    bool RemoveSettlement(const QUuid& settlement_id);

    bool ReadSettlementPrimary(NodeList& node_list, const QUuid& party_id, const QUuid& settlement_id, bool is_finished);
    bool AddSettlementPrimary(const QUuid& node_id, const QUuid& settlement_id) const;
    bool RemoveSettlementPrimary(const QUuid& node_id) const;

    bool SyncPrice(const QUuid& node_id);
    bool InvertTransValue(const QUuid& node_id) const;

    bool ReadStatement(TransList& trans_list, int unit, const QDateTime& start, const QDateTime& end) const;
    bool ReadBalance(double& pbalance, double& cdelta, const QUuid& party_id, int unit, const QDateTime& start, const QDateTime& end) const;
    bool ReadStatementPrimary(NodeList& node_list, const QUuid& party_id, int unit, const QDateTime& start, const QDateTime& end) const;
    bool ReadStatementSecondary(TransList& trans_list, const QUuid& party_id, int unit, const QDateTime& start, const QDateTime& end) const;

    bool WriteTransRange(const QList<TransShadow*>& list);

protected:
    // tree
    void ReadNodeQuery(Node* node, const QSqlQuery& query) const override;
    void WriteNodeBind(Node* node, QSqlQuery& query) const override;

    QString QSReadNode() const override;
    QString QSWriteNode() const override;
    QString QSRemoveNodeSecond() const override;
    QString QSInternalReference() const override;
    QString QSExternalReference() const override;

    // table
    void WriteTransBind(TransShadow* trans_shadow, QSqlQuery& query) const override;
    void ReadTransFunction(TransShadowList& trans_shadow_list, const QUuid& node_id, QSqlQuery& query) override;
    void ReadTransQuery(Trans* trans, const QSqlQuery& query) const override;
    void UpdateTransValueBind(const TransShadow* trans_shadow, QSqlQuery& query) const override;

    QString QSUpdateLeafValue() const override;
    void UpdateLeafValueBind(const Node* node, QSqlQuery& query) const override;

    QString QSReadTrans() const override;
    QString QSWriteTrans() const override;
    QString QSSearchTransValue() const override;
    QString QSSearchTransText() const override;
    QString QSUpdateTransValue() const override;
    QString QSTransToRemove() const override;

private:
    QString QSSearchNode(CString& in_list) const;

    void ReadNodeFunction(NodeHash& node_hash, QSqlQuery& query);
    void SearchNodeFunction(QList<const Node*>& node_list, QSqlQuery& query);

    QString QSReadSettlement() const;
    void ReadSettlementQuery(NodeList& node_list, QSqlQuery& query) const;
    QString QSWriteSettlement() const;
    void WriteSettlementBind(Node* node, QSqlQuery& query) const;

    QString QSRemoveSettlementFirst() const;
    QString QSRemoveSettlementSecond() const;

    QString QSReadSettlementPrimary(bool is_finished) const;

    void ReadSettlementPrimaryQuery(NodeList& node_list, QSqlQuery& query);

    QString QSSyncPriceFirst() const;
    QString QSSyncPriceSecond() const;
    QString QSInvertTransValue() const;

    QString QSReadStatement(int unit) const;
    QString QSReadBalance(int unit) const;
    QString QSReadStatementPrimary(int unit) const;
    QString QSReadStatementSecondary(int unit) const;

    void ReadStatementQuery(TransList& trans_list, QSqlQuery& query) const;
    void ReadStatementPrimaryQuery(NodeList& node_list, QSqlQuery& query) const;
    void ReadStatementSecondaryQuery(TransList& trans_list, QSqlQuery& query) const;

    void WriteTransRangeFunction(const QList<TransShadow*>& list, QSqlQuery& query) const;

private:
    NodeHash node_hash_ {};
};

#endif // SQLO_H
