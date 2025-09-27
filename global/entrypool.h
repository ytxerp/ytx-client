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

#ifndef ENTRYPOOL_H
#define ENTRYPOOL_H

#include <QMutex>
#include <array>
#include <deque>

#include "component/constant.h"
#include "component/enumclass.h"
#include "concepts.h"
#include "table/entry.h"

/*
 * EntryPool is a singleton object pool for Entry-derived objects.
 *
 * Features:
 * 1. Thread-safe allocation and recycling using QMutex.
 * 2. Supports pre-allocation and dynamic expansion of pools.
 * 3. Avoids pool overgrowth by deleting objects when a threshold is exceeded.
 * 4. Batch recycling is optimized: only one lock acquisition for all entries.
 * 5. Modern C++ style using std::array for fixed-size section pools.
 */
class EntryPool {
public:
    static EntryPool& Instance();
    Entry* Allocate(Section section);

    void Recycle(Entry* entry, Section section);
    template <Iterable Container> void Recycle(Container& container, Section section);

    EntryPool(const EntryPool&) = delete;
    EntryPool& operator=(const EntryPool&) = delete;
    EntryPool(EntryPool&&) = delete;
    EntryPool& operator=(EntryPool&&) = delete;

private:
    EntryPool();
    ~EntryPool();

    static Entry* NewResource(Section section);
    void Expand(std::deque<Entry*>& pool, Section section, qsizetype count);

private:
    std::array<std::deque<Entry*>, 6> pools_ {}; // Pools for each Section
    QMutex mutex_ {}; // Mutex protecting all pools
};

// Singleton
inline EntryPool& EntryPool::Instance()
{
    static EntryPool instance;
    return instance;
}

// Constructor: pre-fill pools
inline EntryPool::EntryPool()
{
    for (size_t i = 0; i != kSections.size(); ++i) {
        Expand(pools_[i], kSections[i], Pool::kExpandSize);
    }
}

// Destructor: delete remaining entries
inline EntryPool::~EntryPool()
{
    for (auto& pool : pools_) {
        qDeleteAll(pool);
    }
}

// Expand pool with 'count' new entries
inline void EntryPool::Expand(std::deque<Entry*>& pool, Section section, qsizetype count)
{
    for (qsizetype i = 0; i != count; ++i) {
        pool.push_back(NewResource(section));
    }
}

// Factory function: create a new entry based on section
inline Entry* EntryPool::NewResource(Section section)
{
    switch (section) {
    case Section::kFinance:
        return new EntryF();
    case Section::kItem:
        return new EntryI();
    case Section::kTask:
        return new EntryT();
    case Section::kPartner:
        return new EntryP();
    case Section::kSale:
        return new EntryO();
    case Section::kPurchase:
        return new EntryO();
    default:
        Q_UNREACHABLE();
    }
}

// Allocate one entry (LIFO)
inline Entry* EntryPool::Allocate(Section section)
{
    QMutexLocker locker(&mutex_);
    auto& pool = pools_[static_cast<size_t>(section)];

    if (pool.empty()) {
        Expand(pool, section, Pool::kExpandSize);
    }

    if (pool.empty()) {
        return NewResource(section);
    }

    Entry* e = pool.front();
    pool.pop_front();

    return e;
}

// Recycle one entry
inline void EntryPool::Recycle(Entry* entry, Section section)
{
    if (!entry)
        return;

    QMutexLocker locker(&mutex_);
    auto& pool = pools_[static_cast<size_t>(section)];

    if (static_cast<qsizetype>(pool.size()) + 1 >= Pool::kMaxSize) {
        locker.unlock();
        delete entry;
        return;
    }

    entry->ResetState();
    pool.push_back(entry);
}

// Batch recycle
template <Iterable Container> inline void EntryPool::Recycle(Container& container, Section section)
{
    if (container.isEmpty())
        return;

    QMutexLocker locker(&mutex_);
    auto& pool = pools_[static_cast<size_t>(section)];

    if (static_cast<qsizetype>(pool.size()) + container.size() >= Pool::kMaxSize) {
        locker.unlock();
        qDeleteAll(container);
        return;
    }

    for (Entry* e : container) {
        if (e) {
            e->ResetState();
            pool.push_back(e);
        }
    }

    container.clear();
}

#endif // ENTRYPOOL_H
