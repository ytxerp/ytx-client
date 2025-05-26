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

#ifndef SQL_H
#define SQL_H

#include <QObject>
#include <QSqlDatabase>

#include "component/enumclass.h"
#include "component/info.h"
#include "component/using.h"
#include "table/trans.h"
#include "tree/node.h"

class Sql : public QObject {
    Q_OBJECT

public:
    virtual ~Sql();

protected:
    Sql(QSqlDatabase& main_db, CInfo& info, QObject* parent = nullptr);

signals:
    // send to TreeModel
    void SSyncMultiLeafValue(const QList<QUuid>& node_id_list);
    void SRemoveNode(const QUuid& node_id);
    // send to Mainwindow
    void SFreeWidget(const QUuid& node_id, int node_type);
    // send to sql itsself
    void SSyncProduct(const QUuid& old_node_id, const QUuid& new_node_id);
    // send to sql itsself and treemodel
    void SSyncStakeholder(const QUuid& old_node_id, const QUuid& new_node_id);
    // send to LeafSStation, Stakeholder transactions are removed directly
    void SRemoveMultiTransL(Section section, const QMultiHash<QUuid, QUuid>& leaf_trans);
    void SMoveMultiTransL(Section section, const QUuid& old_node_id, const QUuid& new_node_id, const QSet<QUuid>& trans_id_set);
    void SAppendMultiTrans(Section section, const QUuid& node_id, const TransShadowList& trans_shadow);
    // send to SupportSStation
    void SRemoveMultiTransS(Section section, const QMultiHash<QUuid, QUuid>& support_trans);
    void SMoveMultiTransS(Section section, const QUuid& old_node_id, const QUuid& new_node_id, const QSet<QUuid>& trans_id_set);

public slots:
    // receive from remove node dialog
    virtual void RRemoveNode(const QUuid& node_id, int node_type);
    virtual void RReplaceNode(const QUuid& old_node_id, const QUuid& new_node_id, int node_type, int node_unit);
    // receive from sql
    virtual void RSyncProduct(const QUuid& old_node_id, const QUuid& new_node_id) const
    {
        Q_UNUSED(old_node_id);
        Q_UNUSED(new_node_id);
    }
    virtual void RSyncStakeholder(const QUuid& old_node_id, const QUuid& new_node_id) const
    {
        Q_UNUSED(old_node_id);
        Q_UNUSED(new_node_id);
    }

public:
    // tree
    bool ReadNode(NodeHash& node_hash);
    bool RemoveNode(const QUuid& node_id, int node_type);
    bool WriteNode(const QUuid& parent_id, Node* node);
    bool DragNode(const QUuid& destination_node_id, const QUuid& node_id);
    bool InternalReference(const QUuid& node_id) const;
    bool ExternalReference(const QUuid& node_id) const;
    bool SupportReference(const QUuid& support_id) const;
    bool ReadLeafTotal(Node* node) const;
    bool UpdateLeafValue(const Node* node) const;
    bool SearchNodeName(QSet<QUuid>& node_id_set, CString& text) const;

    // table
    bool ReadTrans(TransShadowList& trans_shadow_list, const QUuid& node_id);
    bool RetrieveTransRange(TransShadowList& trans_shadow_list, const QUuid& node_id, const QSet<QUuid>& trans_id_set);
    bool WriteTrans(TransShadow* trans_shadow);
    bool UpdateTransValue(const TransShadow* trans_shadow) const;
    TransShadow* AllocateTransShadow();

    bool WriteState(Check state) const;

    bool ReadSupportTrans(TransList& trans_list, const QUuid& support_id);
    bool RetrieveTransRange(TransList& trans_shadow_list, const QSet<QUuid>& trans_id_set);
    bool SearchTrans(TransList& trans_list, CString& text);
    bool RemoveTrans(const QUuid& trans_id);

    bool ReadTransRef(TransList& trans_list, const QUuid& node_id, int unit, const QDateTime& start, const QDateTime& end) const;

