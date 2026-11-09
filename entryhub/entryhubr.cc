#include "entryhubr.h"

#include <QJsonArray>

#include "global/entrypool.h"

EntryHubR::EntryHubR(Section section, QObject* parent)
    : EntryHub(section, parent)
{
}

EntryList EntryHubR::ProcessEntryArray(const QJsonArray& array)
{
    EntryList list {};

    for (const auto& value : array) {
        Q_ASSERT(value.isObject());

        const QJsonObject obj { value.toObject() };

        Entry* entry { EntryPool::Instance().Allocate(section_) };
        entry->ReadJson(obj);

        list.emplaceBack(entry);
    }

    return list;
}
