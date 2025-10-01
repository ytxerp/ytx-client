#include "entryhub.h"

#include <QJsonArray>

#include "component/constant.h"
#include "global/entrypool.h"
#include "global/entryshadowpool.h"
#include "utils/entryutils.h"
#include "utils/jsongen.h"

EntryHub::EntryHub(CSectionInfo& info, QObject* parent)
    : QObject(parent)
    , section_ { info.section }
    , info_ { info }
{
}

EntryHub::~EntryHub() { EntryPool::Instance().Recycle(entry_cache_, section_); }

void EntryHub::RemoveLeaf(const QHash<QUuid, QSet<QUuid>>& leaf_entry)
{
    emit SRemoveEntryHash(leaf_entry);
    RemoveLeafFunction(leaf_entry);
}

void EntryHub::RemoveLeafFunction(const QHash<QUuid, QSet<QUuid>>& leaf_entry)
{
    // Recycle entry resources
    for (auto it = leaf_entry.constBegin(); it != leaf_entry.constEnd(); ++it) {
        const QSet<QUuid>& entry_set = it.value();
        for (const QUuid& entry_id : entry_set) {
            EntryPool::Instance().Recycle(entry_cache_.take(entry_id), section_);
        }
    }
}

void EntryHub::ReplaceLeaf(const QUuid& old_node_id, const QUuid& new_node_id)
{
    assert(section_ != Section::kPurchase && section_ != Section::kSale && section_ != Section::kPartner
        && "Invalid section: should not be kPurchase, kSales or kPartner");

    QSet<QUuid> entry_id_set {};
    EntryList entry_list {};

    ReplaceLeafFunction(entry_id_set, entry_list, old_node_id, new_node_id);

    emit SRemoveMultiEntry(old_node_id, entry_id_set);
    emit SAppendMultiEntry(new_node_id, entry_list);
}

void EntryHub::InsertEntry(const QJsonObject& data)
{
    auto* entry = EntryPool::Instance().Allocate(section_);
    entry->ReadJson(data);

    entry_cache_.insert(entry->id, entry);

    emit SAppendOneEntry(entry->lhs_node, entry);
    emit SAppendOneEntry(entry->rhs_node, entry);
}

void EntryHub::InsertMeta(const QUuid& entry_id, const QJsonObject& data)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* entry = it.value();

        if (data.contains(kUserId))
            entry->user_id = QUuid(data.value(kUserId).toString());
        if (data.contains(kCreatedTime))
            entry->created_time = QDateTime::fromString(data.value(kCreatedTime).toString(), Qt::ISODate);
        if (data.contains(kCreatedBy))
            entry->created_by = QUuid(data.value(kCreatedBy).toString());
    };
}

void EntryHub::RemoveEntry(const QUuid& entry_id)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* entry = it.value();

        emit SRemoveOneEntry(entry->lhs_node, entry_id);
        emit SRemoveOneEntry(entry->rhs_node, entry_id);

        EntryPool::Instance().Recycle(entry, section_);
    }
}

void EntryHub::UpdateEntry(const QUuid& id, const QJsonObject& data)
{
    auto it = entry_cache_.constFind(id);
    if (it != entry_cache_.constEnd()) {
        auto* entry = it.value();

        entry->ReadJson(data);

        const int issued_time = std::to_underlying(EntryEnum::kIssuedTime);

        const auto [start, end] = CacheColumnRange();

        emit SRefreshField(entry->lhs_node, id, start, end);
        emit SRefreshField(entry->rhs_node, id, start, end);

        emit SRefreshField(entry->lhs_node, id, issued_time, issued_time);
        emit SRefreshField(entry->rhs_node, id, issued_time, issued_time);
    };
}

void EntryHub::UpdateMeta(const QUuid& entry_id, const QJsonObject& data)
{
    auto it = entry_cache_.constFind(entry_id);
    if (it != entry_cache_.constEnd()) {
        auto* entry = it.value();

        if (data.contains(kUpdatedTime))
            entry->updated_time = QDateTime::fromString(data[kUpdatedTime].toString(), Qt::ISODate);

        if (data.contains(kUpdatedBy))
            entry->updated_by = QUuid(data[kUpdatedBy].toString());
    };
}

