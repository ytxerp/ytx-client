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

#ifndef ENTRYHUBO_H
#define ENTRYHUBO_H

#include "billing/statement/statement.h"
#include "entryhub.h"
#include "report/settlement.h"

class EntryHubO final : public EntryHub {
    Q_OBJECT

public:
    EntryHubO(CSectionInfo& info, QObject* parent = nullptr);

public:
    void RemoveLeaf(const QHash<QUuid, QSet<QUuid>>& leaf_entry) override;

    bool ReadSettlement(SettlementList& list, const QDateTime& start, const QDateTime& end) const;
    bool WriteSettlement(const Settlement* settlement) const;
    bool RemoveSettlement(const QUuid& settlement_id);

    bool ReadSettlementPrimary(SettlementList& list, const QUuid& partner_id, const QUuid& settlement_id, bool status);
    bool AddSettlementPrimary(const QUuid& node_id, const QUuid& settlement_id) const;
    bool RemoveSettlementPrimary(const QUuid& node_id) const;

    bool ReadStatement(StatementList& list, int unit, const QDateTime& start, const QDateTime& end) const;
    bool ReadBalance(double& pbalance, double& cdelta, const QUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end) const;
    bool ReadStatementPrimary(StatementPrimaryList& list, const QUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end) const;
    bool ReadStatementSecondary(StatementSecondaryList& list, const QUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end) const;

protected:
    EntryList ProcessEntryArray(const QJsonArray& array) override;

private:
    QString QSReadSettlement() const;
    // void ReadSettlementQuery(SettlementList& settlement_list, QSqlQuery& query) const;
    QString QSWriteSettlement() const;

    QString QSRemoveSettlementFirst() const;
    QString QSRemoveSettlementSecond() const;

    QString QSReadSettlementPrimary(bool status) const;

    // void ReadSettlementPrimaryQuery(SettlementList& node_list, QSqlQuery& query);

    QString QSInvertTransValue() const;

    QString QSReadStatement(int unit) const;
    QString QSReadBalance(int unit) const;
    QString QSReadStatementPrimary(int unit) const;
    QString QSReadStatementSecondary(int unit) const;

    // void ReadStatementQuery(StatementList& list, QSqlQuery& query) const;
    // void ReadStatementPrimaryQuery(StatementPrimaryList& list, QSqlQuery& query) const;
    // void ReadStatementSecondaryQuery(StatementSecondaryList& list, QSqlQuery& query) const;
};

#endif // ENTRYHUBO_H