    // common
    bool WriteField(CString& table, CString& field, CVariant& value, const QUuid& id) const;

protected:
    // QS means QueryString
    // tree
    virtual QString QSReadNode() const = 0;
    virtual QString QSWriteNode() const = 0;
    virtual QString QSRemoveNodeSecond() const = 0;
    virtual QString QSInternalReference() const = 0;
    virtual QString QSSearchTransValue() const = 0;
    virtual QString QSSearchTransText() const = 0;
    virtual QString QSUpdateLeafValue() const = 0;

    virtual QString QSExternalReference() const { return {}; }
    virtual QString QSSupportReference() const { return {}; }
    virtual QString QSRemoveSupport() const { return {}; }
    virtual QString QSRemoveNodeFirst() const;

    virtual QString QSReadTransRef(int unit) const
    {
        Q_UNUSED(unit);
        return {};
    };

    virtual QString QSLeafTotal(int unit = 0) const
    {
        Q_UNUSED(unit);
        return {};
    }

    virtual void ReadNodeQuery(Node* node, const QSqlQuery& query) const = 0;
    virtual void WriteNodeBind(Node* node, QSqlQuery& query) const = 0;
    virtual void UpdateLeafValueBind(const Node* node, QSqlQuery& query) const = 0;

    //
    QString QSRemoveBranch() const;
    QString QSRemoveNodeThird() const;
    QString QSDragNodeFirst() const;
    QString QSDragNodeSecond() const;
    QString QSFreeView() const;

    //
    bool DBTransaction(std::function<bool()> function);
    bool ReadRelationship(const NodeHash& node_hash, QSqlQuery& query) const;
    bool WriteRelationship(const QUuid& node_id, const QUuid& parent_id, QSqlQuery& query) const;
    bool RemoveNode(const QUuid& node_id);

    virtual bool ReplaceLeaf(const QUuid& old_node_id, const QUuid& new_node_id, int node_unit);
    bool ReplaceSupport(const QUuid& old_node_id, const QUuid& new_node_id);
    void ReplaceLeafFunction(QSet<QUuid>& trans_id_set, const QUuid& old_node_id, const QUuid& new_node_id) const;

    // table
    virtual QString QSReadTrans() const = 0;
    virtual QString QSWriteTrans() const = 0;
    virtual QString QSTransToRemove() const = 0;

    virtual QString QSReadSupportTrans() const { return {}; }
    virtual QString QSRemoveTrans() const;
    virtual QString QSReplaceLeaf() const { return {}; }
    virtual QString QSReplaceSupport() const { return {}; }

    virtual QString QSUpdateTransValue() const { return {}; }

    virtual void ReadTransQuery(Trans* trans, const QSqlQuery& query) const = 0;
    virtual void WriteTransBind(TransShadow* trans_shadow, QSqlQuery& query) const = 0;

    virtual void UpdateTransValueBind(const TransShadow* trans_shadow, QSqlQuery& query) const
    {
        Q_UNUSED(trans_shadow);
        Q_UNUSED(query);
    }

    virtual void ReadTransRefQuery(TransList& trans_list, QSqlQuery& query) const
    {
        Q_UNUSED(trans_list);
        Q_UNUSED(query);
    };

    virtual void ReadTransFunction(TransShadowList& trans_shadow_list, const QUuid& node_id, QSqlQuery& query);
    virtual void CalculateLeafTotal(Node* node, QSqlQuery& query) const;

    //
    void ConvertTrans(Trans* trans, TransShadow* trans_shadow, bool left) const;
    void TransToRemove(QMultiHash<QUuid, QUuid>& leaf_trans, QMultiHash<QUuid, QUuid>& support_trans, const QUuid& node_id, int node_type) const;
    void RemoveSupportFunction(const QUuid& support_id) const;
    void RemoveLeafFunction(const QMultiHash<QUuid, QUuid>& leaf_trans);
    void ReplaceSupportFunction(QSet<QUuid>& trans_id_set, const QUuid& old_support_id, const QUuid& new_support_id);
    bool FreeView(const QUuid& old_node_id, const QUuid& new_node_id) const;
    void ReadTransFunction(TransList& trans_list, QSqlQuery& query);

protected:
    QHash<QUuid, Trans*> trans_hash_ {};
    Trans* last_trans_ {};
    const Section section_ {};

    QSqlDatabase& main_db_;
    CInfo& info_;
};

#endif // SQL_H