void EntryHub::UpdateEntryLinkedNode(const QUuid& id, const QUuid& old_rhs_id, const QUuid& new_rhs_id, const QJsonObject& data)
{
    Entry* entry {};

    auto it = entry_cache_.constFind(id);
    if (it != entry_cache_.constEnd()) {
        entry = it.value();

        const bool is_parallel = (entry->rhs_node == old_rhs_id);
        const QUuid lhs_node = is_parallel ? entry->lhs_node : entry->rhs_node;

        if (is_parallel) {
            entry->rhs_node = new_rhs_id;
        } else {
            entry->lhs_node = new_rhs_id;
        }

        entry->updated_time = QDateTime::fromString(data[kUpdatedTime].toString(), Qt::ISODate);
        entry->updated_by = QUuid(data[kUpdatedBy].toString());

        const int rhs_node_column { EntryUtils::LinkedNodeColumn(section_) };

        emit SRemoveOneEntry(old_rhs_id, id);
        emit SRefreshField(lhs_node, id, rhs_node_column, rhs_node_column);
    } else {
        entry = EntryPool::Instance().Allocate(section_);
        entry->ReadJson(data);
        entry_cache_.insert(entry->id, entry);
    }

    emit SAppendOneEntry(new_rhs_id, entry);
}

/**
 * @brief Returns SQL to remove indirect relationships in a branch (obsolete).
 *
 * This version was used when the closure table stored all distances (full closure).
 * It decreases the distance of affected ancestor-descendant pairs by 1
 * when a node and its descendants are removed.
 *
 * Deprecated: current schema only stores direct relationships (distance = 1).
 */
// QString EntryHub::QSRemoveBranch() const
// {
//     return QString(R"(
//         WITH related_nodes AS (
//             SELECT DISTINCT fp1.ancestor, fp2.descendant
//             FROM %1 AS fp1
//             INNER JOIN %1 AS fp2 ON fp1.descendant = fp2.ancestor
//             WHERE fp2.ancestor = :node_id AND fp2.descendant != :node_id AND fp1.ancestor != :node_id
//         )
//         UPDATE %1
//         SET distance = distance - 1
//         WHERE (ancestor, descendant) IN (
//             SELECT ancestor, descendant FROM related_nodes)
//     )")
//         .arg(info_.path);
// }

// QString DbHub::QSDragNodeFirst() const
// {
//     return QString(R"(
//             WITH related_nodes AS (
//                 SELECT DISTINCT fp1.ancestor, fp2.descendant
//                 FROM %1 AS fp1
//                 INNER JOIN %1 AS fp2 ON fp1.descendant = fp2.ancestor
//                 WHERE fp2.ancestor = :node_id AND fp1.ancestor != :node_id
//             )
//             DELETE FROM %1
//             WHERE (ancestor, descendant) IN (
//             SELECT ancestor, descendant FROM related_nodes)
//             )")
//         .arg(info_.path);
// }

// QString DbHub::QSDragNodeSecond() const
// {
//     return QString(R"(
//             INSERT INTO %1 (ancestor, descendant, distance)
//             SELECT fp1.ancestor, fp2.descendant, fp1.distance + fp2.distance + 1
//             FROM %1 AS fp1
//             INNER JOIN %1 AS fp2
//             WHERE fp1.descendant = :destination_node_id AND fp2.ancestor = :node_id
//             )")
//         .arg(info_.path);
// }

// bool EntryHub::WriteRelationship(const QUuid& node_id, const QUuid& parent_id, QSqlQuery& query, qlonglong version) const
// {
//     auto part_old = QString(R"(
//     INSERT INTO %1 (ancestor, descendant, distance)
//     SELECT ancestor, :node_id, distance + 1 FROM %1
//     WHERE descendant = :parent
//     UNION ALL
//     SELECT :node_id, :node_id, 0
//     )")
//                         .arg(info_.path);
// }

bool EntryHub::ReadTransRef(EntryRefList& list, const QUuid& node_id, int unit, const QDateTime& start, const QDateTime& end) const
{
    // QSqlQuery query(main_db_);
    // query.setForwardOnly(true);

    // auto sql { QSReadTransRef(unit) };
    // if (sql.isEmpty())
    //     return false;

    // query.prepare(sql);
    // query.bindValue(QStringLiteral(":node_id"), node_id.toString(QUuid::WithoutBraces));
    // query.bindValue(QStringLiteral(":start"), start.toString(Qt::ISODate));
    // query.bindValue(QStringLiteral(":end"), end.toString(Qt::ISODate));

    // if (!query.exec()) {
    //     qWarning() << "Failed in TransRefRecord" << query.lastError().text();
    //     return false;
    // }

    // ReadTransRefQuery(list, query);

    return true;
}

