#include "global/leafsstation.h"

#include <QCoreApplication>

LeafSStation* LeafSStation::Instance()
{
    static LeafSStation* instance = new LeafSStation(qApp);
    Q_ASSERT(instance != nullptr);
    return instance;
}

LeafSStation::LeafSStation(QObject* parent)
    : QObject(parent)
{
}

void LeafSStation::RegisterModel(const QUuid& node_id, const LeafModel* model) { model_hash_.insert(node_id, model); }

void LeafSStation::DeregisterModel(const QUuid& node_id) { model_hash_.remove(node_id); }

void LeafSStation::RAppendOneEntry(const QUuid& node_id, Entry* entry)
{
    const auto* model { FindModel(node_id) };

    if (!model)
        return;

    connect(this, &LeafSStation::SAppendOneEntry, model, &LeafModel::RAppendOneEntry, Qt::SingleShotConnection);
    emit SAppendOneEntry(entry);
}

void LeafSStation::RRemoveOneEntry(const QUuid& node_id, const QUuid& entry_id)
{
    const auto* model { FindModel(node_id) };
    if (!model)
        return;

    connect(this, &LeafSStation::SRemoveOneEntry, model, &LeafModel::RRemoveOneEntry, Qt::SingleShotConnection);
    emit SRemoveOneEntry(entry_id);
}

void LeafSStation::RUpdateBalance(const QUuid& node_id, const QUuid& entry_id)
{
    const auto* model { FindModel(node_id) };
    if (!model)
        return;

    connect(this, &LeafSStation::SUpdateBalance, model, &LeafModel::RUpdateBalance, Qt::SingleShotConnection);
    emit SUpdateBalance(entry_id);
}

void LeafSStation::RAppendMultiEntry(const QUuid& node_id, const EntryList& entry_list)
{
    if (entry_list.isEmpty())
        return;

    const auto* model { FindModel(node_id) };
    if (!model)
        return;

    connect(this, &LeafSStation::SAppendMultiEntry, model, &LeafModel::RAppendMultiEntry, Qt::SingleShotConnection);
    emit SAppendMultiEntry(entry_list);
}

void LeafSStation::RRefreshStatus(const QSet<QUuid>& affected_node)
{
    for (const auto& node_id : affected_node) {
        const auto* model { FindModel(node_id) };
        if (!model)
            return;

        connect(this, &LeafSStation::SRefreshStatus, model, &LeafModel::RRefreshStatus, Qt::SingleShotConnection);
        emit SRefreshStatus();
    }
}

void LeafSStation::RRefreshField(const QUuid& node_id, const QUuid& entry_id, int start, int end)
{
    const auto* model { FindModel(node_id) };
    if (!model)
        return;

    connect(this, &LeafSStation::SRefreshField, model, &LeafModel::RRefreshField, Qt::SingleShotConnection);
    emit SRefreshField(entry_id, start, end);
}

void LeafSStation::RDirectionRule(const QUuid& node_id, bool rule)
{
    const auto* model { FindModel(node_id) };
    if (!model)
        return;

    connect(this, &LeafSStation::SDirectionRule, model, &LeafModel::RDirectionRule, Qt::SingleShotConnection);
    emit SDirectionRule(rule);
}

void LeafSStation::RNodeStatus(const QUuid& node_id, int value)
{
    const auto* model { FindModel(node_id) };
    if (!model)
        return;

    connect(this, &LeafSStation::SNodeStatus, model, &LeafModel::RNodeStatus, Qt::SingleShotConnection);
    emit SNodeStatus(value);
}

void LeafSStation::RRemoveEntryHash(const QHash<QUuid, QSet<QUuid>>& leaf_entry)
{
    for (auto it = leaf_entry.constBegin(); it != leaf_entry.constEnd(); ++it) {
        const QUuid& node_id = it.key();
        const QSet<QUuid>& entry_id_set = it.value();

        const auto* model = FindModel(node_id);
        if (!model)
            continue;

        connect(this, &LeafSStation::SRemoveMultiEntry, model, &LeafModel::RRemoveMultiEntry, Qt::SingleShotConnection);
        emit SRemoveMultiEntry(entry_id_set);
    }
}

void LeafSStation::RRemoveMultiEntry(const QUuid& node_id, const QSet<QUuid>& entry_id_set)
{
    const auto* old_model { FindModel(node_id) };
    if (old_model) {
        connect(this, &LeafSStation::SRemoveMultiEntry, old_model, &LeafModel::RRemoveMultiEntry, Qt::SingleShotConnection);
        emit SRemoveMultiEntry(entry_id_set);
    }
}
