#include "global/tablesstation.h"

#include <QCoreApplication>

#include "table/model/tablemodeldouble.h"

TableSStation* TableSStation::Instance()
{
    static auto* instance = new TableSStation(qApp);
    return instance;
}

TableSStation::TableSStation(QObject* parent)
    : QObject(parent)
{
}

void TableSStation::RegisterModel(QUuid node_id, const TableModel* model)
{
    Q_ASSERT(!node_id.isNull());
    Q_ASSERT(model);

    model_hash_.insert(node_id, model);
}

void TableSStation::DeregisterModel(QUuid node_id)
{
    Q_ASSERT(!node_id.isNull());
    model_hash_.remove(node_id);
}

void TableSStation::RAttachEntry(QUuid node_id, Entry* entry)
{
    Q_ASSERT(!node_id.isNull());

    const auto* model { FindModel(node_id) };

    if (!model)
        return;

    connect(this, &TableSStation::SAttachEntry, model, &TableModel::RAttachEntry, Qt::SingleShotConnection);
    emit SAttachEntry(entry);
}

void TableSStation::RDetachEntry(QUuid node_id, QUuid entry_id, QUuid counter_node_id)
{
    Q_ASSERT(!node_id.isNull());
    Q_ASSERT(!entry_id.isNull());

    const auto* model { FindModel(node_id) };
    if (!model)
        return;

    connect(this, &TableSStation::SDetachEntry, model, &TableModel::RDetachEntry, Qt::SingleShotConnection);
    emit SDetachEntry(entry_id, counter_node_id);
}

void TableSStation::RUpdateBalance(QUuid node_id, QUuid entry_id)
{
    Q_ASSERT(!node_id.isNull());
    Q_ASSERT(!entry_id.isNull());

    const auto* base { FindModel(node_id) };
    if (!base)
        return;

    Q_ASSERT(qobject_cast<const TableModelDouble*>(base));
    const auto* model { static_cast<const TableModelDouble*>(base) };

    connect(this, &TableSStation::SUpdateBalance, model, &TableModelDouble::RUpdateBalance, Qt::SingleShotConnection);
    emit SUpdateBalance(entry_id);
}

void TableSStation::RAppendEntries(QUuid node_id, const EntryList& entry_list)
{
    Q_ASSERT(!node_id.isNull());

    if (entry_list.isEmpty())
        return;

    const auto* model { FindModel(node_id) };
    if (!model)
        return;

    connect(this, &TableSStation::SAppendEntries, model, &TableModel::RAppendEntries, Qt::SingleShotConnection);
    emit SAppendEntries(entry_list);
}

void TableSStation::RRefreshBatchEntries(const QSet<QUuid>& affected_node, int column)
{
    for (const auto& node_id : affected_node) {
        Q_ASSERT(!node_id.isNull());

        const auto* model { FindModel(node_id) };
        if (!model)
            continue;

        connect(this, &TableSStation::SRefreshBatchEntries, model, &TableModel::RRefreshBatchEntries, Qt::SingleShotConnection);
        emit SRefreshBatchEntries(column);
    }
}

void TableSStation::RRefreshField(QUuid node_id, QUuid entry_id, int start, int end)
{
    Q_ASSERT(!node_id.isNull());
    Q_ASSERT(!entry_id.isNull());

    const auto* model { FindModel(node_id) };
    if (!model)
        return;

    connect(this, &TableSStation::SRefreshField, model, &TableModel::RRefreshField, Qt::SingleShotConnection);
    emit SRefreshField(entry_id, start, end);
}

void TableSStation::RDirectionRule(QUuid node_id, bool rule)
{
    Q_ASSERT(!node_id.isNull());

    const auto* base { FindModel(node_id) };
    if (!base)
        return;

    Q_ASSERT(qobject_cast<const TableModelDouble*>(base));
    const auto* model { static_cast<const TableModelDouble*>(base) };

    connect(this, &TableSStation::SDirectionRule, model, &TableModelDouble::RDirectionRule, Qt::SingleShotConnection);
    emit SDirectionRule(rule);
}

void TableSStation::RDeleteEntryHash(const QHash<QUuid, QSet<QUuid>>& entry_hash)
{
    for (auto it = entry_hash.constBegin(); it != entry_hash.constEnd(); ++it) {
        QUuid node_id = it.key();
        Q_ASSERT(!node_id.isNull());

        const QSet<QUuid>& entry_id_set = it.value();

        const auto* base { FindModel(node_id) };
        if (!base)
            return;

        Q_ASSERT(qobject_cast<const TableModelDouble*>(base));
        const auto* model { static_cast<const TableModelDouble*>(base) };

        connect(this, &TableSStation::SDeleteEntries, model, &TableModelDouble::RDeleteEntries, Qt::SingleShotConnection);
        emit SDeleteEntries(entry_id_set);
    }
}

void TableSStation::RDeleteEntries(QUuid node_id, const QSet<QUuid>& entry_id_set)
{
    Q_ASSERT(!node_id.isNull());

    const auto* base { FindModel(node_id) };
    if (!base)
        return;

    Q_ASSERT(qobject_cast<const TableModelDouble*>(base));
    const auto* model { static_cast<const TableModelDouble*>(base) };

    connect(this, &TableSStation::SDeleteEntries, model, &TableModelDouble::RDeleteEntries, Qt::SingleShotConnection);
    emit SDeleteEntries(entry_id_set);
}