EntryShadow* EntryHub::AllocateEntryShadow()
{
    auto* entry = EntryPool::Instance().Allocate(section_);
    entry->id = QUuid::createUuidV7();

    auto* entry_shadow { EntryShadowPool::Instance().Allocate(section_) };
    entry_shadow->BindEntry(entry, true);

    entry_cache_.insert(entry->id, entry);
    return entry_shadow;
}

// QString EntryHub::BuildSelect(const QString& table, QStringList& condition) const
// {
//     static const auto kWhere { QStringLiteral(" WHERE ") };
//     static const auto kAnd { QStringLiteral(" AND ") };

//     const QString sql = QString("SELECT * FROM %1").arg(table);

//     if (condition.isEmpty()) {
//         return sql;
//     }

//     for (QString& cond : condition) {
//         cond = cond.simplified();
//         if (!cond.isEmpty()) {
//             cond = QStringLiteral("(") + cond + QStringLiteral(")");
//         }
//     }

//     return sql + kWhere + condition.join(kAnd);
// }

void EntryHub::AckLeafTable(const QUuid& node_id, const QJsonArray& array)
{
    const EntryList entry_list = ProcessEntryArray(array);
    emit SAppendMultiEntry(node_id, entry_list);
}

void EntryHub::SearchEntry(const QJsonArray& array)
{
    EntryList list = ProcessEntryArray(array);
    emit SSearchEntry(list);
}

EntryList EntryHub::ProcessEntryArray(const QJsonArray& array)
{
    EntryList list {};

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };
        const QUuid id = QUuid(obj.value(kId).toString());

        Entry* entry {};

        if (auto it = entry_cache_.constFind(id); it != entry_cache_.constEnd()) {
            entry = it.value();
        } else {
            entry = EntryPool::Instance().Allocate(section_);
            entry->ReadJson(obj);
            entry_cache_.insert(id, entry);
        }

        list.emplaceBack(entry);
    }

    return list;
}

void EntryHub::ActionEntry(const QUuid& node_id, EntryAction action, const QJsonObject& meta)
{
    auto Update = [action](Entry* entry) {
        switch (action) {
        case EntryAction::kMarkAll:
            entry->mark_status = true;
            break;
        case EntryAction::kMarkNone:
            entry->mark_status = false;
            break;
        case EntryAction::kMarkToggle:
            entry->mark_status = !entry->mark_status;
            break;
        default:
            break;
        }
    };

    for (auto* entry : std::as_const(entry_cache_)) {
        if (entry->lhs_node == node_id || entry->rhs_node == node_id) {
            Update(entry);
            entry->updated_by = QUuid(meta.value(kUpdatedBy).toString());
            entry->updated_time = QDateTime::fromString(meta.value(kUpdatedTime).toString(), Qt::ISODate);
        }
    }

    emit SRefreshMarkStatus(node_id);
}

void EntryHub::ActionEntryMeta(const QUuid& node_id, const QJsonObject& meta)
{
    for (auto* entry : std::as_const(entry_cache_)) {
        if (entry->lhs_node == node_id || entry->rhs_node == node_id) {
            entry->updated_by = QUuid(meta.value(kUpdatedBy).toString());
            entry->updated_time = QDateTime::fromString(meta.value(kUpdatedTime).toString(), Qt::ISODate);
        }
    }
}

void EntryHub::ReplaceLeafFunction(QSet<QUuid>& entry_id_set, EntryList& entry_list, const QUuid& old_node_id, const QUuid& new_node_id) const
{
    for (auto* entry : std::as_const(entry_cache_)) {
        if (entry->lhs_node == old_node_id && entry->rhs_node != new_node_id) {
            entry_id_set.insert(entry->id);
            entry_list.emplaceBack(entry);
            entry->lhs_node = new_node_id;
        }

        if (entry->rhs_node == old_node_id && entry->lhs_node != new_node_id) {
            entry_id_set.insert(entry->id);
            entry_list.emplaceBack(entry);
            entry->rhs_node = new_node_id;
        }
    }
}
