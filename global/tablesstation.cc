#include "global/tablesstation.h"

#include <QCoreApplication>

TableSStation* TableSStation::Instance()
{
    static auto* instance = new TableSStation(qApp);
    return instance;
}

TableSStation::TableSStation(QObject* parent)
    : QObject(parent)
{
}

void TableSStation::RegisterModel(const QUuid& node_id, const TableModel* model)
{
    Q_ASSERT(!node_id.isNull());
    Q_ASSERT(model);

    model_hash_.insert(node_id, model);
}

void TableSStation::DeregisterModel(const QUuid& node_id)
{
    Q_ASSERT(!node_id.isNull());
    model_hash_.remove(node_id);
}

void TableSStation::RAttachOneEntry(const QUuid& node_id, Entry* entry)
{
    Q_ASSERT(!node_id.isNull());

    const auto* model { FindModel(node_id) };

    if (!model)
        return;

    connect(this, &TableSStation::SAttachOneEntry, model, &TableModel::RAttachOneEntry, Qt::SingleShotConnection);
    emit SAttachOneEntry(entry);
}

void TableSStation::RDetachOneEntry(const QUuid& node_id, const QUuid& entry_id)
{
    Q_ASSERT(!node_id.isNull());
    Q_ASSERT(!entry_id.isNull());

    const auto* model { FindModel(node_id) };
    if (!model)
        return;

    connect(this, &TableSStation::SDetachOneEntry, model, &TableModel::RDetachOneEntry, Qt::SingleShotConnection);
    emit SDetachOneEntry(entry_id);
}

void TableSStation::RUpdateBalance(const QUuid& node_id, const QUuid& entry_id)
{
    Q_ASSERT(!node_id.isNull());
    Q_ASSERT(!entry_id.isNull());

    const auto* model { FindModel(node_id) };
    if (!model)
        return;

    connect(this, &TableSStation::SUpdateBalance, model, &TableModel::RUpdateBalance, Qt::SingleShotConnection);
    emit SUpdateBalance(entry_id);
}

void TableSStation::RAppendMultiEntries(const QUuid& node_id, const EntryList& entry_list)
{
    Q_ASSERT(!node_id.isNull());

    if (entry_list.isEmpty())
        return;

    const auto* model { FindModel(node_id) };
    if (!model)
        return;

    connect(this, &TableSStation::SAppendMultiEntries, model, &TableModel::RAppendMultiEntries, Qt::SingleShotConnection);
    emit SAppendMultiEntries(entry_list);
}

void TableSStation::RRefreshStatus(const QSet<QUuid>& affected_node)
{
    for (const auto& node_id : affected_node) {
        Q_ASSERT(!node_id.isNull());

        const auto* model { FindModel(node_id) };
        if (!model)
            continue;

        connect(this, &TableSStation::SRefreshStatus, model, &TableModel::RRefreshStatus, Qt::SingleShotConnection);
        emit SRefreshStatus();
    }
}

void TableSStation::RRefreshField(const QUuid& node_id, const QUuid& entry_id, int start, int end)
{
    Q_ASSERT(!node_id.isNull());
    Q_ASSERT(!entry_id.isNull());

    const auto* model { FindModel(node_id) };
    if (!model)
        return;

    connect(this, &TableSStation::SRefreshField, model, &TableModel::RRefreshField, Qt::SingleShotConnection);
    emit SRefreshField(entry_id, start, end);
}

void TableSStation::RDirectionRule(const QUuid& node_id, bool rule)
{
    Q_ASSERT(!node_id.isNull());

    const auto* model { FindModel(node_id) };
    if (!model)
        return;

    connect(this, &TableSStation::SDirectionRule, model, &TableModel::RDirectionRule, Qt::SingleShotConnection);
    emit SDirectionRule(rule);
}

void TableSStation::RDeleteEntryHash(const QHash<QUuid, QSet<QUuid>>& entry_hash)
{
    for (auto it = entry_hash.constBegin(); it != entry_hash.constEnd(); ++it) {
        const QUuid& node_id = it.key();
        Q_ASSERT(!node_id.isNull());

        const QSet<QUuid>& entry_id_set = it.value();

        const auto* model = FindModel(node_id);
        if (!model)
            continue;

        connect(this, &TableSStation::SDeleteMultiEntries, model, &TableModel::RDeleteMultiEntries, Qt::SingleShotConnection);
        emit SDeleteMultiEntries(entry_id_set);
    }
}

void TableSStation::RDeleteMultiEntries(const QUuid& node_id, const QSet<QUuid>& entry_id_set)
{
    Q_ASSERT(!node_id.isNull());

    const auto* old_model { FindModel(node_id) };
    if (old_model) {
        connect(this, &TableSStation::SDeleteMultiEntries, old_model, &TableModel::RDeleteMultiEntries, Qt::SingleShotConnection);
        emit SDeleteMultiEntries(entry_id_set);
    }
}
